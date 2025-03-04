//
// Guidance: --------------------------------------------
//
// TBD
//
// End Guidance: ----------------------------------------
//
//
//
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /occupancy/ecal_occ
// hnamepath: /occupancy/ecal_num_events
//
// e-mail: davidl@jlab.org
// e-mail: tbritton@jlab.org
// e-mail: somov@jlab.org
// e-mail: staylor@jlab.org
//

{
	// RootSpy saves the current directory and style before
	// calling the macro and restores it after so it is OK to
	// change them and not change them back.
        TDirectory *savedir = gDirectory;
	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("occupancy");
	if(dir) dir->cd();

	TH2I *ecal_occ = (TH2I*)gDirectory->FindObjectAny("ecal_occ");
	TH1I *ecal_num_events = (TH1I*)gDirectory->FindObjectAny("ecal_num_events");

	double Nevents = 1.0;
	if(ecal_num_events) Nevents = (double)ecal_num_events->GetBinContent(1);

	TLegend *legend_sa = new TLegend(0.1,0.85,0.3,0.9);
	if(ecal_occ)legend_sa->AddEntry(ecal_occ, "fADC","f");

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
	gPad->SetRightMargin(2.0);
	gPad->SetLeftMargin(2.0);
	if(ecal_occ){
		ecal_occ->SetStats(0);
		TH1* h = ecal_occ->DrawCopy("colz");
		h->Scale(1./Nevents);
		h->GetZaxis()->SetRangeUser(0.0001, 0.25);
		
		char str[256];
		sprintf(str,"%0.0f events", Nevents);
		TLatex lat(30.0, 61.75, str);
		lat.SetTextAlign(22);
		lat.SetTextSize(0.035);
		lat.Draw();
	}

	legend_sa->Draw();

#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
		auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
		if( min_events < 1 ) min_events = 1E4;
		if( Nevents >= min_events ) {
			cout << "ECAL Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
			rs_SavePad("ECAL_occupancy", 0);
			rs_ResetAllMacroHistos("//ECAL_occupancy");
		}
	}
#endif
}
