
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /occupancy/cdc_num_events
// hnamepath: /occupancy/cdc_axes
// hnamepath: /occupancy/cdc_occ_ring_01
// hnamepath: /occupancy/cdc_occ_ring_02
// hnamepath: /occupancy/cdc_occ_ring_03
// hnamepath: /occupancy/cdc_occ_ring_04
// hnamepath: /occupancy/cdc_occ_ring_05
// hnamepath: /occupancy/cdc_occ_ring_06
// hnamepath: /occupancy/cdc_occ_ring_07
// hnamepath: /occupancy/cdc_occ_ring_08
// hnamepath: /occupancy/cdc_occ_ring_09
// hnamepath: /occupancy/cdc_occ_ring_10
// hnamepath: /occupancy/cdc_occ_ring_11
// hnamepath: /occupancy/cdc_occ_ring_12
// hnamepath: /occupancy/cdc_occ_ring_13
// hnamepath: /occupancy/cdc_occ_ring_14
// hnamepath: /occupancy/cdc_occ_ring_15
// hnamepath: /occupancy/cdc_occ_ring_16
// hnamepath: /occupancy/cdc_occ_ring_17
// hnamepath: /occupancy/cdc_occ_ring_18
// hnamepath: /occupancy/cdc_occ_ring_19
// hnamepath: /occupancy/cdc_occ_ring_20
// hnamepath: /occupancy/cdc_occ_ring_21
// hnamepath: /occupancy/cdc_occ_ring_22
// hnamepath: /occupancy/cdc_occ_ring_23
// hnamepath: /occupancy/cdc_occ_ring_24
// hnamepath: /occupancy/cdc_occ_ring_25
// hnamepath: /occupancy/cdc_occ_ring_26
// hnamepath: /occupancy/cdc_occ_ring_27
// hnamepath: /occupancy/cdc_occ_ring_28
//
// e-mail: davidl@jlab.org
// e-mail: njarvis@jlab.org
// e-mail: zihlmann@jlab.org
// e-mail: tbritton@jlab.org
//
//  Guidance: --------------------------------------------
//
//  During regular runs with beam the CDC occupancies should show a smooth trend
//  from high (bright yellow) in the innermost ring to low (dark blue) in the outermost ring.
//  During cosmics runs, the whole detector is a uniform low occupancy.
// 
//  Hot channels can be caused by faulty electronics, baseline shift or noise.
//  Cold channels can be caused by poor connections, baseline shift or faulty electronics.
//
//  The channels are connected to the HV boards in clusters of 20 to 24.
//  
//  There are 2 known dead channels (wire disconnected) and a small number (4-6) 
// of channels with persistent problems. These channels are not neighbouring.
//
//  If you see single hot (bright yellow) or new dead (white or dark blue) channels, 
//  please make a logentry in HDCDC and notify the experts in the logentry.
//
//  If you see a cluster of hot or dead channels, please make the logentry  
//  and also contact the experts as below.
//
//  Daytime - contact the expert by phone using the drift chamber on call number
//  Overnight - contact the expert by phone if the cluster includes more than 24 channels.
//
//
// End Guidance: ----------------------------------------



{
	// RootSpy saves the current directory and style before
	// calling the macro and restores it after so it is OK to
	// change them and not change them back.

	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("occupancy");
	if(dir) dir->cd();

	double Nevents = 1.0;
	TH1I *cdc_num_events = (TH1I*)gDirectory->FindObjectAny("cdc_num_events");
	if(cdc_num_events) Nevents = (double)cdc_num_events->GetBinContent(1);
 	TH2D *cdc_axes = (TH2D *)gDirectory->FindObjectAny("cdc_axes");

	// Just for testing
	if(gPad == NULL){
		TCanvas *c1 = new TCanvas("c1");
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	if(!gPad) return;

	TCanvas *c1 = gPad->GetCanvas();
	if(!c1) return;
	c1->cd(0);

	// Draw axes
	double minScale = 0.0, maxScale = 0.10;
	if(cdc_axes){
	  cdc_axes->SetStats(0);
	  cdc_axes->Fill(100,100); // without this, the color ramp is not drawn
	  cdc_axes->GetZaxis()->SetRangeUser(minScale, maxScale);
	  cdc_axes->Draw("colz");
	}
	
	// Draw inner and outer circles so we can see if outer ring is missing
	TEllipse *e = new TEllipse(0.0, 0.0, 56.0, 56.0);
	e->SetLineWidth(3);
	e->Draw();
	e = new TEllipse(0.0, 0.0, 9.55, 9.55);
	e->SetLineWidth(2);
	e->Draw();

	// Draw All rings
	for(unsigned int iring=1; iring<=28; iring++){
		char hname[256];
		sprintf(hname, "cdc_occ_ring_%02d", iring);
		TH1 *h = (TH1*)(gDirectory->Get(hname));
		if(h){
			sprintf(hname, "cdc_occ_ring_norm_%02d", iring);
			TH1 *hh = (TH1*)h->Clone(hname);
			hh->Scale(1.0/Nevents);
			hh->GetZaxis()->SetRangeUser(minScale, maxScale);
			hh->SetStats(0);
			hh->Draw("same col pol");  // draw remaining histos without overwriting color palette
		}
	}
	
	char str[256];
	sprintf(str,"%g events", Nevents);
	TLatex lat;
	lat.SetTextAlign(22);
	lat.SetTextSize(0.035);
	lat.DrawLatex(0.0, 61.0, str);

#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
		auto min_events = 5*rs_GetFlag("MIN_EVENTS_RSAI");
		if( min_events < 1 ) min_events = 5E5;
		if( Nevents >= min_events ) {
			cout << "CDC Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
			rs_SavePad("CDC_occupancy", 0);
			rs_ResetAllMacroHistos("//CDC_occupancy");
		}
	}
#endif
}
