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
#include <TFitResult.h>

#include "/work/halld/home/gxproj2/calibration/ilib/Nicer2D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice2D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice3D2D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nicer1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice1Db.c"

void nonlinear_correction_pi0_lh2(TString path_base = "", int npts = 20, int iter = 0){
  
  TString str_m = "";
  if (path_base.Contains("method0"))
    str_m = "FCAL_Pi0HFA";
  else if (path_base.Contains("method1"))
    str_m = "FCAL_Pi0TOF";
  else if (path_base.Contains("method2"))
    str_m = "FCAL_Pi0log";

  if (path_base.Contains("method-0"))
    str_m = "FCAL_Pi0HFA";
  else if (path_base.Contains("method-1"))
    str_m = "FCAL_Pi0TOF";
  else if (path_base.Contains("method-2"))
    str_m = "FCAL_Pi0log";
  
  int Npts = 16;
  if (npts < 16)
    Npts = npts;
  
  cout << endl << "FCAL calibration summary plots" << endl;
  cout <<         "==============================" << endl;
  
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
   
  TCanvas *C1;
  
  TPad *SmallC;
  TGaxis *XAxis,*YAxis;
  TLatex XTitle,YTitle,XtopTitle;
  
  TLegend *legend;

  ifstream in;
  ofstream out;
  TString cleg = "";
  //TString base = "edcd";
  TString base = "edc";
  TString path = path_base + Form("%s_%d/", base.Data(), iter);
  if (path_base.Contains("simu"))
    path = path_base + Form("pi0_edc_%d/", iter);
    //path = path_base + Form("eta_edc_%d/", iter);
    //path = path_base + Form("g4_edc_%d/", iter);
    //path = path_base + "g4_corr/";
    
  cout << path << endl;
  TString file_name = path + "runpi0.root";
  TFile * ifile_org = new TFile(file_name);
  cout << file_name << endl;
  // Fit function
  TF1 * fFitFunc;
  TF1 * fgau;
  TF1 * fpol;
  TF1 * fhga;
  TF1 * fsig;
  TF1 * fbkg;
  
  TF1 * bkg_eta_fc;
  TF1 * sig_eta_fc;
  TF1 * tot_eta_fc;
  TF1 * bkg_pi0_fc;
  TF1 * sig_pi0_fc;
  TF1 * tot_pi0_fc;

  TFitResultPtr r;
  
  double fPi0Pos;
  double fPi0Pos_err;
  double pdgPi0Mass = 0.1349766;
  //double min_a = 0.05;
  //double max_a = 7.00;
  double min_a = 0.6;
  //double max_a = 10.0;
  double max_a = 6.0;
  double step_a = (max_a - min_a) / ((double) npts);
  TH1F * h1D[28][npts];
  TH2F * h2D[28];
  TH1F * h_Chi2 = new TH1F("Chi2", ";#font[42]{#chi^{2}/ndf};#font[42]{Count [a.u.]}", 100, 0., 20.);
  TH1F * h_width = new TH1F("width", ";#font[42]{#sigma [MeV/#it{c}^{2}]};#font[42]{Count [a.u.]}", 100, 5., 25.);
  TH1F * h_mean = new TH1F("mean", ";#font[42]{#mu [MeV/#it{c}^{2}]};#font[42]{Count [a.u.]}", 300, 50., 200.);
  TH1F * h_width_err = new TH1F("width_err", ";#font[42]{Error on #sigma [MeV/#it{c}^{2}]};#font[42]{Count [a.u.]}", 100, 0., 2.);
  TH1F * h_mean_err = new TH1F("mean_err", ";#font[42]{Error on #mu [MeV/#it{c}^{2}]};#font[42]{Count [a.u.]}", 100, 0., 2.);
  int ring_nb = 25;
  double par_pol[100][100][100];
  for (int i = 0; i < iter; i ++) {
    TString path_txt = path_base + Form("%s_%d/", base.Data(), i);
    if (path_base.Contains("simu"))
      path_txt = path_base + Form("g4_edc_%d/", i);
    cout << path_txt + Form("energy_dependence_correction_vs_ring_%d.txt", i) << endl;
    in.open(path_txt + Form("energy_dependence_correction_vs_ring_%d.txt", i));
    int rg = 0;
    while ( in.good() ) {
      in>>par_pol[rg][0][i]>>par_pol[rg][1][i]>>par_pol[rg][2][i]>>par_pol[rg][3][i]>>par_pol[rg][4][i]>>par_pol[rg][5][i];
      rg ++;
    }
    in.close();
  }
  
  TGraphErrors * gr_endep[28];
  TGraphErrors * gr_endep_plot[28];
  TString rg_rg_str[] = {"1-2", "3-4", "5", "6-7", "8-9", "10-11", "12-17", "18-20", "21-24"};
  for (int i = 0; i < ring_nb; i ++) {
    cout << "ring " << i << endl;
    gr_endep[i] = new TGraphErrors();
    gr_endep_plot[i] = new TGraphErrors();
    //h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0log/Pi0MassVsE_10_ringl_%d", i));
    //h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0log/Pi0MassVsE_20_ringl_%d", i));
    //h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0log/Pi0Mass2gVsE_30_ringl_%d", i));
    if (path_base.Contains("method0"))
      h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0HFA/Pi0MassVsE_ring_%d", i));
    else if (path_base.Contains("method1"))
      h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0TOF/Pi0MassVsE_rings_%d", i));
    else if (path_base.Contains("method2"))
      h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0log/Pi0MassVsE_ringl_%d", i));
    if (path_base.Contains("method-0"))
      h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0HFA/Pi0MassVsE_ring_%d", i));
    else if (path_base.Contains("method-1"))
      h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0TOF/Pi0MassVsE_rings_%d", i));
    else if (path_base.Contains("method-2"))
      h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0log/Pi0MassVsE_ringl_%d", i));
    //h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0log/Pi0Mass2gVsE_ringl_%d", i));
    
    if (h2D[i] && h2D[i]->GetEntries() > 0) {
      cout << h2D[i]->GetEntries() << endl;
      Nice3D2D(h2D[i],0.05,0.05,42,505,1.4,1.55,1.75,"",
	       "#font[42]{#frac{E_{deposited}^{cluster 1} + E_{deposited}^{cluster 2}}{2} [GeV]}",
	       "#font[42]{#gamma#gamma invariant mass [GeV/#it{c}^{2}]}",
	       "#font[42]{Count [a.u.]}");

      smallBetween1 = .15;
      smallBetween2 = .25;
      smallBetween3 = .05;
      smallBetween4 = .20;
      
      cleg = path + Form("non-linear-correction_sq_%d", i);
      C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
      gPad->SetLeftMargin(smallBetween1);
      gPad->SetRightMargin(smallBetween2);
      gPad->SetTopMargin(smallBetween3);
      gPad->SetBottomMargin(smallBetween4);
      
      h2D[i]->Draw("colz");
      t->DrawLatex(6.0, 0.22, Form("#font[42]{Ring # %d}", i));
      gPad->SetLogz();
      
      C1->Print(cleg + ".pdf");
    }
  }
  //int ring_region_nb = 3;
  //int start[] = {1, 6, 23};
  //int dimen[] = {5, 16, 1};
  int ring_region_nb = 9;
  int start[] = {2, 3, 5, 6, 8, 10, 12, 18, 21};
  int dimen[] = {1, 2, 1, 2, 2,  2,  6,  3,  1};
  TH2F * h2Dfake[30];
  cout <<"making new ring" << endl;
  for (int i = 0; i < ring_region_nb; i ++) {
    h2Dfake[i] = (TH2F *) h2D[start[i]];
    cout << start[i] << endl;
    for (int j = 1; j < dimen[i]; j ++) {
      cout << start[i] + j << endl;
      h2Dfake[i]->Add(h2D[start[i] + j]);
    }
  }
  
  for (int i = 0; i < ring_region_nb; i ++) {
    if (h2Dfake[i] && h2Dfake[i]->GetEntries() > 0) {
      Nice3D2D(h2Dfake[i],0.05,0.05,42,505,1.4,1.55,1.75,"",
	       //"#font[42]{#frac{E_{deposited}^{cluster 1} + E_{deposited}^{cluster 2}}{2} [GeV]}",
	       //"#font[42]{#bar{p}_{#gamma} (|p_{#gamma}^{1}-p_{#gamma}^{2}| < 100 MeV/#it{c}) [GeV/#it{c}]}",
	       "#font[42]{p_{#gamma} [GeV/#it{c}]}",
	       "#font[42]{#gamma#gamma invariant mass [GeV/#it{c}^{2}]}",
	       "#font[42]{Count [a.u.]}");
      
      for (int j = 0; j < npts; j ++) {
	double p_min = min_a + j * step_a;
	double p_max = min_a + (j + 1) * step_a;
	int min_bin = h2Dfake[i]->GetXaxis()->FindBin(p_min);
	int max_bin = h2Dfake[i]->GetXaxis()->FindBin(p_max);
	cout << "i " << i << " j " << j << " p_min " << p_min << " p_max " << p_max << " min bin " << min_bin << " max bin " << max_bin << endl;
	h1D[i][j] = (TH1F *) h2Dfake[i]->ProjectionY(Form("h1D_%d_%d", i, j), min_bin, max_bin);
	Nice1D(h1D[i][j], 0.05, 0.05, 42, 505, 1.1, 1.6, "", "#font[42]{#gamma#gamma invariant mass [GeV/#it{c}^{2}]}", "#font[41]{Counts}");
	//if (h1D[i][j]->GetEntries() < 500)
	//h1D[i][j]->Rebin(10);
      }   
      smallBetween1 = .15;
      smallBetween2 = .25;
      smallBetween3 = .05;
      smallBetween4 = .20;
      
      cleg = path + Form("non-linear-correction_ring_region_%d", i);
      C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
      gPad->SetLeftMargin(smallBetween1);
      gPad->SetRightMargin(smallBetween2);
      gPad->SetTopMargin(smallBetween3);
      gPad->SetBottomMargin(smallBetween4);
      
      h2Dfake[i]->Draw("colz");
      t->DrawLatex(6.0, 0.22, Form("#font[42]{Ring # %s}", rg_rg_str[i].Data()));
      gPad->SetLogz();
      
      C1->Print(cleg + ".pdf");
    
      smallBetween1 = .15;
      smallBetween2 = .05;
      smallBetween3 = .05;
      smallBetween4 = .15;
      int canvas_nb = npts / Npts;
      int k = 0;
      
      for  (int l = 0; l < canvas_nb; l ++) {
	
	cleg = path + Form("non-linear-correction_ring_region_im_%d_nb_%d", i, l);
	C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
	C1->Divide(4,4);
	
	for (int m = 0; m < Npts; m ++) {
	  
	  int j = m + l * Npts;
	  
	  if (j < npts) {
	    double p = min_a + j * step_a + step_a / 2.0;
	    C1->cd(m + 1);
	    gPad->SetLeftMargin(smallBetween1);
	    gPad->SetRightMargin(smallBetween2);
	    gPad->SetTopMargin(smallBetween3);
	    gPad->SetBottomMargin(smallBetween4);

	    if (h1D[i][j] && h1D[i][j]->GetEntries() > 10) {
	      
	      h1D[i][j]->SetMinimum(0);
	      h1D[i][j]->SetLineColor(1);
	      h1D[i][j]->SetMarkerColor(1);
	      h1D[i][j]->SetMarkerStyle(20);
	      h1D[i][j]->SetMarkerSize(0.5);
	      
	      fgau = new TF1(Form("fgau_%d_%d", i, j), "gaus", 0.12, 0.16);
	      fsig = new TF1(Form("fsig_%d_%d", i, j), "crystalball", 0.12, 0.16);
	      //fbkg = new TF1(Form("fbkg_%d_%d", i, j), "exp(pol5)");
	      //fFitFunc = new TF1(Form("fFitFunc_%d_%d", i, j), "crystalball+exp(pol5(5))");
	      fbkg = new TF1(Form("fbkg_%d_%d", i, j), "pol3");
	      fFitFunc = new TF1(Form("fFitFunc_%d_%d", i, j), "crystalball+pol3(5)");
	      fFitFunc->SetLineColor(2);
	      int fBinPi0Pos = h1D[i][j]->FindBin(pdgPi0Mass);
	      double min_mass = 0.08;
	      double max_mass = 0.180;
	      if (i == 0) min_mass = 0.105;
	      fPi0Pos = h1D[i][j]->GetBinCenter(h1D[i][j]->GetMaximumBin());
	      //if (fPi0Pos < 0.135) min_mass = fPi0Pos * 1.1;
	      if (fPi0Pos < 0.100 || fPi0Pos > 0.160) 
		fPi0Pos = 0.135;
	      
	      
	      fPi0Pos = h1D[i][j]->GetBinCenter(h1D[i][j]->GetMaximumBin());
	      if (fPi0Pos < 0.100 || fPi0Pos > 0.160) fPi0Pos = 0.135;
	      double mass_low = h1D[i][j]->GetXaxis()->GetBinCenter(1);
	      if (mass_low < 0.01) mass_low = 0.02;
	      /*if (i > (ring_region_nb - 1)) {
		min_mass = mass_low;
		max_mass = 0.2;
		}*/
	      cout << "min mass " << min_mass << " max mass " << max_mass << endl;
	      h1D[i][j]->GetXaxis()->SetRangeUser(min_mass, max_mass);
	      fPi0Pos = h1D[i][j]->GetBinCenter(h1D[i][j]->GetMaximumBin());
	      
	      Double_t par_tot_fct[13];
	      Double_t height = h1D[i][j]->GetBinCenter(fBinPi0Pos);
	      
	      //if (i > (ring_region_nb - 1))
	      //height = h1D[i][j]->GetMaximum() / 2;
	      Double_t par_sig_fct[5] = {height, fPi0Pos, 0.01, 4000, 1000}; 
	      Double_t par_bkg_fct[6] = {1., 1., 1., 1., 1., 1.};
	      
	      fFitFunc->SetRange(min_mass, max_mass);
	      fbkg->SetRange(min_mass, max_mass);
	      fgau->SetRange(min_mass, max_mass);
	      
	      fsig->SetParameters(par_sig_fct);
	      h1D[i][j]->Fit(fsig, "RBQS0");
	      fsig->GetParameters(&par_tot_fct[0]);
	      h1D[i][j]->Fit(fbkg, "RBQS0");
	      fbkg->GetParameters(&par_tot_fct[5]);
	      fFitFunc->SetParameters(par_tot_fct);
	      fFitFunc->SetParLimits(0, h1D[i][j]->GetMaximum() * 0.05, h1D[i][j]->GetMaximum() * 1.2);
	      //if (i < (ring_region_nb - 1)) {
	      fFitFunc->SetParLimits(1, 0.12, 0.15);
	      fFitFunc->SetParLimits(2, 0.005, 0.020);
	      //} else {
	      //fFitFunc->SetParLimits(1, mass_low, 0.15);
	      //fFitFunc->SetParLimits(2, 0.005, 0.10);
	      //}
	      h1D[i][j]->Fit(fFitFunc, "RMBQ0");
	      fFitFunc->SetParameters(par_tot_fct);
	      r = h1D[i][j]->Fit(fFitFunc, "RMBQS0");

	      double Chi2 = r->Chi2() / (double) r->Ndf();
	      fPi0Pos  = r->Parameter(1);
	      fPi0Pos_err = r->ParError(1);
	      /*if (Chi2 > 10.0) { 
		fFitFunc->SetParameters(par_tot_fct);
		fFitFunc->SetParLimits(0, 0, h1D[i][j]->GetMaximum() * 1.2);
		r = h1D[i][j]->Fit(fFitFunc, "RMBQS0");
		Chi2 = r->Chi2() / (double) r->Ndf();
	      }
	      fPi0Pos  = r->Parameter(1);
	      fPi0Pos_err = r->ParError(1);*/
	      h_Chi2->Fill(Chi2);
	      h_mean->Fill(r->Parameter(1) * 1e3);
	      h_width->Fill(r->Parameter(2) * 1e3);
	      h_mean_err->Fill(r->ParError(1) * 1e3);
	      h_width_err->Fill(r->ParError(2) * 1e3);
	      if (Chi2 < 1000) {
		if (i < (ring_region_nb)) {
		  if (i == 1) {
		    cout << "k "<< k << " p " << p << " gains " <<  fPi0Pos / pdgPi0Mass << " Chi2 " << Chi2 << endl;
		  }
		  gr_endep_plot[i]->SetPoint(k, p, fPi0Pos / pdgPi0Mass);
		  gr_endep_plot[i]->SetPointError(k, 0, fPi0Pos_err / pdgPi0Mass);
		  k ++;
		} /*else if (i >= (ring_region_nb - 1)) {
		  gr_endep_plot[i]->SetPoint(k, p, fPi0Pos / 0.1);
		  gr_endep_plot[i]->SetPointError(k, 0, fPi0Pos_err / 0.1);
		  k ++;
		  }*/
	      }
	      
	      h1D[i][j]->Draw("p");
	      t->DrawLatex(0.15, ((double) h1D[i][j]->GetMaximum()), Form("#font[42]{p_{#gamma} = %0.2f GeV}", p));
	      //if (i < 25)
	      //h1D[i][j]->GetXaxis()->SetRangeUser(0.120, 0.180);
	      //else
	      //h1D[i][j]->GetXaxis()->SetRangeUser(0.050, 0.20);
	      //fPi0Pos = h1D[i][j]->GetBinCenter(h1D[i][j]->GetMaximumBin());
	      TLine.SetLineWidth(2);
	      TLine.SetLineStyle(2);
	      TLine.SetLineColor(4);
	      
	      TLine.DrawLine(fPi0Pos, 0, fPi0Pos, h1D[i][j]->GetMaximum() + 20);
	      
	      if (fFitFunc) {
		if (Chi2 >= 20.0) {
		  fFitFunc->SetLineColor(4);
		} else {
		  fFitFunc->SetLineColor(2);
		}
		fFitFunc->Draw("same");
	      }
	    }
	  }
	}
	C1->Print(cleg + ".pdf");
      }
    }
  }

  //TH2F * DrawC = new TH2F("DrawC", ";#font[42]{p_{#gamma} [GeV]};#font[42]{#mu / m_{PDG}};", 100, min_a, max_a - 0.001, 100, 0.92, 1.09999);
  TH2F * DrawC = new TH2F("DrawC", ";#font[42]{p_{#gamma} [GeV]};#font[42]{#mu / m_{PDG}};", 100, 0.0, 9.99, 100, 0.92, 1.09999);
  Nicer2D(DrawC, 0.05, 42, 505, 1.2, 1.4, 1.2);
  
  smallBetween1 = .15;
  smallBetween2 = .025;
  smallBetween3 = .025;
  smallBetween4 = .15;
  cout <<path + Form("energy_dependence_correction_vs_ring_%d.txt", iter) << endl;
  out.open(path + Form("energy_dependence_correction_vs_ring_%d.txt", iter));
  for (int j = 0; j < 2; j ++) {
  
    cleg = path + Form("Corr_vs_p_%d", j);
    C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
  
    DrawC->Draw();
    
    for (int l = 0; l < 6; l ++) {
      
      int i = l + j * 6;
      cout <<"j " << j << " l " << l << " i " << i << endl;
      if (i < ring_region_nb) {
	int color = l + 1;
	if (color == 5) color = kOrange;
	if (gr_endep_plot[i] && gr_endep_plot[i]->GetN() > 0) {
	  min_a = 1.0;
	  max_a = 3.5;//gr_endep_plot[i]->GetX()[gr_endep_plot[i]->GetN() - 1];
	  fpol = new TF1(Form("fpol_plot_%d", i), "[0]-exp(-[1]*x+[2])", min_a, max_a);
	  //fhga = new TF1(Form("fhga_plot_%d", i), "[0]-exp(-[1]*x-[2]*x*x-[3]*x*x*x+[4])", 0.7, 1.8);
	  fpol->SetParameters(1, 1, 1);
	  fpol->SetLineColor(color);
	  //fhga->SetParameters(1, 1, 1, 1, 1);
	  //fhga->SetLineColor(color);
	  //fhga->SetLineStyle(3);
	  
	  gr_endep_plot[i]->Fit(fpol, "RB+");
	  //gr_endep_plot[i]->Fit(fhga, "RB+");
	  
	  if (fpol->GetParameter(0) < 2.0) {
	    for (int z = 0; z < 3; z ++) {	    
	      par_pol[i][z][iter] = fpol->GetParameter(z);
	    }
	    for (int z = 0; z < 6; z ++) {
	      out.precision(7);
	      out.width(20);
	      if (z < 3)
		out<<fpol->GetParameter(z);
	      else
		out<<0;
	      //out<<0;
	    }
	    out<<endl;
	  } else {
	    out.precision(7);
	    out.width(20);
	    out<<0;
	    for (int z = 1; z < 6; z ++) {
	      out.precision(7);
	      out.width(20);
	      out<<0;
	    }
	    out<<endl;
	  }
	  for (int z = 0; z < gr_endep_plot[i]->GetN(); z ++) {
	    double p = gr_endep_plot[i]->GetX()[z];
	    double err = gr_endep_plot[i]->GetEY()[z];
	    double corr = 1;
	    for (int r = 0; r < (iter+1); r ++) {
	      //corr *= par_pol[i][0][r] + par_pol[i][1][r] * p + par_pol[i][2][r] * pow(p, 2) + par_pol[i][3][r] * pow(p, 3) + par_pol[i][4][r] * pow(p, 4) + par_pol[i][5][r] * pow(p, 5);
	      //corr *= par_pol[i][0][r] + par_pol[i][1][r] * p + par_pol[i][2][r] * pow(p, 2);
	      //double exp_coef = -par_pol[i][1][r] * p + par_pol[i][2][r]
	      corr *= par_pol[i][0][r] - TMath::Exp(-par_pol[i][1][r] * p + par_pol[i][2][r]);
	    }
	    cout <<"ring " << i <<" p " << p << " corr " << corr << endl;
	    gr_endep[i]->SetPoint(z, p, corr);
	    gr_endep[i]->SetPointError(z, 0, err * corr);
	  }
	  gr_endep_plot[i]->SetMarkerStyle(20 + l);
	  gr_endep_plot[i]->SetMarkerSize(1.2);
	  gr_endep_plot[i]->SetMarkerColor(color);
	  gr_endep_plot[i]->SetLineColor(color);
	  gr_endep_plot[i]->Draw("PSZ");
	  fpol->SetRange(0, 10);
	  fpol->SetLineStyle(2);
	  fpol->Draw("same");

	  //fhga->SetRange(3.5, 10);
	  //fhga->SetLineStyle(3);
	  //fhga->Draw("same");
	} else {
	  out.precision(7);
	  out.width(20);
	  out<<1;
	  for (int z = 1; z < 6; z ++) {
	    out.precision(7);
	    out.width(20);
	    out<<0;
	  }
	  out<<endl;
	}
	TLine.SetLineColor(2);
	TLine.SetLineWidth(2);
	TLine.SetLineStyle(2);
	TLine.DrawLine(min_a, 1.0, max_a - 0.001, 1.0);
	
      }
      legend=new TLegend(0.225,0.75,0.35,0.95);
      for (int l = 0; l < 6; l ++) {
	int i = l + j * 6;
	cout <<"j " << j << " l " << l << " i " << i << endl;
	if (i < ring_region_nb) {
	  legend->AddEntry(gr_endep_plot[i], TString::Format("#font[41]{Ring # %s}", rg_rg_str[i].Data()),"p");
	}
      }
    }
    legend->SetFillColor(0);
    legend->SetTextFont(42);
    legend->SetTextSize(.03);
    legend->SetLineColor(0);
    legend->Draw("same");
    C1->Print(cleg + ".pdf");
  }
  for (int l = 0; l < 15; l ++) {
    out.precision(7);
    out.width(20);
    out<<2;
    for (int z = 1; z < 6; z ++) {
      int va = 0;
      if (z == 3) va = 1;
      out.precision(7);
      out.width(20);
      out<<va;
    }
    out<<endl;
  }
  out.close();
  //
  out.open(path + Form("energy_dependence_correction_vs_ring_multi_%d.txt", iter));
  for (int j = 0; j < 2; j ++) {
  
    cleg = path + Form("Corr_multo_vs_p_%d", j);
    C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
  
    DrawC->Draw();
    
    for (int l = 0; l < 6; l ++) {
      
      int i = l + j * 6;
      cout <<"j " << j << " l " << l << " i " << i << endl;
      if (i < ring_region_nb) {
	int color = l + 1;
	if (color == 5) color = kOrange;
	if (gr_endep[i] && gr_endep[i]->GetN() > 0) {
	  min_a = 1.0;
	  max_a = 3.5;
	  fpol = new TF1(Form("fpol_%d", i), "[0]-exp(-[1]*x+[2])", min_a, max_a);
	  fpol->SetParameters(1, 1, 1);
	  fpol->SetLineColor(color);
	  gr_endep[i]->Fit(fpol, "RB+");
	  
	  if (fpol->GetParameter(0) < 2.0) {
	    for (int z = 0; z < 3; z ++) {	    
	      par_pol[i][z][iter] = fpol->GetParameter(z);
	    }
	    for (int z = 0; z < 6; z ++) {
	      out.precision(7);
	      out.width(20);
	      if (z < 3)
		out<<fpol->GetParameter(z);
	      else
		out<<0;
	    }
	    out<<endl;
	  } else {
	    out.precision(7);
	    out.width(20);
	    out<<1;
	    for (int z = 1; z < 6; z ++) {
	      out.precision(7);
	      out.width(20);
	      out<<0;
	    }
	    out<<endl;
	  }
	  gr_endep[i]->SetMarkerStyle(20 + l);
	  gr_endep[i]->SetMarkerSize(1.2);
	  gr_endep[i]->SetMarkerColor(color);
	  gr_endep[i]->SetLineColor(color);
	  gr_endep[i]->Draw("PSZ");
	  fpol->SetRange(0, 10);
	  fpol->SetLineStyle(2);
	  fpol->Draw("same");	  

	} else {
	  out.precision(7);
	  out.width(20);
	  out<<0;
	  for (int z = 1; z < 6; z ++) {
	    out.precision(7);
	    out.width(20);
	    out<<0;
	  }
	  out<<endl;
	}
	TLine.SetLineColor(2);
	TLine.SetLineWidth(2);
	TLine.SetLineStyle(2);
	TLine.DrawLine(min_a, 1.0, max_a - 0.001, 1.0);
	
      }
      legend=new TLegend(0.225,0.75,0.35,0.95);
      for (int l = 0; l < 6; l ++) {
	int i = l + j * 6;
	cout <<"j " << j << " l " << l << " i " << i << endl;
	if (i < ring_region_nb) {
	  legend->AddEntry(gr_endep[i], TString::Format("#font[41]{Ring # %d}", i),"p");
	}
      }
    }
    legend->SetFillColor(0);
    legend->SetTextFont(42);
    legend->SetTextSize(.03);
    legend->SetLineColor(0);
    legend->Draw("same");
    C1->Print(cleg + ".pdf");
  }
  for (int l = 0; l < 15; l ++) {
    out.precision(7);
    out.width(20);
    out<<2;
    for (int z = 1; z < 6; z ++) {
      int va = 0;
      if (z == 3) va = 1;
      out.precision(7);
      out.width(20);
      out<<va;
    }
    out<<endl;
  }
  out.close();
  //
  
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .15;
  cleg = path + "Chi2_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_Chi2, 0.05, 42, 504, 1.1, 1.5);
  h_Chi2->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_width, 0.05, 42, 504, 1.1, 1.5);
  h_width->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Mean_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_mean, 0.05, 42, 504, 1.1, 1.5);
  h_mean->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_err_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_width_err, 0.05, 42, 504, 1.1, 1.5);
  h_width_err->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Mean_err_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_mean_err, 0.05, 42, 504, 1.1, 1.5);
  h_mean_err->Draw();
  
  C1->Print(cleg + ".pdf");


  const int sq_nb = 25;
  TH1F * h1Dr[sq_nb];
  TGraphErrors * grFitPars_rg_eta[2];
  TGraphErrors * grFitPars_rg_pi0[2];
  
  for (int i = 0; i < 2; i ++) {
    grFitPars_rg_eta[i] = new TGraphErrors();
    grFitPars_rg_pi0[i] = new TGraphErrors();
    grFitPars_rg_eta[i]->SetName(Form("grFitPars_rg_eta_%d", i));
    grFitPars_rg_pi0[i]->SetName(Form("grFitPars_rg_pi0_%d", i));
  }
  
  for (int i = 0; i < sq_nb; i ++) {
    if (i != 0) {
      TString histo = Form(str_m + "/Pi0Mass_ring_%d", i);
      if (i > 21)
	histo = Form(str_m + "/Pi0Mass2g_ring_%d", i);
      cout << histo << endl;
      h1Dr[i] = (TH1F *) ifile_org->Get(histo);
      if (i > 21)
	h1Dr[i]->Rebin(5);
      
      h1Dr[i]->SetMarkerStyle(20);
      h1Dr[i]->SetMarkerColor(1);
      h1Dr[i]->SetLineColor(1);
      h1Dr[i]->SetMarkerSize(1.0);
      Nice1D(h1Dr[i], 0.05, 0.05, 42,505,1.,1.8,"","#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]}","#font[41]{Counts [a.u.]}");

      bkg_eta_fc  = new TF1(Form("bkg_eta_fc_%d", i),"pol3",0.440,0.700);
      sig_eta_fc  = new TF1(Form("sig_eta_fc_%d", i),"gaus",0.500,0.600);
      tot_eta_fc  = new TF1(Form("tot_eta_fc_%d", i),"gaus + pol3(3)",0.440,0.700);
            
      tot_eta_fc->SetLineColor(2);
      tot_eta_fc->SetLineWidth(2);
      bkg_eta_fc->SetLineColor(3);
      bkg_eta_fc->SetLineWidth(1);
      double peak_init = 0.536;
      double width_init = 0.01;
      Double_t par_tot_eta_fc[7], par_sig_eta_fc[3] = {0.,peak_init,width_init}, par_bkg_eta_fc[4] = {1.,1.,1.,1.};
      tot_eta_fc->SetParameters(par_tot_eta_fc);
      sig_eta_fc->SetParameters(par_sig_eta_fc);
      bkg_eta_fc->SetParameters(par_bkg_eta_fc);
      
      h1Dr[i]->Fit(bkg_eta_fc, "RMBQS0");
      bkg_eta_fc->GetParameters(&par_tot_eta_fc[3]);
      h1Dr[i]->Fit(sig_eta_fc, "RMBQS0");
      sig_eta_fc->GetParameters(&par_tot_eta_fc[0]);
      tot_eta_fc->SetParameters(par_tot_eta_fc);
      tot_eta_fc->SetParLimits(1,0.510,0.550);
      tot_eta_fc->SetParLimits(2,0.001,0.03);
      
      h1Dr[i]->Fit(tot_eta_fc,"RMBQS0");
      tot_eta_fc->GetParameters(&par_tot_eta_fc[0]);
      bkg_eta_fc->SetParameters(&par_tot_eta_fc[3]);
      /*
      bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"pol4",0.096,0.18);
      sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + pol4(5)",0.096,0.18);
      */
      bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"exp(pol3)",0.09,0.225);
      //bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"pol3",0.1,0.205);
      if (i == 23)
	sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.10,0.13);
      else
	sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + exp(pol3(5))",0.09,0.225);
      //tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + pol3(5)",0.1,0.205);
      tot_pi0_fc->SetLineColor(2);
      tot_pi0_fc->SetLineWidth(2);
      bkg_pi0_fc->SetLineColor(3);
      bkg_pi0_fc->SetLineWidth(1);
      peak_init = 0.135;
      width_init = 0.008;
      /*
      if (i == 23) {
	peak_init = 0.105;
	width_init = 0.028;
      }
      */
      Double_t par_tot_pi0_fc[11], par_sig_pi0_fc[5] = {0., peak_init, width_init,3.58653e+02,1.53665e+03}, par_bkg_pi0_fc[6] = {1., 1., 1., 1., 1., 1.};
      tot_pi0_fc->SetParameters(par_tot_pi0_fc);
      sig_pi0_fc->SetParameters(par_sig_pi0_fc);
      bkg_pi0_fc->SetParameters(par_bkg_pi0_fc);
      
      h1Dr[i]->Fit(bkg_pi0_fc, "RMBQS0");
      bkg_pi0_fc->GetParameters(&par_tot_pi0_fc[5]);
      h1Dr[i]->Fit(sig_pi0_fc, "RMBQS0");
      sig_pi0_fc->GetParameters(&par_tot_pi0_fc[0]);
      tot_pi0_fc->SetParameters(par_tot_pi0_fc);
      if (i == 23)
	tot_pi0_fc->SetParLimits(1,0.100,0.140);
      else
	tot_pi0_fc->SetParLimits(1,0.130,0.140);
      if (i == 23)
	tot_pi0_fc->SetParLimits(2,0.005,0.030);
      else
	tot_pi0_fc->SetParLimits(2,0.005,0.015);
      
      h1Dr[i]->Fit(tot_pi0_fc,"RMBQS0");
      tot_pi0_fc->GetParameters(&par_tot_pi0_fc[0]);
      bkg_pi0_fc->SetParameters(&par_tot_pi0_fc[5]);

      grFitPars_rg_pi0[0]->SetPoint(i, (double) i, tot_pi0_fc->GetParameter(1) * 1e3);
      grFitPars_rg_pi0[0]->SetPointError(i, 0, tot_pi0_fc->GetParError(1) * 1e3);
      grFitPars_rg_eta[0]->SetPoint(i, (double) i, tot_eta_fc->GetParameter(1) * 1e3);
      grFitPars_rg_eta[0]->SetPointError(i, 0, tot_eta_fc->GetParError(1) * 1e3);
      grFitPars_rg_pi0[1]->SetPoint(i, (double) i, tot_pi0_fc->GetParameter(2) * 1e3);
      grFitPars_rg_pi0[1]->SetPointError(i, 0, tot_pi0_fc->GetParError(2) * 1e3);
      grFitPars_rg_eta[1]->SetPoint(i, (double) i, tot_eta_fc->GetParameter(2) * 1e3);
      grFitPars_rg_eta[1]->SetPointError(i, 0, tot_eta_fc->GetParError(2) * 1e3);
      
      cleg = Form("im_vs_rg_%d", i);
      C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
      gPad->SetLeftMargin(smallBetween1);
      gPad->SetRightMargin(smallBetween2);
      gPad->SetTopMargin(smallBetween3);
      gPad->SetBottomMargin(smallBetween4);
      //if (i == 24) h1Dr[i]->SetMaximum(1e2);
      //if (i == 24) h1Dr[i]->SetMaximum(1e4);
      h1Dr[i]->SetMinimum(1);
      h1Dr[i]->Draw();

      tot_pi0_fc->Draw("same");
      bkg_pi0_fc->Draw("same");
      tot_eta_fc->Draw("same");
      bkg_eta_fc->Draw("same");

      gPad->SetLogy();
      double max = h1Dr[i]->GetMaximum();
      t->SetTextSize(0.03);
      t->DrawLatex(0.35, max/1.0, Form("#font[42]{#mu_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(1) * 1e3, tot_pi0_fc->GetParError(1) * 1e3));
      t->DrawLatex(0.35, max/2.0, Form("#font[42]{#sigma_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(2) * 1e3, tot_pi0_fc->GetParError(2) * 1e3));
      t->DrawLatex(0.35, max/4.0, Form("#font[42]{#mu_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(1) * 1e3, tot_eta_fc->GetParError(1) * 1e3));
      t->DrawLatex(0.35, max/8.0, Form("#font[42]{#sigma_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(2) * 1e3, tot_eta_fc->GetParError(2) * 1e3));
      
      TLine.SetLineColor(2);
      TLine.DrawLine(fPi0Pos, 0, fPi0Pos, h1Dr[i]->GetMaximum() * 1.2);
      C1->Print(path + cleg + ".pdf");
    }
  }
  
  TH2F * DrawM_rg_pi0 = new TH2F("DrawM_rg_pi0", ";#font[42]{Ring #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, 0.0, 24.99, 100, 128, 135.99);
  TH2F * DrawW_rg_pi0 = new TH2F("DrawW_rg_pi0", ";#font[42]{Ring #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, 0.0, 24.99, 100, 7.0, 8.99);
  Nicer2D(DrawM_rg_pi0, 0.05, 42, 505, 1.2, 1.4, 1.2);
  Nicer2D(DrawW_rg_pi0, 0.05, 42, 505, 1.2, 1.4, 1.2);

  TH2F * DrawM_rg_eta = new TH2F("DrawM_rg_eta", ";#font[42]{Ring #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, 0.0, 24.99, 100, 530, 549.99);
  TH2F * DrawW_rg_eta = new TH2F("DrawW_rg_eta", ";#font[42]{Ring #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, 0.0, 24.99, 100, 10.0, 29.99);
  Nicer2D(DrawM_rg_eta, 0.05, 42, 505, 1.2, 1.4, 1.2);
  Nicer2D(DrawW_rg_eta, 0.05, 42, 505, 1.2, 1.4, 1.2);
  
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .15;
  cleg = path + "Mean_vs_ring_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawM_rg_pi0->Draw();
  grFitPars_rg_pi0[0]->SetMarkerStyle(20);
  grFitPars_rg_pi0[0]->SetMarkerSize(1.2);
  grFitPars_rg_pi0[0]->SetMarkerColor(1);
  grFitPars_rg_pi0[0]->SetLineColor(1);
  grFitPars_rg_pi0[0]->Draw("PSZ");
  TLine.SetLineColor(2);
  TLine.SetLineWidth(2);
  TLine.SetLineStyle(2);
  TLine.DrawLine(0, 134.97, 24.99, 134.97);
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_vs_ring_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawW_rg_pi0->Draw();
  grFitPars_rg_pi0[1]->SetMarkerStyle(20);
  grFitPars_rg_pi0[1]->SetMarkerSize(1.2);
  grFitPars_rg_pi0[1]->SetMarkerColor(1);
  grFitPars_rg_pi0[1]->SetLineColor(1);
  grFitPars_rg_pi0[1]->Draw("PSZ");
  
  C1->Print(cleg + ".pdf");
    
  cleg = path + "Mean_vs_ring_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawM_rg_eta->Draw();
  grFitPars_rg_eta[0]->SetMarkerStyle(20);
  grFitPars_rg_eta[0]->SetMarkerSize(1.2);
  grFitPars_rg_eta[0]->SetMarkerColor(1);
  grFitPars_rg_eta[0]->SetLineColor(1);
  grFitPars_rg_eta[0]->Draw("PSZ");
  TLine.SetLineColor(2);
  TLine.SetLineWidth(2);
  TLine.SetLineStyle(2);
  TLine.DrawLine(0, 547.43, 24.99, 547.43);
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_vs_ring_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawW_rg_eta->Draw();
  grFitPars_rg_eta[1]->SetMarkerStyle(20);
  grFitPars_rg_eta[1]->SetMarkerSize(1.2);
  grFitPars_rg_eta[1]->SetMarkerColor(1);
  grFitPars_rg_eta[1]->SetLineColor(1);
  grFitPars_rg_eta[1]->Draw("PSZ");
  C1->Print(cleg + ".pdf");

  
  
  TGraphErrors * grFitPars_eta[2];
  TGraphErrors * grFitPars_pi0[2];
  
  for (int i = 0; i < 2; i ++) {
    grFitPars_eta[i] = new TGraphErrors();
    grFitPars_pi0[i] = new TGraphErrors();
    grFitPars_eta[i]->SetName(Form("grFitPars_eta_%d", i));
    grFitPars_pi0[i]->SetName(Form("grFitPars_pi0_%d", i));
  }
  
  TFile * f_aaMass[100];
  TH1F * h_aaMass[100];

  for (int i = 0; i < iter; i ++) {
    
    if (i < iter) {
      f_aaMass[i] = new TFile (Form(path + "../edc_%d/runpi0.root", i + 1));
      h_aaMass[i] = (TH1F *) f_aaMass[i]->Get(str_m + "/Pi0Mass");
      h_aaMass[i]->SetName(Form("Pi0Mass_%d", i));
      Nice1D(h_aaMass[i], 0.05, 0.05, 42, 505, 1.1, 1.3, "", "#font[42]{#gamma#gamma invariant mass [GeV/#it{c}^{2}]}", "#font[41]{Counts}");
      
      bkg_eta_fc  = new TF1(Form("bkg_eta_fc_%d", i),"pol3",0.440,0.700);
      sig_eta_fc  = new TF1(Form("sig_eta_fc_%d", i),"gaus",0.500,0.600);
      tot_eta_fc  = new TF1(Form("tot_eta_fc_%d", i),"gaus + pol3(3)",0.440,0.700);
            
      tot_eta_fc->SetLineColor(2);
      tot_eta_fc->SetLineWidth(2);
      bkg_eta_fc->SetLineColor(3);
      bkg_eta_fc->SetLineWidth(1);
      double peak_init = 0.536;
      double width_init = 0.01;
      Double_t par_tot_eta_fc[7], par_sig_eta_fc[3] = {0.,peak_init,width_init}, par_bkg_eta_fc[4] = {1.,1.,1.,1.};
      tot_eta_fc->SetParameters(par_tot_eta_fc);
      sig_eta_fc->SetParameters(par_sig_eta_fc);
      bkg_eta_fc->SetParameters(par_bkg_eta_fc);
      
      h_aaMass[i]->Fit(bkg_eta_fc, "RMBQS0");
      bkg_eta_fc->GetParameters(&par_tot_eta_fc[3]);
      h_aaMass[i]->Fit(sig_eta_fc, "RMBQS0");
      sig_eta_fc->GetParameters(&par_tot_eta_fc[0]);
      tot_eta_fc->SetParameters(par_tot_eta_fc);
      tot_eta_fc->SetParLimits(1,0.510,0.550);
      tot_eta_fc->SetParLimits(2,0.001,0.03);
      
      h_aaMass[i]->Fit(tot_eta_fc,"RMBQS0");
      tot_eta_fc->GetParameters(&par_tot_eta_fc[0]);
      bkg_eta_fc->SetParameters(&par_tot_eta_fc[3]);
      /*
      bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"pol4",0.096,0.18);
      sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + pol4(5)",0.096,0.18);
      */
      bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"exp(pol3)",0.09,0.225);
      sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + exp(pol3(5))",0.09,0.225);
      tot_pi0_fc->SetLineColor(2);
      tot_pi0_fc->SetLineWidth(2);
      bkg_pi0_fc->SetLineColor(3);
      bkg_pi0_fc->SetLineWidth(1);
      peak_init = 0.135;
      width_init = 0.008;
      Double_t par_tot_pi0_fc[11], par_sig_pi0_fc[5] = {0., peak_init, width_init,3.58653e+02,1.53665e+03}, par_bkg_pi0_fc[6] = {1., 1., 1., 1., 1., 1.};
      tot_pi0_fc->SetParameters(par_tot_pi0_fc);
      sig_pi0_fc->SetParameters(par_sig_pi0_fc);
      bkg_pi0_fc->SetParameters(par_bkg_pi0_fc);
      
      h_aaMass[i]->Fit(bkg_pi0_fc, "RMBQS0");
      bkg_pi0_fc->GetParameters(&par_tot_pi0_fc[5]);
      h_aaMass[i]->Fit(sig_pi0_fc, "RMBQS0");
      sig_pi0_fc->GetParameters(&par_tot_pi0_fc[0]);
      tot_pi0_fc->SetParameters(par_tot_pi0_fc);
      tot_pi0_fc->SetParLimits(1,0.130,0.140);
      tot_pi0_fc->SetParLimits(2,0.005,0.015);
      
      h_aaMass[i]->Fit(tot_pi0_fc,"RMBQS0");
      tot_pi0_fc->GetParameters(&par_tot_pi0_fc[0]);
      bkg_pi0_fc->SetParameters(&par_tot_pi0_fc[5]);

      grFitPars_pi0[0]->SetPoint(i, (double) i, tot_pi0_fc->GetParameter(1) * 1e3);
      grFitPars_pi0[0]->SetPointError(i, 0, tot_pi0_fc->GetParError(1) * 1e3);
      grFitPars_eta[0]->SetPoint(i, (double) i, tot_eta_fc->GetParameter(1) * 1e3);
      grFitPars_eta[0]->SetPointError(i, 0, tot_eta_fc->GetParError(1) * 1e3);
      grFitPars_pi0[1]->SetPoint(i, (double) i, tot_pi0_fc->GetParameter(2) * 1e3);
      grFitPars_pi0[1]->SetPointError(i, 0, tot_pi0_fc->GetParError(2) * 1e3);
      grFitPars_eta[1]->SetPoint(i, (double) i, tot_eta_fc->GetParameter(2) * 1e3);
      grFitPars_eta[1]->SetPointError(i, 0, tot_eta_fc->GetParError(2) * 1e3);

      smallBetween1 = .15;
      smallBetween2 = .05;
      smallBetween3 = .05;
      smallBetween4 = .15;
      cleg = path + Form("aaMass_iter_%d", i);
      C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
      gPad->SetLeftMargin(smallBetween1);
      gPad->SetRightMargin(smallBetween2);
      gPad->SetTopMargin(smallBetween3);
      gPad->SetBottomMargin(smallBetween4);

      h_aaMass[i]->SetLineColor(1);
      h_aaMass[i]->SetLineWidth(2);
      h_aaMass[i]->SetMinimum(1e2);
      h_aaMass[i]->Draw();
      tot_pi0_fc->Draw("same");
      bkg_pi0_fc->Draw("same");
      tot_eta_fc->Draw("same");
      bkg_eta_fc->Draw("same");
      gPad->SetLogy();
      double max = h_aaMass[i]->GetMaximum();
      t->SetTextSize(0.03);
      t->DrawLatex(0.35, max/1.0, Form("#font[42]{#mu_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(1) * 1e3, tot_pi0_fc->GetParError(1) * 1e3));
      t->DrawLatex(0.35, max/2.0, Form("#font[42]{#sigma_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(2) * 1e3, tot_pi0_fc->GetParError(2) * 1e3));
      t->DrawLatex(0.35, max/4.0, Form("#font[42]{#mu_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(1) * 1e3, tot_eta_fc->GetParError(1) * 1e3));
      t->DrawLatex(0.35, max/8.0, Form("#font[42]{#sigma_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(2) * 1e3, tot_eta_fc->GetParError(2) * 1e3));
      C1->Print(cleg + ".pdf");
    }
  }
    
  TH2F * DrawM_pi0 = new TH2F("DrawM_pi0", ";#font[42]{Iteration #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, 0.0, (double) (iter + 1), 100, 128, 135.99);
  TH2F * DrawW_pi0 = new TH2F("DrawW_pi0", ";#font[42]{Iteration #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, 0.0, (double) (iter + 1), 100, 7.0, 9.99);
  Nicer2D(DrawM_pi0, 0.05, 42, 505, 1.2, 1.4, 1.2);
  Nicer2D(DrawW_pi0, 0.05, 42, 505, 1.2, 1.4, 1.2);

  TH2F * DrawM_eta = new TH2F("DrawM_eta", ";#font[42]{Iteration #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, 0.0, (double) (iter + 1), 100, 530, 549.99);
  TH2F * DrawW_eta = new TH2F("DrawW_eta", ";#font[42]{Iteration #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, 0.0, (double) (iter + 1), 100, 10.0, 29.99);
  Nicer2D(DrawM_eta, 0.05, 42, 505, 1.2, 1.4, 1.2);
  Nicer2D(DrawW_eta, 0.05, 42, 505, 1.2, 1.4, 1.2);
  
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .15;
  cleg = path + "Mean_vs_iteration_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawM_pi0->Draw();
  grFitPars_pi0[0]->SetMarkerStyle(20);
  grFitPars_pi0[0]->SetMarkerSize(1.2);
  grFitPars_pi0[0]->SetMarkerColor(1);
  grFitPars_pi0[0]->SetLineColor(1);
  grFitPars_pi0[0]->Draw("PSZ");
  TLine.SetLineColor(2);
  TLine.SetLineWidth(2);
  TLine.SetLineStyle(2);
  TLine.DrawLine(0, 134.97, (double) (iter + 1), 134.97);
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_vs_iteration_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawW_pi0->Draw();
  grFitPars_pi0[1]->SetMarkerStyle(20);
  grFitPars_pi0[1]->SetMarkerSize(1.2);
  grFitPars_pi0[1]->SetMarkerColor(1);
  grFitPars_pi0[1]->SetLineColor(1);
  grFitPars_pi0[1]->Draw("PSZ");
  
  C1->Print(cleg + ".pdf");
    
  cleg = path + "Mean_vs_iteration_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawM_eta->Draw();
  grFitPars_eta[0]->SetMarkerStyle(20);
  grFitPars_eta[0]->SetMarkerSize(1.2);
  grFitPars_eta[0]->SetMarkerColor(1);
  grFitPars_eta[0]->SetLineColor(1);
  grFitPars_eta[0]->Draw("PSZ");
  TLine.SetLineColor(2);
  TLine.SetLineWidth(2);
  TLine.SetLineStyle(2);
  TLine.DrawLine(0, 547.43, (double) (iter + 1), 547.43);
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_vs_iteration_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawW_eta->Draw();
  grFitPars_eta[1]->SetMarkerStyle(20);
  grFitPars_eta[1]->SetMarkerSize(1.2);
  grFitPars_eta[1]->SetMarkerColor(1);
  grFitPars_eta[1]->SetLineColor(1);
  grFitPars_eta[1]->Draw("PSZ");
  C1->Print(cleg + ".pdf");
    
  TFile * outFile = new TFile(path + "energy-correction.root","RECREATE");
  h_Chi2->Write();
  h_mean->Write();
  h_mean_err->Write();
  h_width->Write();
  h_width_err->Write();

  for (int i = 0; i < 2; i ++) {
    grFitPars_eta[i]->Write();
    grFitPars_pi0[i]->Write();
  }

  for (int i = 0; i < 2; i ++) {
    grFitPars_rg_eta[i]->Write();
    grFitPars_rg_pi0[i]->Write();
  }
  
  for (int i = 0; i < ring_region_nb; i ++) {
    for (int j = 0; j < npts; j ++) {
      if (h1D[i][j] && h1D[i][j]->GetEntries() > 0)
	h1D[i][j]->Write();
    }
    if (gr_endep_plot[i] && gr_endep_plot[i]->GetN() > 0) {
      gr_endep_plot[i]->SetName(Form("gr_endep_plot_%d", i));
      gr_endep_plot[i]->Write();
    }
    if (gr_endep[i] && gr_endep[i]->GetN() > 0) {
      gr_endep[i]->SetName(Form("gr_endep_%d", i));
      gr_endep[i]->Write();
    }
  }
  outFile->Write();
  outFile->Close();

}
