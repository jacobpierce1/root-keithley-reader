void plot_keithley( string csv_filename, string graphName, string rootfilename, string fileStatus) {

  ifstream fin;
  fin.open( csv_filename);

  string dummys;
  char dummyc;
  int rId;
  float rCurrent, rValue, rVolt;
  int counter = 0;

  TGraph *gr = new TGraph();
  gr->SetMarkerStyle(24);
  for(int i=0; i<8; i++){
    getline( fin, dummys);
  }
  while( !fin.eof() && fin.good()){
    getline(fin, dummys);
    sscanf( dummys.c_str(), "%*d,%f,Amp DC,%*f,%f,F,F,F,F,F,F,Front,F,Main,%f,Volt DC,%*d,%*s,%*s", &rCurrent, &rValue, &rVolt );
    gr->SetPoint( counter, rVolt, rCurrent );
    counter ++; 
  }
  gr->RemovePoint(gr->GetN()-1);

  TCanvas *c1 = new TCanvas("c1","c1", 800, 600);
  c1->SetFillColor(10);
  gPad->SetTickx(1);
  gPad->SetTicky(1);
  gr->Draw("apl");

  TFile *fout = new TFile( rootfilename.c_str(), fileStatus.c_str());
  gr->Write( graphName.c_str() );
  fout->Write();
  fout->Close();
  fin.close();
  fin.clear();
}
