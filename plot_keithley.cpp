#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
using namespace std;

// struct to store all the information for a plot 
struct plot_args {
  string dir;
  int num_projects;
  vector<string> projects;
  vector<int> channels; 
  //  vector<float> time_shifts;
  vector<string> leg_labels;
  string title;
  float time_cut;
  bool fill_tree;
};

// functions
void make_compare(plot_args args);
bool file_exists(string proj_name);
void graph_compare(plot_args args);
TLegend *make_legend ();
void copy_files (plot_args args, int i);
vector<string> split(string str, char delimiter);
void fill_tree(plot_args args);
string switch_chars(string str, char start_char, char end_char);
void combine_temp_files(plot_args args);
vector<string> get_projects(string dir);
const string currentDateTime();

// constants
const string directory = "/home/helix/SIPM_TESTING/sipm_iv_curves/";
const char script_delimiter = '!';


/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



// function to compare two projects
void make_compare(plot_args args) {
  cout << "Generating comparison" << endl;
  
  // make sure all files exist
  for (int i=0; i<args.num_projects; i++) {
    string file = args.dir + args.projects[i];
    if (!file_exists(file)) {
      cout << "Project " << i << " does not exist: "; 
      return;
    }
  }

  // make and plot the graph
  graph_compare(args); 
}


// make graphs for the project
void graph_compare(plot_args args) {
  // declare canvas, graphs, multigraph, legend
  TCanvas *c1 = new TCanvas("c1", "IV Plot", 200, 10, 700, 500);
  c1->SetLogy();
  TGraph *graphs[args.num_projects];
  TMultiGraph *mg = new TMultiGraph();
  string mg_title = args.title + ";" + "Voltage (V); Current (A)";
  mg->SetTitle(mg_title.c_str());

  // make legend
  TLegend *leg = make_legend();

  // make graphs and add to multigraph
  for(int i=0; i<args.num_projects; i++) {
    // open file and declare graph
    string file = args.dir + args.projects[i];
    ifstream fin (file.c_str());
    graphs[i] = new TGraph();
    graphs[i]->SetLineWidth(2);
    graphs[i]->SetMarkerColorAlpha(i+1, 0);
    graphs[i]->SetLineColor(i+1);  // lucky coincidence that these are the colors we want
    graphs[i]->SetFillColor(0);
    
    // first 8 lines of the file are not data
    string line;
    for (int j=0; j<8; j++) {
      getline(fin, line);
    }

    // now read the rest of the file into graphs[i]
    float rCurrent, rValue, rVolt;
    int counter = 0;
    while(getline(fin, line)) {
      sscanf(line.c_str(), "%*d,%f,Amp DC,%*f,%f,F,F,F,F,F,F,Front,F,Main,%f,Volt DC,%*d,%*s,%*s", &rCurrent, &rValue, &rVolt );
      graphs[i]->SetPoint(counter, rVolt, rCurrent);
      counter++;
    }
    graphs[i]->RemovePoint(graphs[i]->GetN()-1);
    
    //add to multigraph and legend
    mg->Add(graphs[i]);
    leg->AddEntry(graphs[i], args.leg_labels[i].c_str());

    // close and clear file
    fin.close();
    fin.clear();
  }
    
  // draw multigraph and legend; return.
  mg->Draw("apl");
  leg->Draw();
  return;
}



// construct the legend
TLegend *make_legend() { 
  leg_w = .2;
  leg_h = .15;
  leg_x = .67;
  leg_y = .71;
  TLegend *leg = new TLegend(leg_x, leg_y, leg_x+leg_w, leg_y+leg_h);
  leg->SetHeader("Legend"); 
  return leg;
}



// check whether file exists
bool file_exists(string file) {
  cout  << file << endl;
  ifstream test_file(file.c_str());
  return test_file.good();
}



// fill a ttree with the data and store in a binary file.
void fill_tree(plot_args args) {
  cout << "Filling tree" << endl;

  // get title of the file (remove ambiguity for comparison)
  string title = "";
  if (args.title != "Comparison") title = args.title;
  else title = args.title + "_" + currentDateTime();

  // open file and use correct directory
  string file_name = title + ".root";
  string file_location = directory + args.dir;
  TFile ofile(file_name.c_str(), "RECREATE");
  
  // declare tree and branches
  TTree *tree = new TTree("name", "title");   // todo
  float time;
  tree->Branch("Time", &time, "Time/F");

  float temps[args.num_projects];
  for (int i=0; i<args.num_projects; i++) {
    string branch_name = "Temps" + to_string(i);
    tree->Branch(branch_name.c_str(), &temps[i], (branch_name + "/F").c_str());
  }

  // open the files to be read
  ifstream files[5];
  for (int i=0; i<args.num_projects; i++) {
    files[i].open((file_location + args.projects[i]).c_str());
  }
  
  // open the files and read into the them
  string line;
  for (int i=0; i<args.num_projects; i++) {
    while(getline(files[i], line)) {

      // get data
      vector<string> data = split(line, ' ');
      time = atof(data[0].c_str());
      temps[i] = atof(data[1].c_str());

      // add time if not there already

      // write into the tree
      int status = tree->Fill();
    }
  }

  // save the tree to the file and close the file.
  tree->Write();
  ofile.Close();

  /* by now, there should be a file in the directory of PlotData.cpp 
     called <project>.root. this manually moves the file to the 
     correct directory. there is probably a better way to do this. */
  string sys_cmd_tmp = "mv " + file_name + " " + directory + "/TTREES/";
  const char *sys_cmd = sys_cmd_tmp.c_str();
  system(sys_cmd);
  return;
}



// split according to delimiter
vector<string> split(string str, char delimiter) {
  // initializing vars
  size_t length = count(str.begin(), str.end(), delimiter) + 1;
  vector<string> split_string(length);
  size_t pos = 0;
  string token = "";

  // loop through str
  int i = 0;
  while ((pos = str.find(delimiter)) != std::string::npos) {
    token = str.substr(0, pos);
    str.erase(0, pos + 1);
    split_string[i] = token;
    i++;
  }
  if (str != "") split_string[i] = str;

  return split_string;
}



string replace_chars(string str, char start_char, char end_char) {
  for(string::iterator it = str.begin(); it != str.end(); it++) {
    if(*it == start_char) *it = end_char;
  }
  return str;
}



// get current date/time and report it as a string
const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d-%T", &tstruct);

    return buf;
}




// use linux shell command to count number of files
vector<string> get_projects(string dir) {
  
  // send names of available files into text file called temp
  string cmd1 = "ls " + dir + " > temp.txt";
  system(cmd1.c_str());

  // read out of the text file
  vector<string> files;
  string line;
  ifstream temp_file ("temp.txt");
  if (temp_file.is_open()) {
    while (getline(temp_file, line)) {
      files.push_back(line);
    }
  }

  // read out of the file
  else cout << "ERROR: could not count number of files to be plotted" << endl;
  temp_file.close();

  return files;
}


// obtain command line args from the shell script ./plot
plot_args parse_args(string arg_string) {
  
  // initializing everything
  vector<string> parsed_args = split(arg_string, script_delimiter);
  int num_args = parsed_args.size() - 1;
  parsed_args.erase(parsed_args.begin() + num_args);   // fixes weird parsing error

  // fill the plot_args containing info about graph
  plot_args args;
  if (parsed_args[0] == "") args.dir = directory + "plot_directory/";
  else args.dir = directory + parsed_args[0] + "/"; 
  args.fill_tree = false;

  // fill projects and num_projects
  args.projects = get_projects(args.dir);
  args.num_projects = args.projects.size();

  // title
  if (num_args >= 3) {
    args.title = replace_chars(parsed_args[2], '_', ' ');
    if(args.title == "d" || args.title == "default") {
      args.title = "Keithley Plot";
    }
  } else {
    args.title = "Keithley Plot";
  }

  // get time cut
  if (num_args >= 5) args.time_cut = atof(parsed_args[4].c_str());
  else args.time_cut = 0;

  // legend labels. this is done last since it changes the mode.
  if (num_args >= 2){

    // check for the fill_tree mode
    if(parsed_args[1] == "tree") args.fill_tree = true;

    // otherwise proceed as normally
    else {
      args.leg_labels = split(parsed_args[1], ',');
      for (int i=0; i<args.num_projects; i++) {
	args.leg_labels[i] = replace_chars(args.leg_labels[i], '_', ' ');
	if (args.leg_labels[i] == "" || args.leg_labels[i] == "default" || 
	    args.leg_labels[i] == "d") args.leg_labels[i] = args.projects[i];
      }
    }
  } else for (int i=0; i<args.num_projects; i++) args.leg_labels.push_back(args.projects[i]);

  return args;
}



// main function
void plot_keithley(string arg_string) {
  
  plot_args args = parse_args(arg_string);
  
  make_compare(args);
  if (args.fill_tree) fill_tree(args);
  return;
}
