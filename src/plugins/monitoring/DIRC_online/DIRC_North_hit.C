// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_TimeOverThresholdVsChannel_LED
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_TimeVsChannel_LED
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_Time_NonLED
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_Time_LED
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_PixelOccupancy_LED
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_PixelOccupancy_NonLED
//
// e-mail: davidl@jlab.org
// e-mail: jrsteven@jlab.org
// e-mail: billlee@jlab.org
// e-mail: tbritton@jlab.org


{  
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("DIRC_online");
  if(dir) dir->cd();

  TH2I* hTimeVsChannel_LED = (TH2I*)gDirectory->Get("Hit/NorthUpperBox/Hit_TimeVsChannel_LED");
  TH1I* hTS = (TH1I*)gDirectory->Get("Hit/NorthUpperBox/Hit_Time_NonLED");
  TH1I* hTS_LED = (TH1I*)gDirectory->Get("Hit/NorthUpperBox/Hit_Time_LED");  

  TH2I* hOcc_LED = (TH2I*)gDirectory->Get("Hit/NorthUpperBox/Hit_PixelOccupancy_LED");
  TH2I* hOcc = (TH2I*)gDirectory->Get("Hit/NorthUpperBox/Hit_PixelOccupancy_NonLED");

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

  if(hOcc_LED){
    c1->cd(1);
    hOcc_LED->SetTitleSize(tsize,"xy");
    hOcc_LED->Draw("colz");
  }

  if(hTimeVsChannel_LED){
    c1->cd(2);
    hTimeVsChannel_LED->SetTitleSize(tsize,"xy");
    hTimeVsChannel_LED->Draw("colz");
  }

  if(hOcc){
    c1->cd(3);
    hOcc->SetTitleSize(tsize,"xy");
    hOcc->Draw("colz");
  }


  if(hTS && hTS_LED){
    hTS->SetLineColor(kBlack);
    hTS_LED->SetLineColor(kBlue);
    c1->cd(4);
    hTS_LED->SetTitleSize(tsize,"xy");
    hTS_LED->Draw();
    hTS->Scale(hTS_LED->GetMaximum()/hTS->GetMaximum());
    hTS->Draw("h same");

    TLegend *leg = new TLegend(0.6, 0.6, 0.85, 0.8);
    leg->AddEntry(hTS_LED,"LED trigger","l");
    leg->AddEntry(hTS,"Non-LED triggers","l");
    leg->Draw("same");
  }


}
