// hnamepath: /HELI/BeamHelicity

{
	cout << "Entered macro: HELI_online/HistMacro_BeamHelicity.C" << endl;

	TDirectory *locTopDirectory = gDirectory;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HELI");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Beam Helicity Histograms
	TH1F* locHist_BeamHelicity = (TH1F*)locDirectory->Get("BeamHelicity");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("HELI", "HELI", 600, 400); //for testing
	else
		locCanvas = gPad->GetCanvas();

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_BeamHelicity != NULL)
	{
		locHist_BeamHelicity->GetXaxis()->SetTitleSize(0.05);
		locHist_BeamHelicity->GetYaxis()->SetTitleSize(0.05);
		locHist_BeamHelicity->GetXaxis()->SetLabelSize(0.05);
		locHist_BeamHelicity->GetYaxis()->SetLabelSize(0.05);
		locHist_BeamHelicity->SetMinimum(0);
		locHist_BeamHelicity->Draw();
	}

	cout << "end main part of macro: HELI_online/HistMacro_BeamHelicity.C" << endl;
}
