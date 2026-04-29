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
	TH2D *axes = new TH2D("cdc_axes", "CDC Occupancy", 100, -57.0*4.0/3.0, 57.0*4.0/3.0, 100, -57.0, 57.0);

	double minScale = 0.0, maxScale = 0.1;
	axes->SetStats(0);
	axes->Fill(100,100); // without this, the color ramp is not drawn
	axes->GetZaxis()->SetRangeUser(minScale, maxScale);
	axes->GetXaxis()->SetNdivisions(0);
	axes->GetYaxis()->SetNdivisions(0);	
	axes->Draw("colz");

	// Draw inner and outer circles so we can see if outer ring is missing
	TEllipse *e = new TEllipse(0.0, 0.0, 56.0, 56.0);
	e->SetLineWidth(3);
	e->Draw();
	e = new TEllipse(0.0, 0.0, 9.55, 9.55);
	e->SetLineWidth(2);
	e->Draw();
	
	for(unsigned int iring=1; iring<=28; iring++){
		char hname[256];
		sprintf(hname, "cdc_occ_ring[%d]", iring);
		TH1 *h = (TH1*)(dir->Get(hname));
		if(h){
			sprintf(hname, "cdc_occ_ring_norm[%d]", iring);
			TH1 *hh = (TH1*)h->Clone(hname);
			hh->Scale(1.0/Nevents);
			hh->GetZaxis()->SetRangeUser(minScale, maxScale);
			hh->SetStats(0);
			hh->Draw("same col pol");  // draw remaining histos without overwriting color palette
		}
	}
}
