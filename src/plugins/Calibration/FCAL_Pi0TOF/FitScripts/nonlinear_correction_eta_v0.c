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

void nonlinear_correction_eta_v0(TString path_base = "", int npts = 20, int iter = 0){

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
    //path = path_base + Form("g4_edc_%d/", iter);
    path = path_base + Form("eta_edc_%d/", iter);
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

  TFitResultPtr r;
  
  double fPi0Pos;
  double fPi0Pos_err;
  double pdgPi0Mass = 0.1349766;
  double pdgEtaMass = 0.54743;
  //double min_a = 0.05;
  //double max_a = 7.00;
  double min_a = 0.6;
  double max_a = 10.0;
  //double max_a = 6.0;
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
    cout << path_txt + Form("energy_dependence_correction_vs_ring_eta_%d.txt", i) << endl;
    in.open(path_txt + Form("energy_dependence_correction_vs_ring_eta_%d.txt", i));
    int rg = 0;
    while ( in.good() ) {
      in>>par_pol[rg][0][i]>>par_pol[rg][1][i]>>par_pol[rg][2][i]>>par_pol[rg][3][i]>>par_pol[rg][4][i]>>par_pol[rg][5][i];
      rg ++;
    }
    in.close();
  }
  
  TGraphErrors * gr_endep[28];
  TGraphErrors * gr_endep_plot[28];
  
  for (int i = 0; i < ring_nb; i ++) {
    cout << "ring " << i << endl;
    gr_endep[i] = new TGraphErrors();
    gr_endep_plot[i] = new TGraphErrors();
    //h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0log/Pi0MassVsE_30_ringl_%d", i));
    h2D[i] = (TH2F *) ifile_org->Get(Form("FCAL_Pi0log/Pi0MassVsE_ringl_%d", i));

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
      
      cleg = path + Form("non-linear-correction_sq_eta_%d", i);
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
  TString rg_rg_str[] = {"1-2", "3-4", "5", "6-7", "8-9", "10-11", "12-17", "18-20", "21-24"};
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
      
      cleg = path + Form("non-linear-correction_ring_region_eta_%d", i);
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
	
	cleg = path + Form("non-linear-correction_ring_region_im_eta_%d_nb_%d", i, l);
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
	      
	      fgau = new TF1(Form("fgau_%d_%d", i, j), "gaus", 0.44, 0.7);
	      fsig = new TF1(Form("fsig_%d_%d", i, j), "gaus", 0.5, 0.6);
	      //fbkg = new TF1(Form("fbkg_%d_%d", i, j), "exp(pol5)");
	      //fFitFunc = new TF1(Form("fFitFunc_%d_%d", i, j), "crystalball+exp(pol5(5))");
	      fbkg = new TF1(Form("fbkg_%d_%d", i, j), "pol3");
	      fFitFunc = new TF1(Form("fFitFunc_%d_%d", i, j), "gaus+pol3(3)", 0.44, 0.7);
	      fFitFunc->SetLineColor(2);
	      //int fBinPi0Pos = h1D[i][j]->FindBin(pdgPi0Mass);
	      double min_mass = 0.44;
	      double max_mass = 0.70;
	      
	      fPi0Pos = 0.54743;
	      
	      
	      cout << "min mass " << min_mass << " max mass " << max_mass << endl;
	      h1D[i][j]->GetXaxis()->SetRangeUser(min_mass, max_mass);
	      double co = h1D[i][j]->Integral();
	      if (co < 10) continue;
	      fPi0Pos = h1D[i][j]->GetBinCenter(h1D[i][j]->GetMaximumBin());
	      int fBinPi0Pos = h1D[i][j]->FindBin(fPi0Pos);
	      Double_t par_tot_fct[13];
	      Double_t height = h1D[i][j]->GetBinCenter(fBinPi0Pos);
	      
	      //if (i > (ring_region_nb - 1))
	      //height = h1D[i][j]->GetMaximum() / 2;
	      Double_t par_sig_fct[5] = {height, 0.548, 0.03}; 
	      Double_t par_bkg_fct[6] = {1., 1., 1., 1., 1., 1.};
	      
	      fFitFunc->SetRange(min_mass, max_mass);
	      fbkg->SetRange(min_mass, max_mass);
	      fgau->SetRange(min_mass, max_mass);
	      
	      fsig->SetParameters(par_sig_fct);
	      h1D[i][j]->Fit(fsig, "RBQS0");
	      fsig->GetParameters(&par_tot_fct[0]);
	      h1D[i][j]->Fit(fbkg, "RBQS0");
	      fbkg->GetParameters(&par_tot_fct[3]);
	      fFitFunc->SetParameters(par_tot_fct);
	      fFitFunc->SetParLimits(0, h1D[i][j]->GetMaximum() * 0.05, h1D[i][j]->GetMaximum() * 1.2);
	      //if (i < (ring_region_nb - 1)) {
	      //fFitFunc->SetParLimits(1, 0.51, 0.57);
	      //fFitFunc->SetParLimits(2, 0.01, 0.10);
	      fFitFunc->SetParLimits(1, 0.51, 0.8);
	      fFitFunc->SetParLimits(2, 0.01, 0.05);
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
		  gr_endep_plot[i]->SetPoint(k, p, fPi0Pos / pdgEtaMass);
		  gr_endep_plot[i]->SetPointError(k, 0, fPi0Pos_err / pdgEtaMass);
		  k ++;
		} /*else if (i >= (ring_region_nb - 1)) {
		  gr_endep_plot[i]->SetPoint(k, p, fPi0Pos / 0.1);
		  gr_endep_plot[i]->SetPointError(k, 0, fPi0Pos_err / 0.1);
		  k ++;
		  }*/
	      }
	      
	      h1D[i][j]->Draw("p");
	      t->DrawLatex(0.6, ((double) h1D[i][j]->GetMaximum()), Form("#font[42]{p_{#gamma} = %0.2f GeV}", p));
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
  cout <<path + Form("energy_dependence_correction_vs_ring_eta_%d.txt", iter) << endl;
  out.open(path + Form("energy_dependence_correction_vs_ring_eta_%d.txt", iter));
  for (int j = 0; j < 2; j ++) {
  
    cleg = path + Form("Corr_vs_p_eta_%d", j);
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
	  min_a = gr_endep_plot[i]->GetX()[0];
	  max_a = 3.5;//gr_endep_plot[i]->GetX()[gr_endep_plot[i]->GetN() - 1];
	  //fpol = new TF1(Form("fpol_%d", i), "[0] * x + [1] * pow(x,2) + [2] * pow(x,3) + [3] * pow(x,4)", min_a, max_a);
	  //f1=new TF1("f1","[0]-exp(-[1]*x+[2])",0,3);
	  //fpol = new TF1(Form("fpol_plot_%d", i), "pol2", 2.0, max_a);
	  //fpol = new TF1(Form("fpol_plot_%d", i), "[0]+[1]*x*x", min_a, max_a);
	  fpol = new TF1(Form("fpol_plot_%d", i), "[0]-exp(-[1]*x+[2])", min_a, max_a);
	  //fhga = new TF1(Form("fhga_plot_%d", i), "[0]*x + [1]*pow(x,2)", 3.5, 8.0);
	  if (i < 6)
	    fhga = new TF1(Form("fhga_plot_%d", i), "pol2", 3.5, 8.0);
	  else
	    fhga = new TF1(Form("fhga_plot_%d", i), "pol2", 3.5, 6.0);
	  double paramC=1.01991, paramD=1.25434, paramE=-2.28024;
	  fpol->SetParameters(paramC, paramD, paramE);
	  //fpol->SetParameters(1, 1);
	  fpol->SetLineColor(color);
	  //fpol->SetParameters(1, 1, 1);
	  fhga->SetParameters(1, 1, 1);
	  fhga->SetLineColor(color);
	  fhga->SetLineStyle(3);
	  //fpol->SetParLimits(2,-50.0, -0.001);
	  //gr_endep_plot[i]->Fit(fpol, "RB+");
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
		out<<fhga->GetParameter(z - 3);
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
	  //fpol->Draw("same");
	  
	  fhga->SetRange(3.5, 10);
	  fhga->SetLineStyle(3);
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
  out.close();
  //
  out.open(path + Form("energy_dependence_correction_vs_ring_multi_eta_%d.txt", iter));
  for (int j = 0; j < 2; j ++) {
  
    cleg = path + Form("Corr_multo_vs_p_eta_%d", j);
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
	  min_a = gr_endep[i]->GetX()[0];
	  max_a = 3.5;//gr_endep[i]->GetX()[gr_endep[i]->GetN() - 1];
	  //fpol = new TF1(Form("fpol_%d", i), "[0] * x + [1] * pow(x,2) + [2] * pow(x,3) + [3] * pow(x,4)", min_a, max_a);
	  //f1=new TF1("f1","[0]-exp(-[1]*x+[2])",0,3);
	  //fpol = new TF1(Form("fpol_%d", i), "pol5", min_a, max_a);
	  //fpol = new TF1(Form("fpol_%d", i), "pol2", min_a, max_a);
	  fpol = new TF1(Form("fpol_%d", i), "[0]-exp(-[1]*x+[2])", min_a, max_a);
	  double paramC=1.01991, paramD=1.25434, paramE=-2.28024;
	  fpol->SetParameters(paramC, paramD, paramE);
	  fpol->SetLineColor(color);
	  //fpol->SetParameters(1, 1, 1);
	  //fpol->SetParLimits(2,-50.0, 0);
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
  out.close();
  //
  
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .15;
  cleg = path + "Chi2_distribution_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_Chi2, 0.05, 42, 504, 1.1, 1.5);
  h_Chi2->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_distribution_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_width, 0.05, 42, 504, 1.1, 1.5);
  h_width->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Mean_distribution_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_mean, 0.05, 42, 504, 1.1, 1.5);
  h_mean->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_err_distribution_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_width_err, 0.05, 42, 504, 1.1, 1.5);
  h_width_err->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Mean_err_distribution_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_mean_err, 0.05, 42, 504, 1.1, 1.5);
  h_mean_err->Draw();
  
  C1->Print(cleg + ".pdf");

  TFile * outFile = new TFile(path + "energy-correction_eta.root","RECREATE");
  h_Chi2->Write();
  h_mean->Write();
  h_mean_err->Write();
  h_width->Write();
  h_width_err->Write();
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
