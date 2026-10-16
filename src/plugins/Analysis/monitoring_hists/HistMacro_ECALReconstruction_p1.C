// hnamepath: /Independent/Hist_Reconstruction/ECALShowerYVsX
// hnamepath: /Independent/Hist_Reconstruction/ECALShowerEnergy
// hnamepath: /Independent/Hist_Neutrals/ECALNeutralShowerEnergy
// hnamepath: /Independent/Hist_Neutrals/ECALNeutralShowerDeltaT
// hnamepath: /Independent/Hist_Neutrals/ECALNeutralShowerDeltaTVsE

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("Hist_Reconstruction");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
	TH2I* locHist_ECALShowerYVsX = (TH2I*)gDirectory->Get("ECALShowerYVsX");
	TH1I* locHist_ECALShowerEnergy = (TH1I*)gDirectory->Get("ECALShowerEnergy");

	gDirectory->cd("../Hist_Neutrals");
	TH1I* locHist_ECALNeutralShowerEnergy = (TH1I*)gDirectory->Get("ECALNeutralShowerEnergy"); //photon(?)
	TH1I* locHist_ECALNeutralShowerDeltaT = (TH1I*)gDirectory->Get("ECALNeutralShowerDeltaT"); //photon(?)
	TH2I* locHist_ECALNeutralShowerDeltaTVsE = (TH2I*)gDirectory->Get("ECALNeutralShowerDeltaTVsE"); //photon(?)

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("ECALReconstruction_p1", "ECALReconstruction_p1", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(3, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_ECALShowerYVsX != NULL)
	{
		locHist_ECALShowerYVsX->GetXaxis()->SetTitleSize(0.04);
		locHist_ECALShowerYVsX->GetYaxis()->SetTitleSize(0.04);
		locHist_ECALShowerYVsX->GetXaxis()->SetLabelSize(0.04);
		locHist_ECALShowerYVsX->GetYaxis()->SetLabelSize(0.04);
		locHist_ECALShowerYVsX->Draw("COLZ");
		gPad->SetLogz();
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_ECALShowerEnergy != NULL)
	{
		locHist_ECALShowerEnergy->GetXaxis()->SetTitleSize(0.04);
		locHist_ECALShowerEnergy->GetXaxis()->SetLabelSize(0.04);
		locHist_ECALShowerEnergy->GetYaxis()->SetLabelSize(0.04);
		locHist_ECALShowerEnergy->Draw();
	}

	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_ECALNeutralShowerEnergy != NULL)
	{
		locHist_ECALNeutralShowerEnergy->GetXaxis()->SetTitleSize(0.04);
		locHist_ECALNeutralShowerEnergy->GetXaxis()->SetLabelSize(0.04);
		locHist_ECALNeutralShowerEnergy->GetYaxis()->SetLabelSize(0.04);
		locHist_ECALNeutralShowerEnergy->Draw();
	}

	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_ECALNeutralShowerDeltaT != NULL)
	{
		locHist_ECALNeutralShowerDeltaT->GetXaxis()->SetTitleSize(0.04);
		locHist_ECALNeutralShowerDeltaT->GetXaxis()->SetLabelSize(0.04);
		locHist_ECALNeutralShowerDeltaT->GetYaxis()->SetLabelSize(0.04);
		locHist_ECALNeutralShowerDeltaT->Draw();
	}

	locCanvas->cd(5);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_ECALNeutralShowerDeltaTVsE != NULL)
	{
		locHist_ECALNeutralShowerDeltaTVsE->GetXaxis()->SetTitleSize(0.04);
		locHist_ECALNeutralShowerDeltaTVsE->GetYaxis()->SetTitleSize(0.04);
		locHist_ECALNeutralShowerDeltaTVsE->GetYaxis()->SetTitleOffset(0.85);
		locHist_ECALNeutralShowerDeltaTVsE->GetXaxis()->SetLabelSize(0.04);
		locHist_ECALNeutralShowerDeltaTVsE->GetYaxis()->SetLabelSize(0.04);
		locHist_ECALNeutralShowerDeltaTVsE->Draw("COLZ");
	}
}

