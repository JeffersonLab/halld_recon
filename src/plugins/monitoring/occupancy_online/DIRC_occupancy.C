
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /occupancy/dirc_num_events
// hnamepath: /occupancy/dirc_tdc_pixel_N_occ
// hnamepath: /occupancy/dirc_tdc_pixel_S_occ
//
// e-mail: davidl@jlab.org
// e-mail: marki@jlab.org
// e-mail: tbritton@jlab.org
//

{
	// RootSpy saves the current directory and style before
	// calling the macro and restores it after so it is OK to
	// change them and not change them back.
        TDirectory *savedir = gDirectory;
	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("occupancy");
	if(dir) dir->cd();

	TH2I *dirc_tdc_pixel_N_occ_led = (TH2I*)gDirectory->FindObjectAny("dirc_tdc_pixel_N_occ_led");
	TH2I *dirc_tdc_pixel_S_occ_led = (TH2I*)gDirectory->FindObjectAny("dirc_tdc_pixel_S_occ_led");
	TH2I *dirc_tdc_pixel_N_occ = (TH2I*)gDirectory->FindObjectAny("dirc_tdc_pixel_N_occ");
        TH2I *dirc_tdc_pixel_S_occ = (TH2I*)gDirectory->FindObjectAny("dirc_tdc_pixel_S_occ");

	// Just for testing
	if(gPad == NULL){
		TCanvas *c1 = new TCanvas("c1","DIRC Occupancy",600,400);
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	if(!gPad) {savedir->cd(); return;}

	TCanvas *c1 = gPad->GetCanvas();
	c1->cd(0);
	c1->Clear();

	TPad *p1 = new TPad("p1","p1",0.,0.5,1.,1.);
	p1->Draw();
	p1->cd();
	gStyle->SetOptStat(0);
	if(dirc_tdc_pixel_S_occ_led) dirc_tdc_pixel_S_occ_led->DrawCopy("colz");

	c1->cd(0);
	TPad *p2 = new TPad("p2","p2",0.,0.,1.,0.5);
	p2->Draw();
	p2->cd();
	gStyle->SetOptStat(0);
	if(dirc_tdc_pixel_S_occ) dirc_tdc_pixel_S_occ->DrawCopy("colz");
}
