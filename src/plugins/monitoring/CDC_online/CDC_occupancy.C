
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /CDC/cdc_num_events
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[1]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[2]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[3]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[4]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[5]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[6]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[7]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[8]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[9]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[10]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[11]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[12]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[13]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[14]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[15]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[16]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[17]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[18]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[19]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[20]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[21]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[22]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[23]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[24]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[25]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[26]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[27]
// hnamepath: /CDC/rings_occupancy/cdc_occ_ring[28]

// e-mail: zihlmann@jlab.org
// e-mail: njarvis@jlab.org
//
// Guidance: --------------------------------------------
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
//  If you see a cluster of hot or dead channels, please make the log entry 
//  and also contact the experts using the drift chamber on call phone number.  
//
//  Daytime - contact the expert
//  Overnight - contact the expert if the cluster includes more than 24 channels.
//
// End Guidance: ----------------------------------------


{
	// Get number of events
	double Nevents = 1.0;
	TDirectory *CDCdir = (TDirectory*)gDirectory->FindObjectAny("CDC");
	if(CDCdir){
		TH1I *cdc_num_events = (TH1I*)CDCdir->Get("cdc_num_events");
		if(cdc_num_events) Nevents = (double)cdc_num_events->GetBinContent(1);
	}

	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("rings_occupancy");
	if(!dir) return;
	
	dir->cd();

	// Just for testing
	if(gPad == NULL){
		TCanvas *c1 = new TCanvas("c1");
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	
	if(!gPad) return;

	TCanvas *c1 = gPad->GetCanvas();

	// Draw axes
	TH2D *axes = (TH2D *)dir->Get("axes");
	if(!axes) axes = new TH2D("axes", "CDC Occupancy", 100, -65.0, 65.0, 100, -65.0, 65.0);

	double minScale = 0.08, maxScale = 0.15;
	axes->SetStats(0);
	axes->Fill(100,100); // without this, the color ramp is not drawn
	axes->GetZaxis()->SetRangeUser(minScale, maxScale);
	axes->Draw("colz");

	double TheMax = 0.;
	double TheMin = 99999999;;
	for(unsigned int iring=1; iring<=28; iring++){
		char hname[256];
		sprintf(hname, "cdc_occ_ring[%d]", iring);
		TH1 *h = (TH1*)(dir->Get(hname));
		if(h){
		  double min = h->GetMinimum();
		  if (min<TheMin){
		    TheMin = min;
		  }
		  double max = h->GetMaximum();
		  if (max>TheMax){
		    TheMax = max;
		  }
		}
	}
	for(unsigned int iring=1; iring<=28; iring++){
		char hname[256];
		sprintf(hname, "cdc_occ_ring[%d]", iring);
		TH1 *h = (TH1*)(dir->Get(hname));
		if(h){
			sprintf(hname, "cdc_occ_ring_norm[%d]", iring);
			TH1 *hh = (TH1*)h->Clone(hname);
			hh->Scale(1.0/Nevents);
			//hh->GetZaxis()->SetRangeUser(minScale, maxScale);
			hh->GetZaxis()->SetRangeUser(TheMin*0.95, TheMax*1.05);
			hh->SetStats(0);
			hh->Draw("same col pol");  // draw remaining histos without overwriting color palette
		}
		gPad->SetGrid();
	}
}
