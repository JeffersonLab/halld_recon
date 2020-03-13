// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// Guidance: --------------------------------------------
//
// Upper 2 panels: DIRC PMT hit time for physics triggers 
// (black) and LED triggers (blue) for South and North boxes
// Lower left panel: LED reference time signal from SiPM: 
// ADC (red) and TDC (blue), both should have clear single peaks
// Lower right panel: LED reference time signal ADC vs TDC
//
// If you have concerns about the plots or any of them look
// significantly different than the reference, please contact 
// one of:
//
//  Run Coordinator:  (757) 383-5542
//
//  Justin Stevens:   x7237    (office)
//                    585-4870 (cell)
//
// End Guidance: ----------------------------------------
//
// hnamepath: /DIRC_online/Hit/SouthLowerBox/Hit_Time_NonLED
// hnamepath: /DIRC_online/Hit/SouthLowerBox/Hit_Time_LED
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_Time_NonLED
// hnamepath: /DIRC_online/Hit/NorthUpperBox/Hit_Time_LED
// hnamepath: /DIRC_online/LEDRefAdcTime
// hnamepath: /DIRC_online/LEDRefTdcTime
// hnamepath: /DIRC_online/LEDRefAdcVsTdcTime
//
// e-mail: davidl@jlab.org
// e-mail: jrsteven@jlab.org
// e-mail: billlee@jlab.org
// e-mail: tbritton@jlab.org

{  
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("DIRC_online");
  if(dir) dir->cd();

  TH1I* hTS = (TH1I*)gDirectory->Get("Hit/SouthLowerBox/Hit_Time_NonLED");
  TH1I* hTS_LED = (TH1I*)gDirectory->Get("Hit/SouthLowerBox/Hit_Time_LED");
  TH1I* hTS_North = (TH1I*)gDirectory->Get("Hit/NorthUpperBox/Hit_Time_NonLED");
  TH1I* hTS_LED_North = (TH1I*)gDirectory->Get("Hit/NorthUpperBox/Hit_Time_LED");

  TH1I* hAdcTime = (TH1I*)gDirectory->Get("LEDRefAdcTime");
  TH1I* hTdcTime = (TH1I*)gDirectory->Get("LEDRefTdcTime");
  TH2I* hAdcVsTdcTime = (TH2I*)gDirectory->Get("LEDRefAdcVsTdcTime");

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

  if(hTS && hTS_LED){
    hTS->SetLineColor(kBlack);
    hTS_LED->SetLineColor(kBlue);
    c1->cd(1);
    hTS_LED->SetTitleSize(tsize,"xy");
    hTS_LED->Draw();
    hTS->Scale(hTS_LED->GetMaximum()/hTS->GetMaximum());
    hTS->Draw("h same");

    TLegend *leg = new TLegend(0.6, 0.6, 0.85, 0.8);
    leg->AddEntry(hTS_LED,"LED trigger","l");
    leg->AddEntry(hTS,"Non-LED triggers","l");
    leg->Draw("same");
  }

  if(hTS_North && hTS_LED_North){
    hTS_North->SetLineColor(kBlack);
    hTS_LED_North->SetLineColor(kBlue);
    c1->cd(2);
    hTS_LED_North->SetTitleSize(tsize,"xy");
    hTS_LED_North->Draw();
    hTS_North->Scale(hTS_LED_North->GetMaximum()/hTS_North->GetMaximum());
    hTS_North->Draw("h same");
  }

  if(hAdcTime && hAdcTime){
    hAdcTime->SetLineColor(kRed);
    hTdcTime->SetLineColor(kGreen);
    c1->cd(3);
    hAdcTime->SetTitleSize(tsize,"xy");
    hAdcTime->Draw();
    hTdcTime->Scale(hAdcTime->GetMaximum()/hTdcTime->GetMaximum());
    hTdcTime->Draw("h same");

    TLegend *leg = new TLegend(0.6, 0.6, 0.85, 0.8);
    leg->AddEntry(hAdcTime,"LED ref ADC","l");
    leg->AddEntry(hTdcTime,"LED ref TDC ","l");
    leg->Draw("same");
  }

  if(hAdcVsTdcTime){
    c1->cd(4);
    hAdcVsTdcTime->Draw("colz");
  }

}
