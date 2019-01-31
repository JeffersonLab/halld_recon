// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /DIRC_online/DigiHit/Hit_NHitsVsBox
// hnamepath: /DIRC_online/DigiHit/NorthUpperBox/TDCDigiHit_TimeVsChannel_NorthUpperBox
// hnamepath: /DIRC_online/DigiHit/SouthLowerBox/TDCDigiHit_TimeVsChannel_SouthLowerBox

{  
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("DIRC_online");
  if(dir) dir->cd();

  TH2I* hOcc = (TH2I*)gDirectory->Get("DigiHit/DigiHit_NHitsVsBox");
  TH2I* hTN = (TH2I*)gDirectory->Get("DigiHit/NorthUpperBox/TDCDigiHit_TimeVsChannel_NorthUpperBox");
  TH2I* hTS = (TH2I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_TimeVsChannel_SouthLowerBox");

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

  if(hTS){
    hTS->SetFillColor(kBlue);
    c1->cd(4);
    hTS->SetTitleSize(tsize,"xy");
    hTS->Draw("colz");
  }

}
