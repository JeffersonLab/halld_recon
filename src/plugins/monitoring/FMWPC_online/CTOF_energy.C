
// The following are special comments used by RootSpy to know                                                                        
// which histograms to fetch for the macro.                                                                                          
//                                                                                                                                   
// hnamepath: /FMWPC/h1_ctof_dE
// hnamepath: /FMWPC/h2_ctof_t
// hnamepath: /FMWPC/h2_ctof_t_adc
//
//

{
  TDirectory *savedir = gDirectory;
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("FMWPC");
  if(dir) dir->cd();

  TH1I *hr_u = (TH1I*)gDirectory->FindObjectAny("h1_ctof_occ_up");
  TH1I *hr_d = (TH1I*)gDirectory->FindObjectAny("h1_ctof_occ_down");

  TH2F *h_de = (TH2F*)gDirectory->FindObjectAny("h1_ctof_dE");
  TH2D *h_t = (TH2D*)gDirectory->FindObjectAny("h2_ctof_t");
  TH2D *h_t_adc = (TH2D*)gDirectory->FindObjectAny("h2_ctof_t_adc");
  TH2D *h_adc_tdc = (TH2D*)gDirectory->FindObjectAny("h2_ctof_t_adc_tdc");

  TLegend *legend_ru = new TLegend(0.5,0.85,0.7,0.9);
  TLegend *legend_rd = new TLegend(0.7,0.85,0.9,0.9);

  if(hr_u){
    hr_u->SetBarWidth(0.5);
    hr_u->SetBarOffset(0);
    hr_u->SetFillColor(2);
    hr_u->SetStats(0);
    hr_u->SetXTitle("Bar Number");
    hr_u->SetYTitle("Occupancy");
    hr_u->SetTitleSize(0.05,"X");
    hr_u->GetXaxis()->CenterTitle();
    hr_u->SetTitleSize(0.05,"Y");
    hr_u->GetYaxis()->CenterTitle();
    hr_u->GetYaxis()->SetRangeUser(0,1.1*hr_u->GetMaximum());
  }
  if(hr_d){
    hr_d->SetBarWidth(0.5);
    hr_d->SetBarOffset(0.5);
    hr_d->SetFillColor(3);
    hr_d->SetStats(0);
  }

  if(h_de){
    h_de->SetXTitle("Bar Number");
    h_de->SetYTitle("Energy GeV");
    h_de->SetStats(0);
    h_de->SetTitleSize(0.05,"X");
    h_de->GetXaxis()->CenterTitle();
    h_de->SetTitleSize(0.05,"Y");
    h_de->GetYaxis()->CenterTitle();
  }

  if(h_t){
    h_t->SetXTitle("Bar Number");
    h_t->SetYTitle("Calibrated Time");
    h_t->SetStats(0);
    h_t->SetTitleSize(0.05,"X");
    h_t->GetXaxis()->CenterTitle();
    h_t->SetTitleSize(0.05,"Y");
    h_t->GetYaxis()->CenterTitle();
  }

  if(h_t_adc){
    h_t_adc->SetXTitle("Bar Number");
    h_t_adc->SetYTitle("Calibrated ADC Time");
    h_t_adc->SetStats(0);
    h_t_adc->SetTitleSize(0.05,"X");
    h_t_adc->GetXaxis()->CenterTitle();
    h_t_adc->SetTitleSize(0.05,"Y");
    h_t_adc->GetYaxis()->CenterTitle();
  }

  if(h_adc_tdc){
    h_adc_tdc->SetXTitle("Bar Number");
    h_adc_tdc->SetYTitle("ADC - TDC Time");
    h_adc_tdc->SetTitleSize(0.05,"X");
    h_adc_tdc->GetXaxis()->CenterTitle();
    h_adc_tdc->SetTitleSize(0.05,"Y");
    h_adc_tdc->GetYaxis()->CenterTitle();
  }

  legend_ru->AddEntry(hr_u,"Up","f");
  legend_rd->AddEntry(hr_d,"Down","f");

  if(gPad == NULL){
    TCanvas *c1 = new TCanvas("c1");
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }
  if(!gPad) {savedir->cd(); return;}

  TCanvas *c1 = gPad->GetCanvas();
  c1->cd(0);
  c1->Clear();

  c1->Divide(3,1);

  //TVirtualPad *pad1 = c1->cd(1);
  //pad1->SetTicks();
  //pad1->SetGridy();
  //if(hr_u) hr_u->Draw("BAR");
  //if(hr_d) hr_d->Draw("BAR sames");

  //legend_ru->Draw();
  //legend_rd->Draw();

  TVirtualPad *pad1 = c1->cd(1);
  pad1->SetTicks();
  pad1->SetGridy();
  if(h_de) h_de->Draw("BAR colz0");

  TVirtualPad *pad2 = c1->cd(2);
  pad2->SetTicks();
  pad2->SetGridy();
  if(h_t) h_t->Draw("BAR colz0");

  //TVirtualPad *pad4 = c1->cd(4);
  //pad4->SetTicks();
  //pad4->SetGridy();
  //if(h_t_adc) h_t_adc->Draw("BAR colz0");

  TVirtualPad *pad3 = c1->cd(3);
  pad3->SetTicks();
  pad3->SetGridy();
  if(h_adc_tdc) h_adc_tdc->Draw("BAR colz0");

#ifdef ROOTSPY_MACROS
  // ------ The following is used by RSAI --------                                                                             
  if( rs_GetFlag("Is_RSAI")==1 ){
    auto min_events = 5*rs_GetFlag("MIN_EVENTS_RSAI");
    if( min_events < 1 ) min_events = 5E5;
    if( adc_events >= min_events ) {
      cout << "CTOF Flagging AI check after " << adc_events << " events (>=" << min_events << ")" << endl;
      rs_SavePad("CTOF_occupancy", 0);
      rs_ResetAllMacroHistos("//CTOF_occupancy");
    }
  }
#endif

}
