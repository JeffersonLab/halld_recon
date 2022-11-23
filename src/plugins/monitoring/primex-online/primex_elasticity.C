// hnamepath: /primex-online/Primakoff_2g
// hnamepath: /primex-online/Primakoff_6g
// hnamepath: /primex-online/Primakoff_2g2pi
// hnamepath: /primex-online/theta_2g
// hnamepath: /primex-online/theta_6g
// hnamepath: /primex-online/theta_2g2pi

{
  
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
  TString dir_path = "primex-online/";
  TString file = "";

  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("primex-online");
  if(dir) dir->cd();
  
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
  Double_t BinMin = 0.4;
  Double_t BinMax = 1.199;
  Double_t SizeLabel = 0.05;
  Double_t SizeTitle = 0.05;
  Int_t TFONT = 42;
  Int_t NDIV = 505;
  Double_t OffsetTitleX = 1.;
  Double_t OffsetTitleY = 1.;
  Double_t OffsetTitleZ = 1.;
  TString NameTitle = "";
  TString NameXTitle = "";
  TString NameYTitle = "";
  TString NameZTitle = "";
  
  for (int i = 0; i < 3; i ++) {
    h2D[i] = (TH2F *) gDirectory->FindObjectAny(str_elasticity[i]);
    if (h2D[i] == NULL) continue;
    NameYTitle = Form("#font[42]{%s/m_{#eta}}", str_E[i].Data());
    NameXTitle = Form("#font[42]{%s/E_{#gamma}^{beam}}", str_m[i].Data());
    NameZTitle = "#font[42]{Entries #}";
    OffsetTitleX = 1.1;
    OffsetTitleY = 0.9;
    OffsetTitleZ = 1.25;
    h2D[i]->SetTitle(NameTitle);
    h2D[i]->SetLabelSize(SizeLabel,"X");
    h2D[i]->SetLabelSize(SizeLabel,"Y");
    h2D[i]->SetLabelSize(SizeLabel,"Z");
    h2D[i]->SetLabelFont(TFONT,"X");
    h2D[i]->SetLabelFont(TFONT,"Y");
    h2D[i]->SetLabelFont(TFONT,"Z");
    h2D[i]->SetTitleSize(SizeTitle);
    h2D[i]->SetTitleSize(SizeTitle,"X");
    h2D[i]->SetTitleSize(SizeTitle,"Y");
    h2D[i]->SetTitleSize(SizeTitle,"Z");
    h2D[i]->SetNdivisions(NDIV,"X");
    h2D[i]->SetNdivisions(NDIV,"Y");
    h2D[i]->SetNdivisions(NDIV,"Z");
    h2D[i]->SetTitleOffset(OffsetTitleX,"X");
    h2D[i]->SetTitleOffset(OffsetTitleY,"Y");
    h2D[i]->SetTitleOffset(OffsetTitleZ,"Z");
    h2D[i]->GetXaxis()->CenterTitle(kTRUE);
    h2D[i]->GetYaxis()->CenterTitle(kTRUE);
    h2D[i]->GetZaxis()->CenterTitle(kTRUE);
    h2D[i]->SetXTitle(NameXTitle);
    h2D[i]->SetYTitle(NameYTitle);
    h2D[i]->SetZTitle(NameZTitle);
    h_theta[i] = (TH1F *) gDirectory->FindObjectAny(str_theta[i]);
    if (h_theta[i] == NULL) continue;
    //h_theta[i]->Rebin(10);
    nbin_im = h_theta[i]->GetNbinsX();
    min_im_bin = h_theta[i]->GetXaxis()->GetXmin();
    max_im_bin = h_theta[i]->GetXaxis()->GetXmax();
    im_step = (max_im_bin - min_im_bin) / ((double) nbin_im);
    OffsetTitleX = 1.0;
    OffsetTitleY = 1.6;
    NameXTitle = Form("#font[42]{#it{#theta}_{#eta#rightarrow%s} [^{o}]}", str_fs[i].Data());
    NameYTitle = Form("#font[41]{Events / %0.3f [^{o}]}", im_step);
    h_theta[i]->SetTitle(NameTitle);
    h_theta[i]->SetLabelSize(SizeLabel,"X");
    h_theta[i]->SetLabelSize(SizeLabel,"Y");
    h_theta[i]->SetLabelFont(TFONT,"X");
    h_theta[i]->SetLabelFont(TFONT,"Y");
    h_theta[i]->SetTitleSize(SizeTitle,"X");
    h_theta[i]->SetTitleSize(SizeTitle,"Y");
    h_theta[i]->SetNdivisions(NDIV,"X");
    h_theta[i]->SetNdivisions(NDIV,"Y");
    h_theta[i]->SetTitleOffset(OffsetTitleX,"X");
    h_theta[i]->SetTitleOffset(OffsetTitleY,"Y");
    h_theta[i]->GetXaxis()->CenterTitle(kTRUE);
    h_theta[i]->GetYaxis()->CenterTitle(kTRUE);
    h_theta[i]->SetXTitle(NameXTitle);
    h_theta[i]->SetYTitle(NameYTitle);
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
    if(gPad == NULL){
    C1 = new TCanvas(cleg, cleg, 1800, 1200);
    C1->cd(0);
    C1->Draw();
    C1->Update();
  }
  
  if( !gPad ) return;
  C1 = gPad->GetCanvas();
  //C1 = new TCanvas(cleg, cleg, 10, 10, 1800, 1200);
  C1->Divide(3,2);
  for (int i = 0; i < 3; i ++) {
    if (h2D[i] == NULL) continue;
    if (h_theta[i] == NULL) continue;
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
