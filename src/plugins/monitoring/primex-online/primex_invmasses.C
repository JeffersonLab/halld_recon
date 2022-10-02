// hnamepath: /primex-online/m2g_sc_trg3
// hnamepath: /primex-online/m2g_sc_trg2
// hnamepath: /primex-online/m2g_sc_w_trg3
// hnamepath: /primex-online/m2g_sc_w_trg2
// hnamepath: /primex-online/m6g_sc_trg3
// hnamepath: /primex-online/m6g_sc_trg2
// hnamepath: /primex-online/m6g_sc_w_trg3
// hnamepath: /primex-online/m6g_sc_w_trg2
// hnamepath: /primex-online/m2g2pi_sc_trg3
// hnamepath: /primex-online/m2g2pi_sc_trg2
// hnamepath: /primex-online/m2g2pi_sc_w_trg3
// hnamepath: /primex-online/m2g2pi_sc_w_trg2

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
    
  TH1F * h_m_gg[4];
  TH1F * h_m_pi0pi0pi0[4];
  TH1F * h_m_pi0pippim[4];
  
  TString str_gg[] = {"m2g_sc_trg3", "m2g_sc_trg2", "m2g_sc_w_trg3", "m2g_sc_w_trg2",};
  TString str_pi0pi0pi0[] = {"m6g_sc_trg3", "m6g_sc_trg2", "m6g_sc_w_trg3", "m6g_sc_w_trg2",};
  TString str_pi0pippim[] = {"m2g2pi_sc_trg3", "m2g2pi_sc_trg2", "m2g2pi_sc_w_trg3", "m2g2pi_sc_w_trg2",};
  
  int nbin_im;
  double min_im_bin, max_im_bin, im_step;
  int color[] = {2, 1, 2, 1};
  int marker[] = {20, 21, 20, 21};
  Double_t BinMin = 0.4;
  Double_t BinMax = 1.199;
  Double_t SizeLabel = 0.05;
  Double_t SizeTitle = 0.05;
  Int_t TFONT = 42;
  Int_t NDIV = 505;
  Double_t OffsetTitleX = 1.;
  Double_t OffsetTitleY = 1.6;
  TString NameTitle = "";
  TString NameXTitle = "";
  TString NameYTitle = "";
  for (int i = 0; i < 4; i ++) {
    h_m_gg[i] = (TH1F *) gDirectory->FindObjectAny(str_gg[i]);
    if (h_m_gg[i] == NULL) continue;
    h_m_pi0pi0pi0[i] = (TH1F *) gDirectory->FindObjectAny(str_pi0pi0pi0[i]);
    if (h_m_pi0pi0pi0[i] == NULL) continue;
    h_m_pi0pippim[i] = (TH1F *) gDirectory->FindObjectAny(str_pi0pippim[i]);
    if (h_m_pi0pippim[i] == NULL) continue;
    //h_m_gg[i]->Rebin(10);
    //h_m_pi0pi0pi0[i]->Rebin(10);
    //h_m_pi0pippim[i]->Rebin(10);

    nbin_im = h_m_gg[i]->GetNbinsX();
    min_im_bin = h_m_gg[i]->GetXaxis()->GetXmin();
    max_im_bin = h_m_gg[i]->GetXaxis()->GetXmax();
    im_step = (max_im_bin - min_im_bin) / ((double) nbin_im);
    NameXTitle = "#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]}";
    NameYTitle = Form("#font[41]{Events / %0.3f [GeV/#it{c}^{2}]}", im_step);
    //Nice1Db(h_m_gg[i], 0.4, 1.199, 0.05, 0.05, 42,505,1.,1.6,"","#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]}", Form("#font[41]{Events / %0.3f [GeV/#it{c}^{2}]}", im_step));
    h_m_gg[i]->SetTitle(NameTitle);
    h_m_gg[i]->SetLabelSize(SizeLabel,"X");
    h_m_gg[i]->SetLabelSize(SizeLabel,"Y");
    h_m_gg[i]->SetLabelFont(TFONT,"X");
    h_m_gg[i]->SetLabelFont(TFONT,"Y");
    h_m_gg[i]->SetTitleSize(SizeTitle,"X");
    h_m_gg[i]->SetTitleSize(SizeTitle,"Y");
    h_m_gg[i]->SetNdivisions(NDIV,"X");
    h_m_gg[i]->SetNdivisions(NDIV,"Y");
    h_m_gg[i]->SetTitleOffset(OffsetTitleX,"X");
    h_m_gg[i]->SetTitleOffset(OffsetTitleY,"Y");
    h_m_gg[i]->GetXaxis()->CenterTitle(kTRUE);
    h_m_gg[i]->GetYaxis()->CenterTitle(kTRUE);
    h_m_gg[i]->SetXTitle(NameXTitle);
    h_m_gg[i]->SetYTitle(NameYTitle);
    h_m_gg[i]->GetXaxis()->SetRange(h_m_gg[i]->GetXaxis()->FindBin(BinMin),h_m_gg[i]->GetXaxis()->FindBin(BinMax));
    nbin_im = h_m_pi0pi0pi0[i]->GetNbinsX();
    min_im_bin = h_m_pi0pi0pi0[i]->GetXaxis()->GetXmin();
    max_im_bin = h_m_pi0pi0pi0[i]->GetXaxis()->GetXmax();
    im_step = (max_im_bin - min_im_bin) / ((double) nbin_im);
    NameXTitle = "#font[42]{#it{m}_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}]}";
    h_m_pi0pi0pi0[i]->SetTitle(NameTitle);
    h_m_pi0pi0pi0[i]->SetLabelSize(SizeLabel,"X");
    h_m_pi0pi0pi0[i]->SetLabelSize(SizeLabel,"Y");
    h_m_pi0pi0pi0[i]->SetLabelFont(TFONT,"X");
    h_m_pi0pi0pi0[i]->SetLabelFont(TFONT,"Y");
    h_m_pi0pi0pi0[i]->SetTitleSize(SizeTitle,"X");
    h_m_pi0pi0pi0[i]->SetTitleSize(SizeTitle,"Y");
    h_m_pi0pi0pi0[i]->SetNdivisions(NDIV,"X");
    h_m_pi0pi0pi0[i]->SetNdivisions(NDIV,"Y");
    h_m_pi0pi0pi0[i]->SetTitleOffset(OffsetTitleX,"X");
    h_m_pi0pi0pi0[i]->SetTitleOffset(OffsetTitleY,"Y");
    h_m_pi0pi0pi0[i]->GetXaxis()->CenterTitle(kTRUE);
    h_m_pi0pi0pi0[i]->GetYaxis()->CenterTitle(kTRUE);
    h_m_pi0pi0pi0[i]->SetXTitle(NameXTitle);
    h_m_pi0pi0pi0[i]->SetYTitle(NameYTitle);
    h_m_pi0pi0pi0[i]->GetXaxis()->SetRange(h_m_pi0pi0pi0[i]->GetXaxis()->FindBin(BinMin),h_m_pi0pi0pi0[i]->GetXaxis()->FindBin(BinMax));
    //Nice1Db(h_m_pi0pi0pi0[i], 0.4, 1.199, 0.05, 0.05, 42,505,1.,1.6,"","#font[42]{#it{m}_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}]}", Form("#font[41]{Events / %0.3f [GeV/#it{c}^{2}]}", im_step));
    nbin_im = h_m_pi0pippim[i]->GetNbinsX();
    min_im_bin = h_m_pi0pippim[i]->GetXaxis()->GetXmin();
    max_im_bin = h_m_pi0pippim[i]->GetXaxis()->GetXmax();
    im_step = (max_im_bin - min_im_bin) / ((double) nbin_im);
    NameXTitle = "#font[42]{#it{m}_{#pi^{0}#pi^{+}#pi^{-}} [GeV/#it{c}^{2}]}";
    h_m_pi0pippim[i]->SetTitle(NameTitle);
    h_m_pi0pippim[i]->SetLabelSize(SizeLabel,"X");
    h_m_pi0pippim[i]->SetLabelSize(SizeLabel,"Y");
    h_m_pi0pippim[i]->SetLabelFont(TFONT,"X");
    h_m_pi0pippim[i]->SetLabelFont(TFONT,"Y");
    h_m_pi0pippim[i]->SetTitleSize(SizeTitle,"X");
    h_m_pi0pippim[i]->SetTitleSize(SizeTitle,"Y");
    h_m_pi0pippim[i]->SetNdivisions(NDIV,"X");
    h_m_pi0pippim[i]->SetNdivisions(NDIV,"Y");
    h_m_pi0pippim[i]->SetTitleOffset(OffsetTitleX,"X");
    h_m_pi0pippim[i]->SetTitleOffset(OffsetTitleY,"Y");
    h_m_pi0pippim[i]->GetXaxis()->CenterTitle(kTRUE);
    h_m_pi0pippim[i]->GetYaxis()->CenterTitle(kTRUE);
    h_m_pi0pippim[i]->SetXTitle(NameXTitle);
    h_m_pi0pippim[i]->SetYTitle(NameYTitle);
    h_m_pi0pippim[i]->GetXaxis()->SetRange(h_m_pi0pippim[i]->GetXaxis()->FindBin(BinMin),h_m_pi0pippim[i]->GetXaxis()->FindBin(BinMax));
    //Nice1Db(h_m_pi0pippim[i], 0.4, 1.199, 0.05, 0.05, 42,505,1.,1.6,"","#font[42]{#it{m}_{#pi^{0}#pi^{+}#pi^{-}} [GeV/#it{c}^{2}]}", Form("#font[41]{Events / %0.3f [GeV/#it{c}^{2}]}", im_step));
    h_m_gg[i]->SetLineColor(color[i]);
    h_m_gg[i]->SetMarkerColor(color[i]);
    h_m_gg[i]->SetMarkerSize(1);
    h_m_gg[i]->SetLineWidth(1);
    h_m_gg[i]->SetMarkerStyle(marker[i]);
    h_m_pi0pi0pi0[i]->SetLineColor(color[i]);
    h_m_pi0pi0pi0[i]->SetMarkerColor(color[i]);
    h_m_pi0pi0pi0[i]->SetMarkerSize(1);
    h_m_pi0pi0pi0[i]->SetLineWidth(1);
    h_m_pi0pi0pi0[i]->SetMarkerStyle(marker[i]);
    h_m_pi0pippim[i]->SetLineColor(color[i]);
    h_m_pi0pippim[i]->SetMarkerColor(color[i]);
    h_m_pi0pippim[i]->SetMarkerSize(1);
    h_m_pi0pippim[i]->SetLineWidth(1);
    h_m_pi0pippim[i]->SetMarkerStyle(marker[i]);
  }

  
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .075;
  smallBetween4 = .15;
  
  cleg = "PrimEx-eta-invariant-masses";
  if(gPad == NULL){
    C1 = new TCanvas(cleg, cleg, 1800, 1200);
    C1->cd(0);
    C1->Draw();
    C1->Update();
  }
  
  if( !gPad ) return;
  C1 = gPad->GetCanvas();
  C1->Divide(3, 2);
   
  //C1 = new TCanvas(cleg, cleg, 10, 10, 1800, 1200);
  //C1->Divide(3,2);
  C1->cd(1);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_m_gg[0] != NULL) {
    h_m_gg[0]->Draw();
    if (h_m_gg[1] != NULL) 
      h_m_gg[1]->Draw("same");
  
    legend=new TLegend(0.45,0.75,0.85,0.9);
    if (h_m_gg[1] != NULL) legend->AddEntry(h_m_gg[1],"#font[42]{Bit 1, E^{FCAL}_{sum} #geq 3.5GeV}","p");
    legend->AddEntry(h_m_gg[0],"#font[42]{Bit 2, E^{FCAL}_{sum} #geq 0.5GeV}","p");
    legend->SetFillColor(0);
    legend->SetTextFont(22);
    legend->SetTextSize(.05);
    legend->SetLineColor(0);
    legend->Draw("same");
  
    t->DrawLatex(0.5, h_m_gg[0]->GetMaximum() * 1.09, "#font[42]{Untagged}");
  }
  C1->cd(2);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_m_pi0pi0pi0[0] != NULL) {
    h_m_pi0pi0pi0[0]->Draw();
    if (h_m_pi0pi0pi0[1] != NULL)
      h_m_pi0pi0pi0[1]->Draw("same");
  
    t->DrawLatex(0.5, h_m_pi0pi0pi0[0]->GetMaximum() * 1.09, "#font[42]{Untagged}");
  }
  C1->cd(3);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_m_pi0pippim[0] != NULL) {
    h_m_pi0pippim[0]->Draw();
    if (h_m_pi0pippim[1] != NULL)
      h_m_pi0pippim[1]->Draw("same");
  
    t->DrawLatex(0.5, h_m_pi0pippim[0]->GetMaximum() * 1.09, "#font[42]{Untagged}");
  }
  C1->cd(4);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_m_gg[2] != NULL) {
    h_m_gg[2]->Draw();
    if (h_m_gg[3] != NULL)
      h_m_gg[3]->Draw("same");
  
    t->DrawLatex(0.65, h_m_gg[2]->GetMaximum() * 0.8, "#font[42]{Tagged & E_{#gamma}^{beam} #geq 8 GeV}");
  }
  C1->cd(5);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_m_pi0pi0pi0[2] != NULL) {
    h_m_pi0pi0pi0[2]->Draw();
    if (h_m_pi0pi0pi0[3] != NULL)
      h_m_pi0pi0pi0[3]->Draw("same");
  
    t->DrawLatex(0.65, h_m_pi0pi0pi0[2]->GetMaximum() * 0.8, "#font[42]{Tagged & E_{#gamma}^{beam} #geq 8 GeV}");
  }
  C1->cd(6);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  if (h_m_pi0pippim[2] != NULL) {
    h_m_pi0pippim[2]->Draw();
    if (h_m_pi0pippim[3] != NULL)
      h_m_pi0pippim[3]->Draw("same");
  
    t->DrawLatex(0.5, h_m_pi0pippim[2]->GetMaximum() * 1.09, "#font[42]{Tagged & E_{#gamma}^{beam} #geq 8 GeV}");
  }
}
