// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /DIRC_online/DigiHit/DigiHit_NHits_LED
// hnamepath: /DIRC_online/DigiHit/DigiHit_NHits_NonLED
// hnamepath: /DIRC_online/DigiHit/SouthLowerBox/TDCDigiHit_Time_LED
// hnamepath: /DIRC_online/DigiHit/SouthLowerBox/TDCDigiHit_Time_NonLED
// hnamepath: /DIRC_online/DigiHit/SouthLowerBox/TDCDigiHit_TimeVsChannel_LED
// hnamepath: /DIRC_online/DigiHit/SouthLowerBox/TDCDigiHit_TimeVsChannel_NonLED

{  
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("DIRC_online");
  if(dir) dir->cd();

  //TH2I* hOcc = (TH2I*)gDirectory->Get("DigiHit/DigiHit_NHitsVsBox");
  //TH2I* hTN = (TH2I*)gDirectory->Get("DigiHit/NorthUpperBox/TDCDigiHit_TimeVsChannel_NorthUpperBox");
  TH1I* hDigiHit_Nhits_LED = (TH1I*)gDirectory->Get("DigiHit/DigiHit_NHits_LED");
  TH1I* hDigiHit_Nhits = (TH1I*)gDirectory->Get("DigiHit/DigiHit_NHits_NonLED");
  TH1I* hDigiHit_Time_LED = (TH1I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_Time_LED");
  TH1I* hDigiHit_Time = (TH1I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_Time_NonLED");
  TH2I* hTS_LED = (TH2I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_TimeVsChannel_LED");
  TH2I* hTS = (TH2I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_TimeVsChannel_NonLED");

  if(gPad == NULL){
    TCanvas *c1 = new TCanvas("c1","DIRC Hit Monitor",150,10,990,660);
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }

  if(!gPad) return;
  TCanvas* c1 = gPad->GetCanvas();
  c1->Divide(2,2);

  double tsize = 0.05;  
  gStyle->SetOptStat("emr");

/*
  if(hOcc){
    hOcc->SetFillColor(kBlue);
    c1->cd(1);
    hOcc->SetLabelSize(0.08,"x");
    hOcc->SetTitleSize(tsize,"xy");
    hOcc->GetXaxis()->SetBinLabel(1, "North");
    hOcc->GetXaxis()->SetBinLabel(2, "South");
    hOcc->Draw("colz");
  }
 
  if(hTN){
    hTN->SetFillColor(kBlue);
    c1->cd(3);
    hTN->SetTitleSize(tsize,"xy");
    hTN->Draw("colz");
  }
*/

    TLegend *leg = new TLegend(0.6, 0.6, 0.85, 0.8);
    leg->AddEntry(hTS_LED,"LED trigger","l");
    leg->AddEntry(hTS,"Non-LED triggers","l");
    leg->Draw("same");


  if(hDigiHit_Nhits && hDigiHit_Nhits_LED) { 
    hDigiHit_Nhits->SetLineColor(kBlack);
    hDigiHit_Nhits_LED->SetLineColor(kBlue);
    c1->cd(1);
    hDigiHit_Nhits_LED->SetTitleSize(tsize,"xy");
    hDigiHit_Nhits_LED->Draw();
    double scale = hDigiHit_Nhits_LED->GetMaximum()/hDigiHit_Nhits->GetMaximum();
    if(hDigiHit_Nhits->GetMaximum() == 0) scale = 1.;
    hDigiHit_Nhits->Scale(scale);
    hDigiHit_Nhits->Draw("h same");

    TLegend *leg = new TLegend(0.6, 0.6, 0.85, 0.8);
    leg->AddEntry(hDigiHit_Nhits_LED,"LED trigger","l");
    leg->AddEntry(hDigiHit_Nhits,"Non-LED triggers","l");
    leg->Draw("same");
  }

  if(hDigiHit_Time && hDigiHit_Time_LED) {
    hDigiHit_Time->SetLineColor(kBlack);
    hDigiHit_Time_LED->SetLineColor(kBlue);
    c1->cd(3);
    hDigiHit_Time->SetTitleSize(tsize,"xy");
    hDigiHit_Time_LED->Draw();
    double scale = hDigiHit_Time_LED->GetMaximum()/hDigiHit_Time->GetMaximum();
    if(hDigiHit_Time->GetMaximum() == 0) scale = 1.;
    hDigiHit_Time->Scale(scale);
    hDigiHit_Time->Draw("h same");

    TLegend *leg = new TLegend(0.6, 0.6, 0.85, 0.8);
    leg->AddEntry(hDigiHit_Time_LED,"LED trigger","l");
    leg->AddEntry(hDigiHit_Time,"Non-LED triggers","l");
    leg->Draw("same");

  }

  if(hTS){
    c1->cd(4);
    hTS->SetTitleSize(tsize,"xy");
    hTS->Draw("colz");
  }

  if(hTS_LED){
    c1->cd(2);
    hTS_LED->SetTitleSize(tsize,"xy");
    hTS_LED->Draw("colz");
  }



}
