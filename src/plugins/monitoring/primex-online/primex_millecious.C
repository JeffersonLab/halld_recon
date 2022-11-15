// hnamepath: /primex-online/fcal_tof_dx
// hnamepath: /primex-online/TaggerTiming_vs_egcut
// hnamepath: /primex-online/FCALetato2g_trg3_Esum_sc_w
// hnamepath: /primex-online/FCALetato2g_trg2_Esum_sc_w
// hnamepath: /primex-online/FCALetato3pi0_trg3_Esum_sc_w
// hnamepath: /primex-online/FCALetato3pi0_trg2_Esum_sc_w
// hnamepath: /primex-online/FCALetato2g2pi_trg3_Esum_sc_w
// hnamepath: /primex-online/FCALetato2g2pi_trg2_Esum_sc_w

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
  
  TH2F * h2D = (TH2F *) gDirectory->FindObjectAny("fcal_tof_dx");
  //Nice3D2D(h2D, 0.05, 0.05, 42, 505, 1.1, 1., 1.25, "",
  //	   "#font[42]{|x_{TOF}-x_{FCAL}| [cm]}",
  //	   "#font[42]{|y_{TOF}-y_{FCAL}| [cm]}",
  //	   "#font[42]{Entries #}");
  Double_t BinMin = -30;
  Double_t BinMax = 30;
  Double_t SizeLabel = 0.05;
  Double_t SizeTitle = 0.05;
  Int_t TFONT = 42;
  Int_t NDIV = 505;
  Double_t OffsetTitleX = 1.1;
  Double_t OffsetTitleY = 1.;
  Double_t OffsetTitleZ = 1.25;
  TString NameTitle = "";
  TString NameXTitle = "#font[42]{|x_{TOF}-x_{FCAL}| [cm]}";
  TString NameYTitle = "#font[42]{|y_{TOF}-y_{FCAL}| [cm]}";
  TString NameZTitle = "#font[42]{Entries #}";
  if (h2D != NULL) {
    h2D->SetTitle(NameTitle);
    h2D->SetLabelSize(SizeLabel,"X");
    h2D->SetLabelSize(SizeLabel,"Y");
    h2D->SetLabelSize(SizeLabel,"Z");
    h2D->SetLabelFont(TFONT,"X");
    h2D->SetLabelFont(TFONT,"Y");
    h2D->SetLabelFont(TFONT,"Z");
    h2D->SetTitleSize(SizeTitle);
    h2D->SetTitleSize(SizeTitle,"X");
    h2D->SetTitleSize(SizeTitle,"Y");
    h2D->SetTitleSize(SizeTitle,"Z");
    h2D->SetNdivisions(NDIV,"X");
    h2D->SetNdivisions(NDIV,"Y");
    h2D->SetNdivisions(NDIV,"Z");
    h2D->SetTitleOffset(OffsetTitleX,"X");
    h2D->SetTitleOffset(OffsetTitleY,"Y");
    h2D->SetTitleOffset(OffsetTitleZ,"Z");
    h2D->GetXaxis()->CenterTitle(kTRUE);
    h2D->GetYaxis()->CenterTitle(kTRUE);
    h2D->GetZaxis()->CenterTitle(kTRUE);
    h2D->SetXTitle(NameXTitle);
    h2D->SetYTitle(NameYTitle);
    h2D->SetZTitle(NameZTitle);
  }
  int nbin_im;
  double min_im_bin, max_im_bin, im_step = 0;
  TH1F * h_time = (TH1F *) ((TH2F *) gDirectory->FindObjectAny("TaggerTiming_vs_egcut"))->ProjectionY("time");
  if (h_time != NULL) {
    nbin_im = h_time->GetNbinsX();
    min_im_bin = h_time->GetXaxis()->GetXmin();
    max_im_bin = h_time->GetXaxis()->GetXmax();
    im_step = (max_im_bin - min_im_bin) / ((double) nbin_im);
  }
  //Nice1Db(h_time,-30, 30, 0.05, 0.05, 42,505,1.,1.3,"","#font[42]{t_{Tagger}-t_{RF} [ns]}", Form("#font[41]{Events / %0.3f [ns]}", im_step));
  OffsetTitleX = 1.;
  OffsetTitleY = 1.3;
  NameXTitle = "#font[42]{t_{Tagger}-t_{RF} [ns]}";
  NameYTitle = Form("#font[41]{Events / %0.3f [ns]}", im_step);
  h_time->SetTitle(NameTitle);
  h_time->SetLabelSize(SizeLabel,"X");
  h_time->SetLabelSize(SizeLabel,"Y");
  h_time->SetLabelFont(TFONT,"X");
  h_time->SetLabelFont(TFONT,"Y");
  h_time->SetTitleSize(SizeTitle,"X");
  h_time->SetTitleSize(SizeTitle,"Y");
  h_time->SetNdivisions(NDIV,"X");
  h_time->SetNdivisions(NDIV,"Y");
  h_time->SetTitleOffset(OffsetTitleX,"X");
  h_time->SetTitleOffset(OffsetTitleY,"Y");
  h_time->GetXaxis()->CenterTitle(kTRUE);
  h_time->GetYaxis()->CenterTitle(kTRUE);
  h_time->SetXTitle(NameXTitle);
  h_time->SetYTitle(NameYTitle);
  if (h_time != NULL)
    h_time->GetXaxis()->SetRange(h_time->GetXaxis()->FindBin(BinMin),h_time->GetXaxis()->FindBin(BinMax));
  TString str_trg_gg[] = {"FCALetato2g_trg3_Esum_sc_w", "FCALetato2g_trg2_Esum_sc_w"};
  TString str_trg_pi0pi0pi0[] = {"FCALetato3pi0_trg3_Esum_sc_w", "FCALetato3pi0_trg2_Esum_sc_w"};
  TString str_trg_pi0pippim[] = {"FCALetato2g2pi_trg3_Esum_sc_w", "FCALetato2g2pi_trg2_Esum_sc_w"};
  TH1F * h_trg_gg[2];
  TH1F * h_trg_pi0pi0pi0[2];
  TH1F * h_trg_pi0pippim[2];
  int color[] = {2, 1};
  int marker[] = {20, 21};
  
  for (int i = 0; i < 2; i ++) {
    
    h_trg_gg[i] = (TH1F *) gDirectory->FindObjectAny(str_trg_gg[i]);
    if (h_trg_gg[i] == NULL) continue;
    h_trg_pi0pi0pi0[i] = (TH1F *) gDirectory->FindObjectAny(str_trg_pi0pi0pi0[i]);
    if (h_trg_pi0pi0pi0[i] == NULL) continue;
    h_trg_pi0pippim[i] = (TH1F *) gDirectory->FindObjectAny(str_trg_pi0pippim[i]);
    if (h_trg_pi0pippim[i]  == NULL) continue;
    //h_trg_gg[i]->Rebin(100);
    //h_trg_pi0pi0pi0[i]->Rebin(100);
    //h_trg_pi0pippim[i]->Rebin(100);
    nbin_im = h_trg_gg[i]->GetNbinsX();
    min_im_bin = h_trg_gg[i]->GetXaxis()->GetXmin();
    max_im_bin = h_trg_gg[i]->GetXaxis()->GetXmax();
    im_step = (max_im_bin - min_im_bin) / ((double) nbin_im);
    NameXTitle = "#font[42]{E^{FCAL}_{sum} [GeV]}";
    NameYTitle = Form("#font[41]{Events / %0.3f [GeV]}", im_step);
    h_trg_gg[i]->SetTitle(NameTitle);
    h_trg_gg[i]->SetLabelSize(SizeLabel,"X");
    h_trg_gg[i]->SetLabelSize(SizeLabel,"Y");
    h_trg_gg[i]->SetLabelFont(TFONT,"X");
    h_trg_gg[i]->SetLabelFont(TFONT,"Y");
    h_trg_gg[i]->SetTitleSize(SizeTitle,"X");
    h_trg_gg[i]->SetTitleSize(SizeTitle,"Y");
    h_trg_gg[i]->SetNdivisions(NDIV,"X");
    h_trg_gg[i]->SetNdivisions(NDIV,"Y");
    h_trg_gg[i]->SetTitleOffset(OffsetTitleX,"X");
    h_trg_gg[i]->SetTitleOffset(OffsetTitleY,"Y");
    h_trg_gg[i]->GetXaxis()->CenterTitle(kTRUE);
    h_trg_gg[i]->GetYaxis()->CenterTitle(kTRUE);
    h_trg_gg[i]->SetXTitle(NameXTitle);
    h_trg_gg[i]->SetYTitle(NameYTitle);
    h_trg_pi0pi0pi0[i]->SetTitle(NameTitle);
    h_trg_pi0pi0pi0[i]->SetLabelSize(SizeLabel,"X");
    h_trg_pi0pi0pi0[i]->SetLabelSize(SizeLabel,"Y");
    h_trg_pi0pi0pi0[i]->SetLabelFont(TFONT,"X");
    h_trg_pi0pi0pi0[i]->SetLabelFont(TFONT,"Y");
    h_trg_pi0pi0pi0[i]->SetTitleSize(SizeTitle,"X");
    h_trg_pi0pi0pi0[i]->SetTitleSize(SizeTitle,"Y");
    h_trg_pi0pi0pi0[i]->SetNdivisions(NDIV,"X");
    h_trg_pi0pi0pi0[i]->SetNdivisions(NDIV,"Y");
    h_trg_pi0pi0pi0[i]->SetTitleOffset(OffsetTitleX,"X");
    h_trg_pi0pi0pi0[i]->SetTitleOffset(OffsetTitleY,"Y");
    h_trg_pi0pi0pi0[i]->GetXaxis()->CenterTitle(kTRUE);
    h_trg_pi0pi0pi0[i]->GetYaxis()->CenterTitle(kTRUE);
    h_trg_pi0pi0pi0[i]->SetXTitle(NameXTitle);
    h_trg_pi0pi0pi0[i]->SetYTitle(NameYTitle);
    h_trg_pi0pippim[i]->SetTitle(NameTitle);
    h_trg_pi0pippim[i]->SetLabelSize(SizeLabel,"X");
    h_trg_pi0pippim[i]->SetLabelSize(SizeLabel,"Y");
    h_trg_pi0pippim[i]->SetLabelFont(TFONT,"X");
    h_trg_pi0pippim[i]->SetLabelFont(TFONT,"Y");
    h_trg_pi0pippim[i]->SetTitleSize(SizeTitle,"X");
    h_trg_pi0pippim[i]->SetTitleSize(SizeTitle,"Y");
    h_trg_pi0pippim[i]->SetNdivisions(NDIV,"X");
    h_trg_pi0pippim[i]->SetNdivisions(NDIV,"Y");
    h_trg_pi0pippim[i]->SetTitleOffset(OffsetTitleX,"X");
    h_trg_pi0pippim[i]->SetTitleOffset(OffsetTitleY,"Y");
    h_trg_pi0pippim[i]->GetXaxis()->CenterTitle(kTRUE);
    h_trg_pi0pippim[i]->GetYaxis()->CenterTitle(kTRUE);
    h_trg_pi0pippim[i]->SetXTitle(NameXTitle);
    h_trg_pi0pippim[i]->SetYTitle(NameYTitle);
        
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
  smallBetween1 = .11;
  smallBetween2 = .175;
  smallBetween3 = .05;
  smallBetween4 = .125;
  C1->cd(1);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h2D != NULL) {
    h2D->SetMinimum(1);
    h2D->Draw("colz");
    gPad->SetLogz();
  }
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .125;
  C1->cd(2);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_time != NULL)
    h_time->Draw();

  C1->cd(4);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_trg_gg[0] != NULL) {
    h_trg_gg[0]->Draw();
    if (h_trg_gg[1] != NULL)
      h_trg_gg[1]->Draw("same");
  
    legend=new TLegend(0.25,0.75,0.45,0.9);
    if (h_trg_gg[1] != NULL) legend->AddEntry(h_trg_gg[1],"#font[42]{Bit 1, E^{FCAL}_{sum} #geq 3.5GeV}","p");
    legend->AddEntry(h_trg_gg[0],"#font[42]{Bit 2, E^{FCAL}_{sum} #geq 0.5GeV}","p");
    legend->SetFillColor(0);
    legend->SetTextFont(22);
    legend->SetTextSize(.05);
    legend->SetLineColor(0);
    legend->Draw("same");
  
    t->DrawLatex(0.5, h_trg_gg[0]->GetMaximum() * 0.7, "#font[42]{#eta#rightarrow#gamma#gamma}");
  }
  C1->cd(5);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_trg_pi0pi0pi0[0] != NULL) {
    h_trg_pi0pi0pi0[0]->Draw();
    if (h_trg_pi0pi0pi0[1] != NULL)
      h_trg_pi0pi0pi0[1]->Draw("same");
  
    t->DrawLatex(0.5, h_trg_pi0pi0pi0[0]->GetMaximum() * 0.7, "#font[42]{#eta#rightarrow#pi^{0}#pi^{0}#pi^{0}}");
  }
  C1->cd(6);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_trg_pi0pippim[0] != NULL) {
    h_trg_pi0pippim[0]->Draw();
    if (h_trg_pi0pippim[1])
      h_trg_pi0pippim[1]->Draw("same");
  
    t->DrawLatex(0.5, h_trg_pi0pippim[0]->GetMaximum() * 0.7, "#font[42]{#eta#rightarrow#pi^{0}#pi^{+}#pi^{-}}");
  }
}
