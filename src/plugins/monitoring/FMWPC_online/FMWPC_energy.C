// The following are special comments used by RootSpy to know                                                                        
// which histograms to fetch for the macro.                                                                                          
//
// hnamepath: /FMWPC/h2_fmwpc_time_chamber
// hnamepath: /FMWPC/h2_fmwpc_pic_chamber
{
  TDirectory *savedir = gDirectory;
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("FMWPC");
  if(dir) dir->cd();

  //Calibrated Hit                                                                                                             
  TH1F *fmwpc_hit_layer_1 = (TH1F*)gDirectory->FindObjectAny("fmwpc_hit_layer_1");
  TH1F *fmwpc_hit_layer_2 = (TH1F*)gDirectory->FindObjectAny("fmwpc_hit_layer_2");
  TH1F *fmwpc_hit_layer_3 = (TH1F*)gDirectory->FindObjectAny("fmwpc_hit_layer_3");
  TH1F *fmwpc_hit_layer_4 = (TH1F*)gDirectory->FindObjectAny("fmwpc_hit_layer_4");
  TH1F *fmwpc_hit_layer_5 = (TH1F*)gDirectory->FindObjectAny("fmwpc_hit_layer_5");
  TH1F *fmwpc_hit_layer_6 = (TH1F*)gDirectory->FindObjectAny("fmwpc_hit_layer_6");

  //Chamber Times                                                                                                              
  TH1F *fmwpc_time_chamber_1 = (TH1F*)gDirectory->FindObjectAny("fmwpc_time_chamber_1");
  TH1F *fmwpc_time_chamber_2 = (TH1F*)gDirectory->FindObjectAny("fmwpc_time_chamber_2");
  TH1F *fmwpc_time_chamber_3 = (TH1F*)gDirectory->FindObjectAny("fmwpc_time_chamber_3");
  TH1F *fmwpc_time_chamber_4 = (TH1F*)gDirectory->FindObjectAny("fmwpc_time_chamber_4");
  TH1F *fmwpc_time_chamber_5 = (TH1F*)gDirectory->FindObjectAny("fmwpc_time_chamber_5");
  TH1F *fmwpc_time_chamber_6 = (TH1F*)gDirectory->FindObjectAny("fmwpc_time_chamber_6");

  //Chamber Pulse Integral                                                                                                     
  TH1D *fmwpc_pulse_integral_chamber_1 = (TH1D*)gDirectory->FindObjectAny("fmwpc_pulse_integral_chamber_1");
  TH1D *fmwpc_pulse_integral_chamber_2 = (TH1D*)gDirectory->FindObjectAny("fmwpc_pulse_integral_chamber_2");
  TH1D *fmwpc_pulse_integral_chamber_3 = (TH1D*)gDirectory->FindObjectAny("fmwpc_pulse_integral_chamber_3");
  TH1D *fmwpc_pulse_integral_chamber_4 = (TH1D*)gDirectory->FindObjectAny("fmwpc_pulse_integral_chamber_4");
  TH1D *fmwpc_pulse_integral_chamber_5 = (TH1D*)gDirectory->FindObjectAny("fmwpc_pulse_integral_chamber_5");
  TH1D *fmwpc_pulse_integral_chamber_6 = (TH1D*)gDirectory->FindObjectAny("fmwpc_pulse_integral_chamber_6");

  TH1I *fmwpc_occ_chamber = (TH1I*)gDirectory->FindObjectAny("h1_fmwpc_occ_chamber");

  //For Rootspy
  TH2F *h2_fmwpc_time_chamber = (TH2F*)gDirectory->FindObjectAny("h2_fmwpc_time_chamber");
  TH2D *h2_fmwpc_pi_chamber = (TH2D*)gDirectory->FindObjectAny("h2_fmwpc_pi_chamber");

  // Just for testing
  if(gPad == NULL){
    TCanvas *c1 = new TCanvas("c1");
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }
  if(!gPad) {savedir->cd(); return;}

  TCanvas *c1 = gPad->GetCanvas();
  c1->Divide(2,1);

  TVirtualPad *pad1 = c1->cd(1);
  pad1->SetTicks();
  pad1->SetLeftMargin(0.15);
  pad1->SetRightMargin(0.15);
  if(h2_fmwpc_time_chamber){
    h2_fmwpc_time_chamber->SetStats(0);
    h2_fmwpc_time_chamber->GetXaxis()->SetLabelSize(0.05);
    h2_fmwpc_time_chamber->GetYaxis()->SetLabelSize(0.05);
    h2_fmwpc_time_chamber->GetXaxis()->SetTitleSize(0.05);
    h2_fmwpc_time_chamber->Draw("h colz0");
  }

  TVirtualPad *pad2 = c1->cd(2);
  pad2->SetTicks();
  pad2->SetLeftMargin(0.15);
  pad2->SetRightMargin(0.15);
  if(h2_fmwpc_pi_chamber){
    h2_fmwpc_pi_chamber->SetStats(0);
    h2_fmwpc_pi_chamber->GetXaxis()->SetLabelSize(0.05);
    h2_fmwpc_pi_chamber->GetYaxis()->SetLabelSize(0.05);
    h2_fmwpc_pi_chamber->GetXaxis()->SetTitleSize(0.05);
    h2_fmwpc_pi_chamber->Draw("h colz0");
  }

#ifdef ROOTSPY_MACROS
    // ------ The following is used by RSAI --------                                                                             
    if( rs_GetFlag("Is_RSAI")==1 ){
      auto min_events = 5*rs_GetFlag("MIN_EVENTS_RSAI");
      if( min_events < 1 ) min_events = 5E5;
      if( Nevents >= min_events ) {
	cout << "FMWPC Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
	rs_SavePad("FMWPC_occupancy", 0);
	rs_ResetAllMacroHistos("//FMWPC_occupancy");
      }
    }
#endif

}
