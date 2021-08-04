// hnamepath: /PS/Hit/LeftArm/Hit_TimeVsColumn_LeftArm
// hnamepath: /PS/Hit/RightArm/Hit_TimeVsColumn_RightArm

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("PS");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
	TH1I* PS_Left_TimeVsCol  = (TH1I*)gDirectory->Get("Hit/LeftArm/Hit_TimeVsColumn_LeftArm");
	TH1I* PS_Right_TimeVsCol = (TH1I*)gDirectory->Get("Hit/RightArm/Hit_TimeVsColumn_RightArm");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("PSTiming", "PSTiming", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(1, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	//gPad->SetGrid();
	if(PS_Left_TimeVsCol != NULL)
	{
	  	PS_Left_TimeVsCol->GetYaxis()->SetRangeUser(-80, 0.);
	    PS_Left_TimeVsCol->Draw("COLZ");
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No PS Times");
	  text->Draw();
	}

    locCanvas->cd(2);
    gPad->SetTicks();
    gPad->SetGrid();
	if(PS_Right_TimeVsCol != NULL)
	{
	  	PS_Right_TimeVsCol->GetYaxis()->SetRangeUser(-80, 0.);
	    PS_Right_TimeVsCol->Draw("COLZ");
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No PS Times");
	  text->Draw();
	}

}

