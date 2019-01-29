// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /DIRC_online/Hit/Hit_NHitsVsBox
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_TimeOverThresholdVsChannel_NorthUpperBox
// hnamepath: /DIRC_online/Hit/SouthLowerBox/Hit_TimeOverThresholdVsChannel_SouthLowerBox
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_TimeVsChannel_NorthUpperBox
// hnamepath: /DIRC_online/Hit/SouthLowerBox/Hit_TimeVsChannel_SouthLowerBox
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_PixelOccupancy_NorthUpperBox
// hnamepath: /DIRC_online/Hit/SouthLowerBox/Hit_PixelOccupancy_SouthLowerBox

{  
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("DIRC_online");
  if(dir) dir->cd();

  TH2I* hOcc = (TH2I*)gDirectory->Get("Hit/Hit_NHitsVsBox");
  TH2I* hTotN = (TH2I*)gDirectory->Get("Hit/NorthUpperBox/Hit_TimeOverThresholdVsChannel_NorthUpperBox");
  TH2I* hTotS = (TH2I*)gDirectory->Get("Hit/SouthLowerBox/Hit_TimeOverThresholdVsChannel_SouthLowerBox");
  TH2I* hTN = (TH2I*)gDirectory->Get("Hit/NorthUpperBox/Hit_TimeVsChannel_NorthUpperBox");
  TH2I* hTS = (TH2I*)gDirectory->Get("Hit/SouthLowerBox/Hit_TimeVsChannel_SouthLowerBox");
  TH2I* hOccN = (TH2I*)gDirectory->Get("Hit/NorthUpperBox/Hit_PixelOccupancy_NorthUpperBox");
  TH2I* hOccS = (TH2I*)gDirectory->Get("Hit/SouthLowerBox/Hit_PixelOccupancy_SouthLowerBox");

  if(gPad == NULL){
    TCanvas *c1 = new TCanvas("c1","DIRC Hit Monitor",150,10,990,660);
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }

  if(!gPad) return;
  TCanvas* c1 = gPad->GetCanvas();
  c1->Divide(3,2);

  double tsize = 0.05;  
  gStyle->SetOptStat("emr");
  if(hOcc){
    c1->cd(1);
    hOcc->SetLabelSize(0.08,"x");
    hOcc->SetTitleSize(tsize,"xy");
    hOcc->GetXaxis()->SetBinLabel(1, "North");
    hOcc->GetXaxis()->SetBinLabel(2, "South");
    hOcc->Draw("colz");
  }
 
  if(hTotN){
    c1->cd(2);
    hTotN->SetTitleSize(tsize,"xy");
    hTotN->Draw("colz");
  }

  if(hTotS){
    c1->cd(3);
    hTotS->SetTitleSize(tsize,"xy");
    hTotS->Draw("colz");
  }

  if(hOccN && hOccS){
    c1->cd(4);
    TPad *p1 = new TPad("p1","p1",0.0,0.5,1.0,1.0);
    p1->Draw();
    p1->cd();
    gStyle->SetOptStat(0);
    if(hOccN && hOccN->Integral() > 10) hOccN->Draw("colz");
	
    c1->cd(4);
    TPad *p2 = new TPad("p2","p2",0.0,0.0,1.0,0.5);
    p2->Draw();
    p2->cd();
    gStyle->SetOptStat(0);
    if(hOccS && hOccS->Integral() > 10) hOccS->Draw("colz");
  }
 
  if(hTN){
    hTN->SetFillColor(kBlue);
    c1->cd(5);
    hTN->SetTitleSize(tsize,"xy");
    hTN->Draw("colz");
  }

  if(hTS){
    hTS->SetFillColor(kBlue);
    c1->cd(6);
    hTS->SetTitleSize(tsize,"xy");
    hTS->Draw("colz");
  }

}
