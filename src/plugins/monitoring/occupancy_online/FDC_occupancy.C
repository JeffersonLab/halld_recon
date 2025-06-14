
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /occupancy/fdc_num_events
// hnamepath: /occupancy/fdc_cathode_occ
// hnamepath: /occupancy/fdc_wire_occ
//
// e-mail: davidl@jlab.org
// e-mail: pentchev@jlab.org
// e-mail: staylor@jlab.org
// e-mail: tbritton@jlab.org
//

{
	// RootSpy saves the current directory and style before
	// calling the macro and restores it after so it is OK to
	// change them and not change them back.

	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("occupancy");
	if(dir) dir->cd();

	TH2I *fdc_cathode_occ = (TH2I*)gDirectory->FindObjectAny("fdc_cathode_occ");
	TH2I *fdc_wire_occ = (TH2I*)gDirectory->FindObjectAny("fdc_wire_occ");

	double Nevents = 1.0;
	TH1I *fdc_num_events = (TH1I*)gDirectory->FindObjectAny("fdc_num_events");
	if(fdc_num_events) Nevents = (double)fdc_num_events->GetBinContent(1);

	// Just for testing
	if(gPad == NULL){
		TCanvas *c1 = new TCanvas("c1");
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	if(!gPad){
		cout << "gPad is NULL!" << endl;
		return;
	}

	char nevents_str[256];
	sprintf(nevents_str,"%g events", Nevents);
	TLatex lat;
	lat.SetTextAlign(22);
	lat.SetTextSize(0.035);

	TCanvas *c1 = gPad->GetCanvas();
	c1->cd(0);
	c1->Clear();

	TPad *fdcpad1 = (TPad*)gDirectory->FindObjectAny("fdcpad1");
	if(!fdcpad1) fdcpad1 = new TPad("fdcpad1", "", 0.0, 0.0, 0.66, 1.0);
	fdcpad1->SetTicks();
	fdcpad1->SetLogz();
	fdcpad1->SetLeftMargin(0.10);
	fdcpad1->SetRightMargin(0.15);
	fdcpad1->Draw();
	fdcpad1->cd();
	if(fdc_cathode_occ){
		fdc_cathode_occ->SetStats(0);
		fdc_cathode_occ->SetYTitle("strip");
		fdc_cathode_occ->SetXTitle("cathode plane");
		TH1* h = fdc_cathode_occ->DrawCopy("colz");
		h->Scale(1./Nevents);
		h->GetZaxis()->SetRangeUser(0.001, 1.0);
	}
	lat.DrawLatex(25.0, 225.0 ,nevents_str);

	c1->cd(0);
	TPad *fdcpad2 = (TPad*)gDirectory->FindObjectAny("fdcpad2");
	if(!fdcpad2) fdcpad2 = new TPad("fdcpad2", "", 0.66, 0.0, 1.0, 1.0);
	fdcpad2->SetTicks();
	fdcpad2->SetLogz();
	fdcpad2->SetLeftMargin(0.10);
	fdcpad2->SetRightMargin(0.12);
	fdcpad2->Draw();
	fdcpad2->cd();
	if(fdc_wire_occ){
		fdc_wire_occ->SetStats(0);
		fdc_wire_occ->SetYTitle("wire");
		fdc_wire_occ->SetXTitle("wire plane");
		TH1* h = fdc_wire_occ->DrawCopy("colz");
		h->Scale(1./Nevents);
		h->GetZaxis()->SetRangeUser(0.001, 1.0);
	}
	lat.DrawLatex(13.5, 100.0,nevents_str);

#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
		auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
		if( min_events < 1 ) min_events = 1E4;
		if( Nevents >= min_events ) {
			cout << "FDC Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
			rs_SavePad("FDC_occupancy", 0);
			rs_ResetAllMacroHistos("//FDC_occupancy");
		}
	}
#endif
}
