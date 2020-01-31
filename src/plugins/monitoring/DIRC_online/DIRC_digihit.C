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
  
	auto hDigiHit_Time_scaled = (TH1D*)gDirectory->Get("hDigiHit_Time_scaled");
	if( !hDigiHit_Time_scaled ) hDigiHit_Time_scaled = (TH1D*)hDigiHit_Time->Clone("hDigiHit_Time_scaled");
	hDigiHit_Time_scaled->SetContent(hDigiHit_Time->GetArray());

	auto hDigiHit_Time_LED_scaled = (TH1D*)gDirectory->Get("hDigiHit_Time_LED_scaled");
	if( !hDigiHit_Time_LED_scaled ) hDigiHit_Time_LED_scaled = (TH1D*)hDigiHit_Time_LED->Clone("hDigiHit_Time_LED_scaled");
	hDigiHit_Time_LED_scaled->SetContent(hDigiHit_Time_LED->GetArray());

    hDigiHit_Time_scaled->SetLineColor(kBlack);
    hDigiHit_Time_LED_scaled->SetLineColor(kBlue);
    c1->cd(3);
    hDigiHit_Time->SetTitleSize(tsize,"xy");
    hDigiHit_Time_LED_scaled->Draw();
    double scale = hDigiHit_Time_LED_scaled->GetMaximum()/hDigiHit_Time_scaled->GetMaximum();
    if(hDigiHit_Time_scaled->GetMaximum() == 0) scale = 1.;
    hDigiHit_Time_scaled->Scale(scale);
    hDigiHit_Time_scaled->Draw("h same");
  }

  if(hDigiHit_Time_North && hDigiHit_Time_LED_North) {

	auto hDigiHit_Time_North_scaled = (TH1D*)gDirectory->Get("hDigiHit_Time_North_scaled");
	if( !hDigiHit_Time_North_scaled ) hDigiHit_Time_North_scaled = (TH1D*)hDigiHit_Time_North->Clone("hDigiHit_Time_North_scaled");
	hDigiHit_Time_North_scaled->SetContent(hDigiHit_Time_North->GetArray());

	auto hDigiHit_Time_North_LED_scaled = (TH1D*)gDirectory->Get("hDigiHit_Time_North_LED_scaled");
	if( !hDigiHit_Time_North_LED_scaled ) hDigiHit_Time_North_LED_scaled = (TH1D*)hDigiHit_Time_North_LED->Clone("hDigiHit_Time_North_LED_scaled");
	hDigiHit_Time_North_LED_scaled->SetContent(hDigiHit_Time_North_LED->GetArray());

    hDigiHit_Time_North_North->SetLineColor(kBlack);
    hDigiHit_Time_North_LED_North->SetLineColor(kBlue);
    c1->cd(4);
    hDigiHit_Time_North_North->SetTitleSize(tsize,"xy");
    hDigiHit_Time_North_LED_North->Draw();
    double scale = hDigiHit_Time_North_LED_North->GetMaximum()/hDigiHit_Time_North_North->GetMaximum();
    if(hDigiHit_Time_North_North->GetMaximum() == 0) scale = 1.;
    hDigiHit_Time_North_North->Scale(scale);
    hDigiHit_Time_North_North->Draw("h same");
  }

}
