//
// Guidance: --------------------------------------------
//
// This shows the tagger microscope occupancies.
//
// Please note that for amorphous radiator running there will be
// 3 channels in the upper plot of fADC values around colun 82
// that are nearly twice as high as its neighbors. This effect
// is not as pronounced for the diamond radiator which is shown
// in the reference plot. This is expected and should be ignored.
//
// End Guidance: ----------------------------------------
//
//
//
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /occupancy/tag_num_events
// hnamepath: /occupancy/tagm_adc_occ
// hnamepath: /occupancy/tagm_tdc_occ
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

	TH2I *tagm_adc_occ = (TH2I*)gDirectory->FindObjectAny("tagm_adc_occ");
	TH2I *tagm_tdc_occ = (TH2I*)gDirectory->FindObjectAny("tagm_tdc_occ");

	TLegend *legend_sa = new TLegend(0.13,0.85,0.33,0.9);
	TLegend *legend_na = new TLegend(0.33,0.85,0.53,0.9);

	if(tagm_tdc_occ){
		tagm_tdc_occ->SetBarWidth(0.5);
		tagm_tdc_occ->SetBarOffset(0);
		tagm_tdc_occ->SetFillColor(kMagenta);
		tagm_tdc_occ->SetStats(0);
		tagm_tdc_occ->SetTitle("TAGM column occupancy");
		tagm_tdc_occ->SetXTitle("Column number");
		tagm_tdc_occ->SetYTitle("fADC/TDC occupancy");
		tagm_tdc_occ->SetTitleSize(0.05,"X");
		tagm_tdc_occ->GetXaxis()->CenterTitle();
		tagm_tdc_occ->SetTitleSize(0.05,"Y");
		tagm_tdc_occ->GetYaxis()->CenterTitle();
	}
	
	if(tagm_adc_occ){
		tagm_adc_occ->SetBarWidth(0.5);
		tagm_adc_occ->SetBarOffset(0.5);
		tagm_adc_occ->SetFillColor(kBlue);
		tagm_adc_occ->SetStats(0);
		tagm_adc_occ->SetTitle("TAGM column occupancy");
		tagm_adc_occ->SetXTitle("Column number");
		tagm_adc_occ->SetYTitle("fADC/TDC occupancy");
		tagm_adc_occ->SetTitleSize(0.05,"X");
		tagm_adc_occ->GetXaxis()->CenterTitle();
		tagm_adc_occ->SetTitleSize(0.05,"Y");
		tagm_adc_occ->GetYaxis()->CenterTitle();
	}
	
	legend_sa->AddEntry(tagm_adc_occ,"fADC","f");
	legend_na->AddEntry(tagm_tdc_occ,"TDC","f");

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

	// ----------------- TAGM ----------------------
	gPad->SetTicks();
	gPad->SetGridy();
	gPad->SetName("TAGM"); // used by RSAI in filenaming
	double ymax = 1.0;
	if(tagm_adc_occ) ymax = tagm_adc_occ->GetMaximum();
	if(tagm_tdc_occ){
		if(tagm_tdc_occ->GetMaximum() > ymax) ymax = tagm_tdc_occ->GetMaximum();
	}
	if(tagm_adc_occ){
		tagm_adc_occ->GetYaxis()->SetRangeUser(0.0, 1.05*ymax);
		tagm_adc_occ->Draw("BAR");
	}
	if(tagm_tdc_occ){
		tagm_tdc_occ->GetYaxis()->SetRangeUser(0.0, 1.05*ymax);
		tagm_tdc_occ->Draw( tagm_adc_occ==NULL ? "BAR":"BAR same" );
	}
	
	legend_sa->Draw();
	legend_na->Draw();
	lat.DrawLatex(85.0, 1.075*ymax, nevents_str);

#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
		auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
		if( min_events < 1 ) min_events = 1E4;
		if( Nevents >= min_events ) {
			cout << "TAGM Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
			rs_SavePad("TAGM_occupancy", 0);
			rs_ResetAllMacroHistos("//TAGM_occupancy");
		}
	}
#endif
}


