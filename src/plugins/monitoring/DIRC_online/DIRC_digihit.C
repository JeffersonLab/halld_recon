// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /DIRC_online/DigiHit/DigiHit_NHitsVsBox_LED
// hnamepath: /DIRC_online/DigiHit/DigiHit_NHitsVsBox_NonLED
// hnamepath: /DIRC_online/DigiHit/SouthLowerBox/TDCDigiHit_Time_LED
// hnamepath: /DIRC_online/DigiHit/SouthLowerBox/TDCDigiHit_Time_NonLED
// hnamepath: /DIRC_online/DigiHit/NorthUpperBox/TDCDigiHit_Time_LED
// hnamepath: /DIRC_online/DigiHit/NorthUpperBox/TDCDigiHit_Time_NonLED
//
// e-mail: davidl@jlab.org
// e-mail: jrsteven@jlab.org
// e-mail: billlee@jlab.org
// e-mail: tbritton@jlab.org


{
  //gDirectory->cd("rootspy");
  
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("DIRC_online");
  if(dir) dir->cd();

  TH2I* hDigiHit_NhitsVsBox_LED = (TH2I*)gDirectory->Get("DigiHit/DigiHit_NHitsVsBox_LED");
  TH2I* hDigiHit_NhitsVsBox = (TH2I*)gDirectory->Get("DigiHit/DigiHit_NHitsVsBox_NonLED");
  TH1I* hDigiHit_Nhits_LED = (TH1I*)hDigiHit_NhitsVsBox_LED->ProjectionY("Nhits_LED",2,2);
  TH1I* hDigiHit_Nhits_LED_North = (TH1I*)hDigiHit_NhitsVsBox_LED->ProjectionY("Nhits_LED_North",1,1);
  TH1I* hDigiHit_Nhits = (TH1I*)hDigiHit_NhitsVsBox->ProjectionY("Nhits",2,2);
  TH1I* hDigiHit_Nhits_North = (TH1I*)hDigiHit_NhitsVsBox->ProjectionY("Nhits_North",1,1);

  TH1I* hDigiHit_Time_LED = (TH1I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_Time_LED");
  TH1I* hDigiHit_Time = (TH1I*)gDirectory->Get("DigiHit/SouthLowerBox/TDCDigiHit_Time_NonLED");
  TH1I* hDigiHit_Time_LED_North = (TH1I*)gDirectory->Get("DigiHit/NorthUpperBox/TDCDigiHit_Time_LED");
  TH1I* hDigiHit_Time_North = (TH1I*)gDirectory->Get("DigiHit/NorthUpperBox/TDCDigiHit_Time_NonLED");

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

    TLegend *leg = new TLegend(0.6, 0.6, 0.85, 0.8);
    leg->AddEntry(hDigiHit_Nhits_LED,"LED trigger","l");
    leg->AddEntry(hDigiHit_Nhits,"Non-LED triggers","l");
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

  if(hDigiHit_Nhits_North && hDigiHit_Nhits_LED_North) { 
    hDigiHit_Nhits_North->SetLineColor(kBlack);
    hDigiHit_Nhits_LED_North->SetLineColor(kBlue);
    c1->cd(2);
    hDigiHit_Nhits_LED_North->SetTitleSize(tsize,"xy");
    hDigiHit_Nhits_LED_North->Draw();
    double scale = hDigiHit_Nhits_LED_North->GetMaximum()/hDigiHit_Nhits_North->GetMaximum();
    if(hDigiHit_Nhits_North->GetMaximum() == 0) scale = 1.;
    hDigiHit_Nhits_North->Scale(scale);
    hDigiHit_Nhits_North->Draw("h same");
  }

  if(hDigiHit_Time_North && hDigiHit_Time_LED_North) {
  
    auto h1 = (TH1I*)gDirectory->Get("hDigiHit_Time_scaled");
    if( !h1 ) h1 = (TH1I*)hDigiHit_Time->Clone("hDigiHit_Time_scaled");
	 for( int ibin=1; ibin<=hDigiHit_Time->GetNbinsX(); ibin++) h1->SetBinContent( ibin, (Int_t)hDigiHit_Time->GetBinContent(ibin));
    //h1->SetContent(hDigiHit_Time->GetArray());

    auto h2 = (TH1I*)gDirectory->Get("h2");
    if( !h2 ) h2 = (TH1I*)hDigiHit_Time_LED->Clone("h2");
	 for( int ibin=1; ibin<=hDigiHit_Time_LED->GetNbinsX(); ibin++) h2->SetBinContent( ibin, (Int_t)hDigiHit_Time_LED->GetBinContent(ibin));
    //h2->SetContent(hDigiHit_Time_LED->GetArray());

    h1->SetLineColor(kBlack);
    h2->SetLineColor(kBlue);
    c1->cd(3);
    hDigiHit_Time->SetTitleSize(tsize,"xy");
    h2->Draw();
    double scale = h2->GetMaximum()/h1->GetMaximum();
    if(h1->GetMaximum() == 0) scale = 1.;
    h1->Scale(scale);
    h1->Draw("h same");
  }

  if(hDigiHit_Time_North && hDigiHit_Time_LED_North) {

    auto h1 = (TH1I*)gDirectory->Get("hDigiHit_Time_North_scaled");
    if( !h1 ) h1 = (TH1I*)hDigiHit_Time_North->Clone("hDigiHit_Time_North_scaled");
	 for( int ibin=1; ibin<=hDigiHit_Time_North->GetNbinsX(); ibin++) h1->SetBinContent( ibin, (Int_t)hDigiHit_Time_North->GetBinContent(ibin));
    //h1->SetContent(hDigiHit_Time_North->GetArray());

    auto h2 = (TH1I*)gDirectory->Get("hDigiHit_Time_North_LED_scaled");
    if( !h2 ) h2 = (TH1I*)hDigiHit_Time_LED_North->Clone("hDigiHit_Time_North_LED_scaled");
	 for( int ibin=1; ibin<=hDigiHit_Time_LED_North->GetNbinsX(); ibin++) h2->SetBinContent( ibin, (Int_t)hDigiHit_Time_LED_North->GetBinContent(ibin));
    //h2->SetContent(hDigiHit_Time_North_LED->GetArray());

    h1->SetLineColor(kBlack);
    h2->SetLineColor(kBlue);
    c1->cd(4);
    h2->SetTitleSize(tsize,"xy");
    h2->Draw();
    double scale = h2->GetMaximum()/h1->GetMaximum();
    if(h1->GetMaximum() == 0) scale = 1.;
    h1->Scale(scale);
    h1->Draw("h same");
  }

}
