// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /DIRC_online/DigiHit/Hit_NHitsVsBox
// hnamepath: /DIRC_online/DigiHit/NorthUpperBox/TDCDigiHit_TimeVsChannel_NorthUpperBox
// hnamepath: /DIRC_online/DigiHit/SouthLowerBox/TDCDigiHit_TimeVsChannel_SouthLowerBox

{  
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("DIRC_online");
  if(dir) dir->cd();

  //TH2I* hOcc = (TH2I*)gDirectory->Get("DigiHit/DigiHit_NHitsVsBox");
  //TH2I* hTN = (TH2I*)gDirectory->Get("DigiHit/NorthUpperBox/TDCDigiHit_TimeVsChannel_NorthUpperBox");
  TH1I* hDigiHit_Nhits = (TH1I*)gDirectory->Get("DigiHit/DigiHit_NHits_LED");
  TH1I* hDigiHit_Time = (TH1I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_Time_LED");
  TH2I* hTS = (TH2I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_TimeVsChannel_NonLED");
  TH2I* hTS_LED = (TH2I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_TimeVsChannel_LED");

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

  if(hDigiHit_Nhits) { 
    hDigiHit_Nhits->SetFillColor(kBlue);
    c1->cd(1);
    hDigiHit_Nhits->SetTitleSize(tsize,"xy");
    hDigiHit_Nhits->Draw();
  }

  if(hDigiHit_Time) {
    hDigiHit_Time->SetFillColor(kBlue);
    c1->cd(3);
    hDigiHit_Time->SetTitleSize(tsize,"xy");
    hDigiHit_Time->Draw();
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
