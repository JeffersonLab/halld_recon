
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /FMWPC/ctof_adc_events
// hnamepath: /FMWPC/ctof_tdc_events
// hnamepath: /FMWPC/ctof_adc_occ_up
// hnamepath: /FMWPC/ctof_adc_occ_down
// hnamepath: /FMWPC/ctof_tdc_occ_up
// hnamepath: /FMWPC/ctof_tdc_occ_down
//
// e-mail: aaustreg@jlab.org
// e-mail: zihlmann@jlab.org
//

{
	// RootSpy saves the current directory and style before
	// calling the macro and restores it after so it is OK to
	// change them and not change them back.
        TDirectory *savedir = gDirectory;
	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("FMWPC");
	if(dir) dir->cd();

	double adc_events = 1.0;
	TH1I *ctof_adc_events = (TH1I*)gDirectory->FindObjectAny("ctof_adc_events");
	if(ctof_adc_events) adc_events = (double)ctof_adc_events->GetBinContent(1);

	TH1I *ha_u = (TH1I*)gDirectory->FindObjectAny("ctof_adc_occ_up");
	TH1I *ha_d = (TH1I*)gDirectory->FindObjectAny("ctof_adc_occ_down");

	TH1I *h_u = (TH1I*)gDirectory->FindObjectAny("ctof_tdc_occ_up");
	TH1I *h_d = (TH1I*)gDirectory->FindObjectAny("ctof_tdc_occ_down");

	TLegend *legend_ua = new TLegend(0.5,0.85,0.7,0.9);
	TLegend *legend_da = new TLegend(0.7,0.85,0.9,0.9);

	TLegend *legend_u = new TLegend(0.5,0.85,0.7,0.9);
	TLegend *legend_d = new TLegend(0.7,0.85,0.9,0.9);

	if(ha_u){
		ha_u->SetBarWidth(0.5);
		ha_u->SetBarOffset(0);
		ha_u->SetFillColor(2);
		ha_u->SetStats(0);
		ha_u->SetXTitle("Bar number");
		ha_u->SetYTitle("fADC occupancy");
		ha_u->SetTitleSize(0.05,"X");
		ha_u->GetXaxis()->CenterTitle();
		ha_u->SetTitleSize(0.05,"Y");
		ha_u->GetYaxis()->CenterTitle();
		ha_u->GetYaxis()->SetRangeUser(0,1.1*ha_u->GetMaximum());
	}
	
	if(ha_d){
		ha_d->SetBarWidth(0.5);
		ha_d->SetBarOffset(0.5);
		ha_d->SetFillColor(3);
		ha_d->SetStats(0);
	}
	

	if(h_u){
		h_u->SetBarWidth(0.5);
		h_u->SetBarOffset(0);
		h_u->SetFillColor(2);
		h_u->SetStats(0);
		h_u->SetXTitle("Bar number");
		h_u->SetYTitle("TDC occupancy");
		h_u->SetTitleSize(0.05,"X");
		h_u->GetXaxis()->CenterTitle();
		h_u->SetTitleSize(0.05,"Y");
		h_u->GetYaxis()->CenterTitle();
		h_u->GetYaxis()->SetRangeUser(0,1.1*h_u->GetMaximum());
	}
	
	if(h_d){
		h_d->SetBarWidth(0.5);
		h_d->SetBarOffset(0.5);
		h_d->SetFillColor(3);
		h_d->SetStats(0);
	}


	legend_ua->AddEntry(ha_u,"Up","f");
	legend_da->AddEntry(ha_d,"Down","f");

	legend_u->AddEntry(h_u,"Up","f");
	legend_d->AddEntry(h_d,"Down","f");


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

	c1->Divide(3,3);
	TVirtualPad *pad1 = c1->cd(1);
	pad1->SetTicks();
	pad1->SetGridy();
	if(ha_u) ha_u->Draw("BAR");
	if(ha_d) ha_d->Draw("BAR sames");

	legend_ua->Draw();
	legend_da->Draw();

	TVirtualPad *pad2 = c1->cd(2);
	pad2->SetTicks();
	pad2->SetGridy();
	if(h_u) h_u->Draw("BAR");
	if(h_d) h_d->Draw("BAR sames");

	legend_u->Draw();
        legend_d->Draw();

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
