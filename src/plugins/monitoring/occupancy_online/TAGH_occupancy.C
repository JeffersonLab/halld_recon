//
// Guidance: --------------------------------------------
//
// This shows the tagger hodoscope occupancies.
//
// End Guidance: ----------------------------------------
//
//
//
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /occupancy/tag_num_events
// hnamepath: /occupancy/tagh_adc_occ
// hnamepath: /occupancy/tagh_tdc_occ
//
// e-mail: davidl@jlab.org
// e-mail: somov@jlab.org
// e-mail: richard.t.jones@uconn.edu
// e-mail: tbritton@jlab.org
//
{
	// RootSpy saves the current directory and style before
	// calling the macro and restores it after so it is OK to
	// change them and not change them back.
	TDirectory *savedir = gDirectory;
	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("occupancy");
	if(dir) dir->cd();

	double Nevents = 1.0;
	TH1I *tag_num_events = (TH1I*)gDirectory->FindObjectAny("tag_num_events");
	if(tag_num_events) Nevents = (double)tag_num_events->GetBinContent(1);

	TH2I *tagh_adc_occ = (TH2I*)gDirectory->FindObjectAny("tagh_adc_occ");
	TH2I *tagh_tdc_occ = (TH2I*)gDirectory->FindObjectAny("tagh_tdc_occ");

	TLegend *legend_sa = new TLegend(0.13,0.85,0.33,0.9);
	TLegend *legend_na = new TLegend(0.33,0.85,0.53,0.9);

	if(tagh_tdc_occ){
		tagh_tdc_occ->SetBarWidth(0.5);
		tagh_tdc_occ->SetBarOffset(0);
		tagh_tdc_occ->SetFillColor(kBlack);
		tagh_tdc_occ->SetStats(0);
		tagh_tdc_occ->SetTitle("TAGH column occupancy");
		tagh_tdc_occ->SetXTitle("Column number");
		tagh_tdc_occ->SetYTitle("fADC/TDC occupancy");
		tagh_tdc_occ->SetTitleSize(0.05,"X");
		tagh_tdc_occ->GetXaxis()->CenterTitle();
		tagh_tdc_occ->SetTitleSize(0.05,"Y");
		tagh_tdc_occ->GetYaxis()->CenterTitle();
	}
	
	if(tagh_adc_occ){
		tagh_adc_occ->SetBarWidth(0.5);
		tagh_adc_occ->SetBarOffset(0.5);
		tagh_adc_occ->SetFillColor(kGreen);
		tagh_adc_occ->SetStats(0);
		tagh_adc_occ->SetTitle("TAGH column occupancy");
		tagh_adc_occ->SetXTitle("Column number");
		tagh_adc_occ->SetYTitle("fADC/TDC occupancy");
		tagh_adc_occ->SetTitleSize(0.05,"X");
		tagh_adc_occ->GetXaxis()->CenterTitle();
		tagh_adc_occ->SetTitleSize(0.05,"Y");
		tagh_adc_occ->GetYaxis()->CenterTitle();
	}
	
	legend_sa->AddEntry(tagh_adc_occ,"fADC","f");
	legend_na->AddEntry(tagh_tdc_occ,"TDC","f");

	char nevents_str[256];
	sprintf(nevents_str,"%0.0f events", Nevents);
	TLatex lat;
	lat.SetTextAlign(22);
	lat.SetTextSize(0.035);

	// Just for testing
	if(gPad == NULL){
		TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	if(!gPad) {savedir->cd(); return;}

	TCanvas *c1 = gPad->GetCanvas();
	c1->cd(0);
	c1->SetLeftMargin(0.13);
	c1->Clear();

	// ----------------- TAGH ----------------------
	gPad->SetTicks();
	gPad->SetGridy();
	gPad->SetName("TAGH"); // used by RSAI in filenaming
	double ymax = 1.0;
	if(tagh_adc_occ) ymax = tagh_adc_occ->GetMaximum();
	if(tagh_tdc_occ){
		if(tagh_tdc_occ->GetMaximum() > ymax) ymax = tagh_tdc_occ->GetMaximum();
	}
	if(tagh_adc_occ){
		tagh_adc_occ->GetYaxis()->SetRangeUser(0.0, 1.05*ymax);
		tagh_adc_occ->Draw("BAR");
	}
	if(tagh_tdc_occ){
		tagh_tdc_occ->GetYaxis()->SetRangeUser(0.0, 1.05*ymax);
		tagh_tdc_occ->Draw( tagh_adc_occ==NULL ? "BAR":"BAR same" );
	}
	
	legend_sa->Draw();
	legend_na->Draw();
	lat.DrawLatex(225.0, 1.075*ymax, nevents_str);

#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
		auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
		if( min_events < 1 ) min_events = 1E4;
		if( Nevents >= min_events ) {
			cout << "TAGH Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
			rs_SavePad("TAGH_occupancy", 0);
			rs_ResetAllMacroHistos("//TAGH_occupancy");
		}
	}
#endif
}


