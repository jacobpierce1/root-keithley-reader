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
  vector<float> axis_cuts;
  vector<string> leg_labels;
  string title;
  float time_cut;
  bool fill_tree;
  bool log_scale;
};

// functions
void make_compare(plot_args args);
bool file_exists(string proj_name);
void graph_compare(plot_args args);
TLegend *make_legend ();
vector<string> split(string str, char delimiter);
void fill_tree(plot_args args);
string replace_chars(string str, char start_char, char end_char);
vector<string> get_projects(string dir);
const string currentDateTime();

// constants
const string directory = "/home/helix/SIPM_TESTING/sipm_iv_curves/";
const char script_delimiter = '!';
const vector<int> NO_AXIS_CUT = {-10000, 10000, -10000, 10000};
const vector<int> GRAPH_COLORS = {1, 2, 3, 4, 6, 7, 8, 9, 30, 11, 40, 41, 42, 28, 46, 38};

// parameters to easily change the order of args
const int DIRECTORY_ARG = 0;
const int LOG_ARG = 1;
const int LEG_ARG = 2;
const int TITLE_ARG = 3;
const int AXISCUT_ARG = 4;

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
  if(args.log_scale) c1->SetLogy();
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
    graphs[i]->SetMarkerColorAlpha(GRAPH_COLORS[i], 0);
    graphs[i]->SetLineColor(GRAPH_COLORS[i]);
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
      vector<string> temp = split(line, ',');
      // sscanf(line.c_str(), "%*d,%f,Amp DC,%*f,%f,F,F,F,F,F,F,%*s,F,Main,%f,Volt DC,%*d,%*s,%*s", &rCurrent, &rValue, &rVolt );
      rCurrent = atof(temp[1].c_str());
      rVolt = atof(temp[14].c_str());
      
      // check whether there is an axis cut and if so do it
      if (args.axis_cuts.size() != 0) {
	if (rVolt > args.axis_cuts[0] && rVolt < args.axis_cuts[1] && rCurrent > args.axis_cuts[2] && rCurrent < args.axis_cuts[3]) {
	  graphs[i]->SetPoint(counter, rVolt, rCurrent);
	}
      }
      else graphs[i]->SetPoint(counter, rVolt, rCurrent);
      counter++;
    }

    // remove last point
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
  string cmd1 = "ls " + dir + "*.csv > temp.txt";
  system(cmd1.c_str());

  // read out of the text file
  vector<string> files;
  string line;
  size_t dir_length = dir.length();
  ifstream temp_file ("temp.txt");
  if (temp_file.is_open()) {
    while (getline(temp_file, line)) {
      files.push_back(line.substr(dir_length));
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
  if (parsed_args[DIRECTORY_ARG] == "") args.dir = directory;
  else args.dir = directory + parsed_args[DIRECTORY_ARG] + "/";
  args.fill_tree = false;
  args.log_scale = false;

  // fill projects and num_projects
  args.projects = get_projects(args.dir);
  args.num_projects = args.projects.size();

  // title
  if (num_args >= TITLE_ARG + 1) {
    args.title = replace_chars(parsed_args[TITLE_ARG], '_', ' ');
    if(args.title == "d" || args.title == "default") {
      args.title = replace_chars(parsed_args[DIRECTORY_ARG], '_', ' ');
    }
  } else args.title = replace_chars(parsed_args[DIRECTORY_ARG], '_', ' ');

  // option to use log scale for y axis
  if (num_args >= LOG_ARG + 1) {
    if (parsed_args[LOG_ARG] == "log") args.log_scale = true;
  }
  
  // get axis cuts
  if (num_args >= AXISCUT_ARG + 1) {
    vector<string> temp = split(parsed_args[AXISCUT_ARG], ',');
    for (int i=0; i<4; i++) {
      if (temp[i] == "d" || temp[i] == "default") args.axis_cuts.push_back(NO_AXIS_CUT[i]);
      else args.axis_cuts.push_back(atof(temp[i].c_str()));
    }
  }

  // legend labels
  if (num_args >= LEG_ARG + 1){

    // check for the fill_tree mode
    if(parsed_args[LEG_ARG + 1] == "tree") args.fill_tree = true;

    // check for default modes
    else if (parsed_args[LEG_ARG] == "default" || parsed_args[LEG_ARG] == "d") {
      for (int i=0; i<args.num_projects; i++) {
	args.leg_labels.push_back(replace_chars(args.projects[i].substr(0, args.projects[i].find('.')), '_', ' '));
      }
    }
  
    // otherwise proceed as normally
    else {
      args.leg_labels = split(parsed_args[LEG_ARG], ',');
      for (int i=0; i<args.num_projects; i++) {

	// check for default option and if so parse project name
	if (args.leg_labels[i] == "" || args.leg_labels[i] == "default" || 
	    args.leg_labels[i] == "d") {
	 
	  args.leg_labels[i] = replace_chars(args.projects[i].substr(0, args.projects[i].find('.')), '_', ' ');
	}

	// the only case in which the legend specified is actually used:
	else args.leg_labels[i] = replace_chars(args.leg_labels[i], '_', ' ');
      }
    }
    
  }
  // case: legend argument not supplied
  else {
        for (int i=0; i<args.num_projects; i++) {
	args.leg_labels.push_back(replace_chars(args.projects[i].substr(0, args.projects[i].find('.')), '_', ' '));
      }
  }
  return args;
}



// main function
void plot_keithley(string arg_string) {
  
  plot_args args = parse_args(arg_string);
  
  make_compare(args);
  if (args.fill_tree) fill_tree(args);
  return;
}
