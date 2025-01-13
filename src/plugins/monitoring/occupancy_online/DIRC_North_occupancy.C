
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// Guidance: --------------------------------------------
//
// Upper panel: DIRC PMT occupancy for LED triggers. If this
// is empty then the DIRC LED is not functioning properly.
// Lower panel: DIRC PMT occupancy for non-LED triggers. If 
// this is empty then there may not be any physics triggers.
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
// hnamepath: /occupancy/dirc_num_events
// hnamepath: /occupancy/dirc_tdc_pixel_N_occ
// hnamepath: /occupancy/dirc_tdc_pixel_N_occ_led
//
// e-mail: davidl@jlab.org
// e-mail: tbritton@jlab.org
//

{
	// RootSpy saves the current directory and style before
	// calling the macro and restores it after so it is OK to
	// change them and not change them back.
   	TDirectory *savedir = gDirectory;
	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("occupancy");
	if(dir) dir->cd();

	TH2I *dirc_tdc_pixel_N_occ_led = (TH2I*)gDirectory->FindObjectAny("dirc_tdc_pixel_N_occ_led");
	TH2I *dirc_tdc_pixel_N_occ = (TH2I*)gDirectory->FindObjectAny("dirc_tdc_pixel_N_occ");
	TH1I *dirc_num_events = (TH1I*)gDirectory->FindObjectAny("dirc_num_events");

	double Nevents = 1.0;
	if(dirc_num_events) Nevents = (double)dirc_num_events->GetBinContent(1);

	// Just for testing
	if(gPad == NULL){
		TCanvas *c1 = new TCanvas("c1","DIRC North Occupancy",600,400);
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	if(!gPad) {savedir->cd(); return;}

	TCanvas *c1 = gPad->GetCanvas();
	c1->cd(0);
	c1->Clear();

	TPad *p1 = (TPad*)gDirectory->FindObjectAny("dirc_occ_pad1");
	if(!p1)p1 = new TPad("dirc_occ_pad1","p1",0.,0.5,1.,1.);
	p1->Draw();
	p1->cd();
	gStyle->SetOptStat(0);
	if(dirc_tdc_pixel_N_occ_led) dirc_tdc_pixel_N_occ_led->DrawCopy("colz");

	c1->cd(0);
	TPad *p2 = (TPad*)gDirectory->FindObjectAny("dirc_occ_pad2");
	if(!p2)p2 = new TPad("dirc_occ_pad2","p1",0.,0.,1.,0.5);
	p2->Draw();
	p2->cd();
	gStyle->SetOptStat(0);
	if(dirc_tdc_pixel_N_occ) dirc_tdc_pixel_N_occ->DrawCopy("colz");

#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
		auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
		if( min_events < 1 ) min_events = 1E4;
		if( Nevents >= min_events ) {
			cout << "DIRC Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
			rs_SavePad("DIRC_North_occupancy", 0);
			rs_ResetAllMacroHistos("//DIRC_North_occupancy");
		}
	}
#endif
}
