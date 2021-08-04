// hnamepath: /PSC/Hit/LeftArm/Hit_tdcadcTimeDiffVsModule_LeftArm
// hnamepath: /PSC/Hit/RightArm/Hit_tdcadcTimeDiffVsModule_RightArm

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("PSC");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
	TH1I* PS_Left_TimeVsCol  = (TH1I*)gDirectory->Get("Hit/LeftArm/Hit_tdcadcTimeDiffVsModule_LeftArm");
	TH1I* PS_Right_TimeVsCol = (TH1I*)gDirectory->Get("Hit/RightArm/Hit_tdcadcTimeDiffVsModule_RightArm");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("PSCTiming", "PSCTiming", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(1, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	//gPad->SetGrid();
	if(PS_Left_TimeVsCol != NULL)
	{
	  	PS_Left_TimeVsCol->GetYaxis()->SetRangeUser(-20, 20.);
	    PS_Left_TimeVsCol->Draw("COLZ");
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No PSC Times");
	  text->Draw();
	}

    locCanvas->cd(2);
    gPad->SetTicks();
    gPad->SetGrid();
	if(PS_Right_TimeVsCol != NULL)
	{
	  	PS_Right_TimeVsCol->GetYaxis()->SetRangeUser(-20, 20.);
	    PS_Right_TimeVsCol->Draw("COLZ");
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No PSC Times");
	  text->Draw();
	}

}

