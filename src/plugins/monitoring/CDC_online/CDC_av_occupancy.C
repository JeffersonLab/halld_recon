
{

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

    	gStyle->SetPalette(kGreyYellow);  // 2nd choice kLake

	int ncontours = 10;
	double contours[] = {0.5, 0.6, 0.7, 0.8, 0.9, 1.1, 1.2, 1.3, 1.4, 1.5};
	
	// Draw axes
	TH2D *axes = new TH2D("cdc_axes", "CDC Occupancy", 100, -57.0*4.0/3.0, 57.0*4.0/3.0, 100, -57.0, 57.0);

	double minScale = contours[0];
	double maxScale = contours[ncontours-1];

	axes->SetStats(0);
	axes->Fill(100,100); // without this, the color ramp is not drawn
	axes->GetZaxis()->SetRangeUser(minScale, maxScale);
	axes->SetContour(ncontours, contours);
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
		TH2F *h = (TH2F*)(dir->Get(hname));
		if(h){
			TH2F *hh = (TH2F*)h->Clone(hname);
			double nentries = hh->GetEntries();

			if (!nentries) continue; //don't draw the ring if no hits

			int nbins = hh->GetNbinsX();
		        double mean_counts = nentries/double(nbins);
			
			for (unsigned int j=1; j<= nbins; j++) hh->SetBinContent(j, 1, hh->GetBinContent(j,1)/mean_counts);

			hh->SetStats(0);
			hh->SetLineWidth(1);
			hh->SetLineColor(kGray);
			hh->SetContour(ncontours, contours);
			
			hh->Draw("same col pol");  // draw remaining histos without overwriting color palette

		}
		
	}
}
