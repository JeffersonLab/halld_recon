#include <iostream>
#include <fstream>
#include <sstream> 
using namespace std;
#include <TFile.h>
#include <TGraphErrors.h>
#include <TMath.h>
#include <TGraph2D.h>
#include <TRandom.h>
#include <TRandom3.h>
#include <TStyle.h>
#include <TText.h>
#include <TCanvas.h>
//#include <TF2.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TROOT.h>
#include <TLegend.h>
#include <TFitResult.h>
#include <TRatioPlot.h>
#include <TProfile2D.h>
#include <TLatex.h>
#include <TLine.h>
#include <TGaxis.h>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TLorentzRotation.h"
#include <TGenPhaseSpace.h>

#include "/work/halld/home/gxproj2/calibration/ilib/Nicer2D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nicer1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice3D2D.c"
/*
#include "/home/igal/root/ilib/Nicer2D.c"
#include "/home/igal/root/ilib/Nicer1D.c"
#include "/home/igal/root/ilib/Nice1D.c"
#include "/home/igal/root/ilib/Nice3D2D.c"
*/
//#define ITERATION 7


void mapping_bad_ch(TString path_dir = "", TString str_run = "") {
  //Begin by Defining iteration number.
  //ITERATION is used to get gains from previous iterations
  const int kBlocksWide = 59;
  const int kBlocksTall = 59;
  int kMaxChannels = kBlocksWide * kBlocksTall;
  
  
  // Script used for fitting the output of the FCAL_Pi0TOF plugin
  // Invoke using root -l -b <rootfile> FitGains.C
  // Force Batch Mode
  //gROOT->SetBatch();
  
  gROOT->Reset();
  
  gROOT->SetStyle("Bold");
  gStyle->SetCanvasColor(0);
  gStyle->SetLabelColor(1);
  gStyle->SetLabelColor(1,"Y");
  gStyle->SetHistLineColor(1); 
  gStyle->SetHistLineWidth(1); 
  gStyle->SetNdivisions(505);
  gStyle->SetNdivisions(505,"Y");
  //gROOT->Macro("setcolor2.c");
  //gStyle->SetHistFillColor(999);
  gROOT->SetStyle("Plain");  // white as bg
  gStyle->SetOptStat("111111");
  gStyle->SetFrameLineWidth(1);
  gStyle->SetTitleFontSize(0.1);
  gStyle->SetTitleBorderSize(0);
    
  //Definitions
  Double_t smallBetween1 = .1;
  Double_t smallBetween2 = .1;
  Double_t smallBetween3 = .1;
  Double_t smallBetween4 = .1;
  
  Double_t small = .00001;
  
  TLine TLine;
  //TLine * fLine;
  TLatex *t = new TLatex();
  t->SetTextSize(0.05);
  t->SetTextFont(42);
  t->SetTextAlign(12);
  t->SetTextColor(1);
  
  TCanvas *C1;
  
  TPad *SmallC;
  TGaxis *XAxis,*YAxis;
  TLatex XTitle,YTitle,XtopTitle;
  
  TLegend *legend;

  ifstream in;
  ofstream out;
  TString Line = ""; 
  TString cleg = "";
  

  TFile * ifile = new TFile(path_dir + Form("run_%s.root", str_run.Data())); 
  // Define histograms
  TH2F * hPi0MassVsChNum = (TH2F *) gDirectory->Get("FCAL_Pi0log/Pi0MassVsChNum");
  
  // Make an output file
  TH1F * h_gg_im[2800];

  int sq_vs_ch[2800];
  vector <int> square_tab[24];
  vector <int> bad_det_tab;
  vector <int> bad_rg_tab;
    
  // Initialization
  for (int i = 0; i < 2800; i ++) {
    sq_vs_ch[i] = -1;
  }
  
  int ctr = 0;
  
  for (int i = 0; i < 24; i ++) {
    TString ring_file = Form("rings/FCAL_rings_%d.txt", i + 1);
    in.open(ring_file);
    int k = 0;
    while ( in.good() ) {
      int ch = -1;
      in>>ch;
      if (ch >= 0) {
	sq_vs_ch[ch] = i;
	square_tab[i].push_back(ch);
	ctr ++;
	k ++;
      }
    }
    in.close();
    cout <<"ring " << i << " #ch " << k << endl;
  }
  cout <<"Total ch # "<< ctr << " truth " << hPi0MassVsChNum->GetNbinsX() << endl;

  int max_ctr = 50;
  
  int good_ch_ctr = 0;
  for (int i = 0; i < 24; i ++) {
    
    int canvas_nb = square_tab[i].size() / 16;
    
    for (int j = 0; j <= canvas_nb; j ++) {
      
      for (int k = 0; k < 16; k ++) {
	
	int size = k + j * 16;
	
	if (size < (int) square_tab[i].size()) {
	  
	  int l = square_tab[i][size];
	  
	  h_gg_im[l] = (TH1F *) hPi0MassVsChNum->ProjectionY(Form("h_gg_im_%d", l), l + 1, l + 1);
	  if (h_gg_im[l]->GetEntries() > max_ctr) {
	    good_ch_ctr ++;
	  } else {
	    bad_det_tab.push_back(l);
	    bad_rg_tab.push_back(i);
	  }
	}
      }
    }
  }
  
  out.open(path_dir + Form("bad_gains_%s.txt", str_run.Data()));
  for (int i = 0; i < (int) bad_det_tab.size(); i ++) {
    out << bad_det_tab[i] << endl;
  }
  out.close();
  cout << "Nb of bad channels " << bad_det_tab.size() << endl;
}
