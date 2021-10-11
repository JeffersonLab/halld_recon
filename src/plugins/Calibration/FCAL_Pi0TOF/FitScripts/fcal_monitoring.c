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

#include "Nice3D2D.c"

void fcal_monitoring() {
      
  cout << endl << "Checking FCAL for:" << endl;
  cout <<         " 1/ Bad calibration constants" << endl;
  cout <<         " 2/ Bad blocks" << endl;
  cout <<         " 3/ Hot blocks" << endl;
  
  gROOT->Reset();
    
  gROOT->SetStyle("Bold");
  gStyle->SetCanvasColor(0);
  gStyle->SetLabelColor(1);
  gStyle->SetLabelColor(1,"Y");
  gStyle->SetHistLineColor(1); 
  gStyle->SetHistLineWidth(1); 
  gStyle->SetNdivisions(505);
  gStyle->SetNdivisions(505,"Y");
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
  t->SetTextFont(42);
  t->SetTextAlign(12);
  t->SetTextColor(1);
  t->SetTextSize(0.01);
  
  TCanvas *C1;
  
  TPad *SmallC;
  TGaxis *XAxis,*YAxis;
  TLatex XTitle,YTitle,XtopTitle;
  
  TLegend *legend;

  ifstream in;
  ofstream out;
  TString cleg = "";
  TString path = "./";
  TString file = "";

  const int kBlocksWide = 59;
  const int kBlocksTall = 59;
  int kMaxChannels = kBlocksWide * kBlocksTall;
  double kMidBlock = ( kBlocksWide - 1 ) / 2;
  double kBeamHoleSize = 3;
  
  double blockSize=4.0157;
  double radius=1.20471*100.;
  double blockLength=45.;

  double innerRadius = ( kBeamHoleSize - 1 ) / 2. * blockSize * sqrt(2.)*1.01;

  bool   m_activeBlock[kBlocksTall][kBlocksWide];
  TVector2 m_positionOnFace[kBlocksTall][kBlocksWide];
  double BlockGain[kBlocksTall][kBlocksWide],BlockWidth[kBlocksTall][kBlocksWide],BlockMean[kBlocksTall][kBlocksWide];
  int m_numActiveBlocks = 0;
  int m_numDeadBlocks = 0;
  double BlockGainActive[2800];
  int OuterChannelNum[2800];
  int nOuter = 0;

  double channelNum[2800];
  int m_channelNumber[kBlocksTall][kBlocksWide];
  int m_row[kMaxChannels];
  int m_column[kMaxChannels];
  
  TGraph * gr_gain_low = new TGraph();
  TGraph * gr_gain_high = new TGraph();
  gr_gain_low->SetLineColor(1);
  gr_gain_low->SetLineWidth(3);
  gr_gain_high->SetLineColor(2);
  gr_gain_high->SetLineWidth(3);

  for (int i = 0; i < 2800; i ++) channelNum[i] = i + 1;

  TProfile * hCurrentGainConstants = (TProfile *) gDirectory->Get("FCAL_Pi0/CurrentGainConstants");
  TString hname[] = {"XY/xy_all", "XY/xy_1GeV", "XY/xy_2GeV", "XY/xy_3GeV", "XY/xy_4GeV", "XY/xy_5GeV", "XY/xy_6GeV", "XY/xy_7GeV", "XY/xy_8GeV", "XY/xy_9GeV", "XY/xy_10GeV"};
  TH2F * h2D[11];
  for (int i = 0; i < 12; i ++) {
    if (i < 11) h2D[i] = (TH2F *) gDirectory->Get(hname[i]);
    else h2D[i] = (TH2F *) gDirectory->Get(hname[3]);
    Nice3D2D(h2D[i], 0.05, 0.05, 42, 505, 1.1, 0.9, 1.25, "",
	     "#font[42]{x [cm]}",
	     "#font[42]{y [cm]}",
	     "#font[42]{Entries #}");
    
    smallBetween1 = .1;
    smallBetween2 = .175;
    smallBetween3 = .05;
    smallBetween4 = .125;
    
    if (i == 0) cleg = "FCAL-pattern-for-Emax-larger-than-block-energy-threshold";
    else if (0 < i && i < 11) cleg = Form("FCAL-pattern-for-Emax-larger-than-%dGeV", i);
    else if(i == 11) cleg = "FCAL-pattern-for-Emax-larger-than-3GeV-and-with-bad-gains";

    C1 = new TCanvas(cleg, cleg, 10, 10, 1200, 1200);
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    
    h2D[i]->Draw("colz");
    gPad->SetLogz();
    t->SetTextSize(0.03);
    if (i == 0) t->DrawLatex(-125, 132.5, "Pattern for all E^{cluster}_{max}");
    if (0 < i &&i < 11) t->DrawLatex(-125, 132.5, Form("Pattern for E^{cluster}_{max} #geq %d GeV", i));
    else if (i == 11) t->DrawLatex(-125, 132.5, "Pattern for E^{cluster}_{max} #geq4 GeV");
    if (i == 11) {
      
      for (int row = 0; row < kBlocksTall; row ++) {
	
	for (int col = 0; col < kBlocksWide; col ++) {
	  
	  m_positionOnFace[row][col] =TVector2(  ( col - kMidBlock ) * blockSize,( row - kMidBlock ) * blockSize );
	  double thisRadius = m_positionOnFace[row][col].Mod();
	        
	  if (innerRadius < thisRadius && thisRadius < radius) {
	    
	    double gain = hCurrentGainConstants->GetBinContent(m_numActiveBlocks + 1);
	    double x_center = m_positionOnFace[row][col].X();
	    double y_center = m_positionOnFace[row][col].Y();
	    	    
	    if ( (gain > 1.45 || gain < 0.65) ) {
	      double x0 = x_center - blockSize / 2.0;
	      double x1 = x_center + blockSize / 2.0;
	      double y0 = y_center - blockSize / 2.0;
	      double y1 = y_center + blockSize / 2.0;

	      if (gain > 1.45) {
		TLine.SetLineWidth(2);
		TLine.SetLineColor(2);
	      } else if (gain < 0.65) {
		TLine.SetLineWidth(2);
		TLine.SetLineColor(1);
	      }
	      
	      TLine.DrawLine(x0, y0, x1, y0);
	      TLine.DrawLine(x0, y1, x1, y1);
	      TLine.DrawLine(x0, y0, x0, y1);
	      TLine.DrawLine(x1, y0, x1, y1);
	      t->SetTextSize(0.01);
	      t->DrawLatex(x_center, y_center, Form("#font[42]{%d}", m_numActiveBlocks));
	    }
	    m_numActiveBlocks++;
	  }
	}
      }
      legend=new TLegend(0.7,0.87,0.8,0.92);
      legend->AddEntry(gr_gain_low,"#font[42]{G #leq 0.65}","l");
      legend->AddEntry(gr_gain_high,"#font[42]{G #geq 1.45}","l");
      legend->SetFillColor(0);
      legend->SetTextFont(22);
      legend->SetTextSize(.03);
      legend->SetLineColor(0);
      legend->Draw("same");
    }
    C1->Print(path + cleg + ".pdf");
    C1->Print(path + cleg + ".png");
  }
}
