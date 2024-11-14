// hnamepath: /FCAL_cpp/h_fcal_nclust
// hnamepath: /FCAL_cpp/h_fcal_nhitonly
// hnamepath: /FCAL_cpp/h_fcal_hcr
// hnamepath: /FCAL_cpp/locProfile_FCALTimeShiftMap

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("FCAL_cpp");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
	TH2F* locHist_FCALClusterOccupancy = (TH2F*)gDirectory->Get("h_fcal_nclust");
	TH2F* locHist_FCALMissedTriggerHit = (TH2F*)gDirectory->Get("h_fcal_nhitonly");
	TH2F* locHist_FCALNoiseHitOccupancy = (TH2F*)gDirectory->Get("h_fcal_hcr");
	TProfile2D* locProfile_FCALTimeShiftMap = (TProfile2D*)gDirectory->Get("h_fcalshower_tall_map");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("FCAL_Cpp", "FCAL_Cpp", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_FCALClusterOccupancy != NULL)
	{
		locHist_FCALClusterOccupancy->GetXaxis()->SetTitleSize(0.04);
		locHist_FCALClusterOccupancy->GetYaxis()->SetTitleSize(0.04);
		locHist_FCALClusterOccupancy->GetXaxis()->SetLabelSize(0.04);
		locHist_FCALClusterOccupancy->GetYaxis()->SetLabelSize(0.04);
		locHist_FCALClusterOccupancy->GetXaxis()->SetTitle("FCAL Column");
		locHist_FCALClusterOccupancy->GetYaxis()->SetTitle("FCAL Row");
		locHist_FCALClusterOccupancy->SetStats(0);
		locHist_FCALClusterOccupancy->Draw("COLZ");
		gPad->SetLogz();
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_FCALMissedTriggerHit != NULL)
	{
		locHist_FCALMissedTriggerHit->GetXaxis()->SetTitleSize(0.04);
		locHist_FCALMissedTriggerHit->GetYaxis()->SetTitleSize(0.04);
		locHist_FCALMissedTriggerHit->GetXaxis()->SetLabelSize(0.04);
		locHist_FCALMissedTriggerHit->GetYaxis()->SetLabelSize(0.04);
		locHist_FCALMissedTriggerHit->GetXaxis()->SetTitle("FCAL Column");
		locHist_FCALMissedTriggerHit->GetYaxis()->SetTitle("FCAL Row");
		locHist_FCALMissedTriggerHit->SetStats(0);
		locHist_FCALMissedTriggerHit->Draw("COLZ");
//	gPad->SetLogz();
	}

	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_FCALNoiseHitOccupancy != NULL)
	{
		locHist_FCALNoiseHitOccupancy->GetXaxis()->SetTitleSize(0.04);
		locHist_FCALNoiseHitOccupancy->GetYaxis()->SetTitleSize(0.04);
		locHist_FCALNoiseHitOccupancy->GetXaxis()->SetLabelSize(0.04);
		locHist_FCALNoiseHitOccupancy->GetYaxis()->SetLabelSize(0.04);
		locHist_FCALNoiseHitOccupancy->GetXaxis()->SetTitle("FCAL Column");
		locHist_FCALNoiseHitOccupancy->GetYaxis()->SetTitle("FCAL Row");
		locHist_FCALNoiseHitOccupancy->SetStats(0);
		locHist_FCALNoiseHitOccupancy->Draw("COLZ");
	}

	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locProfile_FCALTimeShiftMap != NULL)
	{
		locProfile_FCALTimeShiftMap->GetXaxis()->SetTitleSize(0.04);
		locProfile_FCALTimeShiftMap->GetYaxis()->SetTitleSize(0.04);
		locProfile_FCALTimeShiftMap->GetXaxis()->SetLabelSize(0.04);
		locProfile_FCALTimeShiftMap->GetYaxis()->SetLabelSize(0.04);
		locProfile_FCALTimeShiftMap->GetXaxis()->SetTitle("FCAL Column");
		locProfile_FCALTimeShiftMap->GetYaxis()->SetTitle("FCAL Row");
		locProfile_FCALTimeShiftMap->GetZaxis()->SetRangeUser(-10,10);
		locProfile_FCALTimeShiftMap->SetStats(0);
		locProfile_FCALTimeShiftMap->Draw("COLZ");
	}

}

