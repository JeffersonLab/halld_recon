#include <iostream>
#include <fstream>
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
#include <TLatex.h>
#include <TLine.h>
#include <TGaxis.h>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TLorentzRotation.h"
#include <TGenPhaseSpace.h>
#include "TProfile.h"
#include "Nice1D.C"
#include "Nice3D2D.C"
#include "Nicer1D.C"
#include "Nice1Db.C"

void primex_elasticity(){
  
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
  gStyle->SetHistFillColor(999);
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
  TLatex *t = new TLatex();
  t->SetTextSize(0.05);
  t->SetTextFont(42);
  t->SetTextAlign(12);
  t->SetTextColor(1);
  //t->SetTextFont(12);

  TCanvas *C1;
  
  TPad *SmallC;
  TGaxis *XAxis,*YAxis;
  TLatex XTitle,YTitle,XtopTitle;
  
  TLegend *legend;

  ifstream in;
  ofstream out;
  TString cleg = "";
  TString xlab = "";
  TString ylab = "";
  TString dir_path = "histo/";
  TString file = "";
  
  TH2F * h2D[3];
  TH1F * h_theta[3];

  TString str_elasticity[] = {"Primakoff_2g", "Primakoff_6g", "Primakoff_2g2pi"};
  TString str_theta[] = {"theta_2g", "theta_6g", "theta_2g2pi"};
  TString str_fs[] = {"#gamma#gamma", "#pi^{0}#pi^{0}#pi^{0}", "#pi^{0}#pi^{+}#pi^{-}"};
  TString str_E[] = {"E^{1}_{#gamma}+E^{2}_{#gamma}", 
		     "E^{1}_{#gamma}+E^{2}_{#gamma}+E^{3}_{#gamma}+E^{4}_{#gamma}+E^{5}_{#gamma}+E^{6}_{#gamma}", 
		     "E^{1}_{#gamma}+E^{2}_{#gamma}+E_{#pi^{+}}+E_{#pi^{-}}"};
  TString str_m[] = {"#it{m}_{#gamma#gamma}", "#it{m}_{#pi^{0}#pi^{0}#pi^{0}}", "#it{m}_{#pi^{0}#pi^{+}#pi^{-}}"};
  int nbin_im;
  double min_im_bin, max_im_bin, im_step;
  
  for (int i = 0; i < 3; i ++) {
    h2D[i] = (TH2F *) gDirectory->Get(dir_path + str_elasticity[i]);
    Nice3D2D(h2D[i], 0.05, 0.05, 42, 505, 1.1, 0.9, 1.25, "",
	     Form("#font[42]{%s/m_{#eta}}", str_E[i].Data()),
	     Form("#font[42]{%s/E_{#gamma}^{beam}}", str_m[i].Data()),
	     "#font[42]{Entries #}");

    h_theta[i] = (TH1F *) gDirectory->Get(dir_path + str_theta[i]);
    h_theta[i]->Rebin(10);
    nbin_im = h_theta[i]->GetNbinsX();
    min_im_bin = h_theta[i]->GetXaxis()->GetXmin();
    max_im_bin = h_theta[i]->GetXaxis()->GetXmax();
    im_step = (max_im_bin - min_im_bin) / ((double) nbin_im);
    Nice1D(h_theta[i], 0.05, 0.05, 42,505,1.,1.6,"",Form("#font[42]{#it{#theta}_{#eta#rightarrow%s} [^{o}]}", str_fs[i].Data()), Form("#font[41]{Events / %0.3f [^{o}]}", im_step));
    
    h_theta[i]->SetLineColor(1);
    h_theta[i]->SetMarkerColor(1);
    h_theta[i]->SetMarkerSize(1);
    h_theta[i]->SetLineWidth(1);
    h_theta[i]->SetMarkerStyle(20);
  }

  
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .075;
  smallBetween4 = .15;
  
  cleg = "PrimEx-eta-elasticity";
  C1 = new TCanvas(cleg, cleg, 10, 10, 1800, 1200);
  C1->Divide(3,2);
  for (int i = 0; i < 3; i ++) {
    smallBetween1 = .1;
    smallBetween2 = .175;
    smallBetween3 = .05;
    smallBetween4 = .125;
    C1->cd(1 + i);
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    h2D[i]->SetMinimum(1);
    h2D[i]->Draw("colz");
    gPad->SetLogz();
    
    smallBetween1 = .15;
    smallBetween2 = .05;
    smallBetween3 = .05;
    smallBetween4 = .15;
    C1->cd(4 + i);
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    h_theta[i]->Draw();
  }

}
