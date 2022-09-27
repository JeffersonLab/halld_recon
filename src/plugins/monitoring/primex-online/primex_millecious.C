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

void primex_millecious(){
  
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
  
  TH2F * h2D = (TH2F *) gDirectory->Get("histo/fcal_tof_dx");
  Nice3D2D(h2D, 0.05, 0.05, 42, 505, 1.1, 1., 1.25, "",
	   "#font[42]{|x_{TOF}-x_{FCAL}| [cm]}",
	   "#font[42]{|y_{TOF}-y_{FCAL}| [cm]}",
	   "#font[42]{Entries #}");
  
  int nbin_im;
  double min_im_bin, max_im_bin, im_step;
  TH1F * h_time = (TH1F *) ((TH2F *) gDirectory->Get("histo/TaggerTiming_vs_egcut"))->ProjectionY("time");
  nbin_im = h_time->GetNbinsX();
  min_im_bin = h_time->GetXaxis()->GetXmin();
  max_im_bin = h_time->GetXaxis()->GetXmax();
  im_step = (max_im_bin - min_im_bin) / ((double) nbin_im);
  Nice1Db(h_time,-30, 30, 0.05, 0.05, 42,505,1.,1.3,"","#font[42]{t_{Tagger}-t_{RF} [ns]}", Form("#font[41]{Events / %0.3f [ns]}", im_step));
  
  TString str_trg_gg[] = {"FCALetato2g_trg3_Esum_sc_w", "FCALetato2g_trg2_Esum_sc_w"};
  TString str_trg_pi0pi0pi0[] = {"FCALetato3pi0_trg3_Esum_sc_w", "FCALetato3pi0_trg2_Esum_sc_w"};
  TString str_trg_pi0pippim[] = {"FCALetato2g2pi_trg3_Esum_sc_w", "FCALetato2g2pi_trg2_Esum_sc_w"};
  TH1F * h_trg_gg[2];
  TH1F * h_trg_pi0pi0pi0[2];
  TH1F * h_trg_pi0pippim[2];
  int color[] = {2, 1};
  int marker[] = {20, 21};
  for (int i = 0; i < 2; i ++) {
    
    h_trg_gg[i] = (TH1F *) gDirectory->Get(dir_path + str_trg_gg[i]);
    h_trg_pi0pi0pi0[i] = (TH1F *) gDirectory->Get(dir_path + str_trg_pi0pi0pi0[i]);
    h_trg_pi0pippim[i] = (TH1F *) gDirectory->Get(dir_path + str_trg_pi0pippim[i]);
    h_trg_gg[i]->Rebin(100);
    h_trg_pi0pi0pi0[i]->Rebin(100);
    h_trg_pi0pippim[i]->Rebin(100);
    nbin_im = h_trg_gg[i]->GetNbinsX();
    min_im_bin = h_trg_gg[i]->GetXaxis()->GetXmin();
    max_im_bin = h_trg_gg[i]->GetXaxis()->GetXmax();
    im_step = (max_im_bin - min_im_bin) / ((double) nbin_im);
    Nice1D(h_trg_gg[i], 0.05, 0.05, 42,505,1.,1.3,"","#font[42]{E^{FCAL}_{sum} [GeV]}", Form("#font[41]{Events / %0.3f [GeV]}", im_step));
    Nice1D(h_trg_pi0pi0pi0[i], 0.05, 0.05, 42,505,1.,1.3,"","#font[42]{E^{FCAL}_{sum} [GeV]}", Form("#font[41]{Events / %0.3f [GeV]}", im_step));
    Nice1D(h_trg_pi0pippim[i], 0.05, 0.05, 42,505,1.,1.3,"","#font[42]{E^{FCAL}_{sum} [GeV]}", Form("#font[41]{Events / %0.3f [GeV]}", im_step));
    
    h_trg_gg[i]->SetLineColor(color[i]);
    h_trg_gg[i]->SetMarkerColor(color[i]);
    h_trg_gg[i]->SetMarkerSize(1);
    h_trg_gg[i]->SetLineWidth(1);
    h_trg_gg[i]->SetMarkerStyle(marker[i]);
    
    h_trg_pi0pi0pi0[i]->SetLineColor(color[i]);
    h_trg_pi0pi0pi0[i]->SetMarkerColor(color[i]);
    h_trg_pi0pi0pi0[i]->SetMarkerSize(1);
    h_trg_pi0pi0pi0[i]->SetLineWidth(1);
    h_trg_pi0pi0pi0[i]->SetMarkerStyle(marker[i]);
    
    h_trg_pi0pippim[i]->SetLineColor(color[i]);
    h_trg_pi0pippim[i]->SetMarkerColor(color[i]);
    h_trg_pi0pippim[i]->SetMarkerSize(1);
    h_trg_pi0pippim[i]->SetLineWidth(1);
    h_trg_pi0pippim[i]->SetMarkerStyle(marker[i]);
  }

  
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .075;
  smallBetween4 = .15;
  
  cleg = "PrimEx-eta-millecious";
  C1 = new TCanvas(cleg, cleg, 10, 10, 1800, 1200);
  C1->Divide(3,2);
  smallBetween1 = .11;
  smallBetween2 = .175;
  smallBetween3 = .05;
  smallBetween4 = .125;
  C1->cd(1);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h2D->SetMinimum(1);
  h2D->Draw("colz");
  gPad->SetLogz();

  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .125;
  C1->cd(2);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h_time->Draw();
  
  C1->cd(4);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h_trg_gg[0]->Draw();
  h_trg_gg[1]->Draw("same");
  
  legend=new TLegend(0.25,0.75,0.45,0.9);
  legend->AddEntry(h_trg_gg[1],"#font[42]{Bit 1, E^{FCAL}_{sum} #geq 3.5GeV}","p");
  legend->AddEntry(h_trg_gg[0],"#font[42]{Bit 2, E^{FCAL}_{sum} #geq 0.5GeV}","p");
  legend->SetFillColor(0);
  legend->SetTextFont(22);
  legend->SetTextSize(.05);
  legend->SetLineColor(0);
  legend->Draw("same");
  
  t->DrawLatex(0.5, h_trg_gg[0]->GetMaximum() * 0.7, "#font[42]{#eta#rightarrow#gamma#gamma}");
  
  
  C1->cd(5);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h_trg_pi0pi0pi0[0]->Draw();
  h_trg_pi0pi0pi0[1]->Draw("same");
  
  t->DrawLatex(0.5, h_trg_pi0pi0pi0[0]->GetMaximum() * 0.7, "#font[42]{#eta#rightarrow#pi^{0}#pi^{0}#pi^{0}}");

  C1->cd(6);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h_trg_pi0pippim[0]->Draw();
  h_trg_pi0pippim[1]->Draw("same");
  
  t->DrawLatex(0.5, h_trg_pi0pippim[0]->GetMaximum() * 0.7, "#font[42]{#eta#rightarrow#pi^{0}#pi^{+}#pi^{-}}");
  
}
