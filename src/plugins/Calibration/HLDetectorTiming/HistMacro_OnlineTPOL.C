// hnamepath: /HLDetectorTiming/TPOL/TPOL_time
// hnamepath: /HLDetectorTiming/TPOL/TPOL_time_per_sector

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HLDetectorTiming");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Setpoints
	double nominalTPOLTime = 10.;

	//Get Histograms
	TH1I* TPOLHit_Timing    = (TH1I*)gDirectory->Get("TPOL/TPOL_time");
	TH2I* TPOLHit_Timing_vs_sector  = (TH2I*)gDirectory->Get("TPOL/TPOL_time_per_sector");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("TPOLTiming", "TPOLTiming", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(1, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(TPOLHit_Timing != NULL)
	{
	    TPOLHit_Timing->GetXaxis()->SetRangeUser(0.0,700.0);
	    TPOLHit_Timing->Draw();
	    TPOLHit_Timing->SetFillColor(kGray);
	    locCanvas->Update();

	    //TLine *ln = new TLine(nominalCDCTime, gPad->GetUymin(), nominalCDCTime, gPad->GetUymax());
	    //ln->SetLineColor(2);
	    //ln->Draw();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No TPOL hits!");
	  text->Draw();
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(TPOLHit_Timing_vs_sector != NULL)
	{
	    TPOLHit_Timing_vs_sector->GetXaxis()->SetRangeUser(0.5,32.5);
	    TPOLHit_Timing_vs_sector->GetYaxis()->SetRangeUser(0.0,700.0);
	    TPOLHit_Timing_vs_sector->Draw("COLZ1");
	    locCanvas->Update();

	    //TLine *ln = new TLine(nominalFDCCathodeTime, gPad->GetUymin(), nominalFDCCathodeTime, gPad->GetUymax());
	    //ln->Draw();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No TPOL hits!");
	  text->Draw();
	}

}

