//
// Guidance: --------------------------------------------
//
// Single channels may show low or no number of entries in occupancy plots
// during normal operation. This could number anywhere from 0 to ~20
// channels, with slow time dependency (0-3 channels/day). These channels are
// expected to be randomly spread around the FCAL, and may or may not also
// line up with yellow/red status on the FCAL HV GUI. Any behavior other than
// what's described here should be reported to FCAL experts.
//
// End Guidance: ----------------------------------------
//
//
//
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /occupancy/fcal_occ
// hnamepath: /occupancy/fcal_num_events
//
// e-mail: dalton@jlab.org
// e-mail: malte@jlab.org
// e-mail: shepherd@jlab.org
//

{
	// RootSpy saves the current directory and style before
	// calling the macro and restores it after so it is OK to
	// change them and not change them back.
        TDirectory *savedir = gDirectory;
	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("occupancy");
	if(dir) dir->cd();

	TH2I *fcal_occ = (TH2I*)gDirectory->FindObjectAny("fcal_occ");
	TH1I *fcal_num_events = (TH1I*)gDirectory->FindObjectAny("fcal_num_events");

	double Nevents = 1.0;
	if(fcal_num_events) Nevents = (double)fcal_num_events->GetBinContent(1);

	// Just for testing
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

	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetRightMargin(0.28);
	gPad->SetLeftMargin(0.24);
	gPad->SetBottomMargin(0.11);
	gPad->SetTopMargin(0.11);
	if(fcal_occ){
		fcal_occ->SetStats(0);
		TH1* h = fcal_occ->DrawCopy("colz");
		h->Scale(1./Nevents);

		float max = h->GetMaximum();
		float setmax = h->Integral()/2360.*4; // set maximum to 4 times the average bin content
		printf("%f %f %f\n",setmax, max, setmax/max);
		if (setmax<0.01) setmax=0.01;
		h->GetZaxis()->SetRangeUser(0.00001, setmax);

		char str[256];
		sprintf(str,"%0.0f events", Nevents);
		TLatex lat;
		lat.SetTextAlign(22);
		lat.SetTextSize(0.035);
		lat.DrawLatexNDC(0.7,0.92,str);
	}

#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
		auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
		if( min_events < 1 ) min_events = 1E4;
		if( Nevents >= min_events ) {
			cout << "FCAL Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
			rs_SavePad("FCAL_occupancy", 0);
			rs_ResetAllMacroHistos("//FCAL_occupancy");
		}
	}
#endif
}
