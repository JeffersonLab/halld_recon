// hnamepath: /HLDetectorTiming/Physics Triggers/CDC/CDCHit time
// hnamepath: /HLDetectorTiming/Physics Triggers/FDC/FDCHit Cathode time
// hnamepath: /HLDetectorTiming/Physics Triggers/FDC/FDCHit Wire time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/Earliest CDC Time Minus Matched BCAL Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/Earliest Flight-time Corrected CDC Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/Earliest Flight-time Corrected FDC Time
// hnamepath: /HLDetectorTiming/Physics Triggers/FDC/FDCHit Wire time vs. module
{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HLDetectorTiming");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
	TH1I* CDC_Timing     = (TH1I*)gDirectory->Get("Physics Triggers/CDC/CDCHit time");
	TH1I* CDC_BCAL_Timing  = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/Earliest CDC Time Minus Matched BCAL Time");
	TH1I* CDC_Earliest_Time  = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/Earliest Flight-time Corrected CDC Time");
	TH1I* FDC_Strip_Timing     = (TH1I*)gDirectory->Get("Physics Triggers/FDC/FDCHit Cathode time");
	TH1I* FDC_Wire_Timing     = (TH1I*)gDirectory->Get("Physics Triggers/FDC/FDCHit Wire time");
	TH1I* FDC_Earliest_Time  = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/Earliest Flight-time Corrected FDC Time");
	TH2I* FDC_Wire_Module_Time  = (TH2I*)gDirectory->Get("Physics Triggers/FDC/FDCHit Wire time vs. module");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("TrackMatchedTiming", "TrackMatchedTiming", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(3, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(CDC_Timing != NULL)
	{
        CDC_Timing->Draw();
        CDC_Timing->SetFillColor(kGray);
	}
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No CDC Times in data");
        text->Draw();
    }

    locCanvas->cd(2);
    gPad->SetTicks();
    gPad->SetGrid();
    if(CDC_Earliest_Time != NULL)
    {
        CDC_Earliest_Time->Draw();
        CDC_Earliest_Time->SetFillColor(kGray);
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No CDC tracks matced to BCAL/SC with reasonable FOM");
        text->Draw();
    }
    
    locCanvas->cd(3);
    gPad->SetTicks();
    gPad->SetGrid();
    if(CDC_BCAL_Timing != NULL)
    {
        CDC_BCAL_Timing->Draw();
        CDC_BCAL_Timing->SetFillColor(kGray);
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No CDC tracks matced to SC with reasonable FOM");
        text->Draw();
    }

    locCanvas->cd(4);
    gPad->SetTicks();
    gPad->SetGrid();
    if(FDC_Strip_Timing != NULL)
    {
        FDC_Strip_Timing->GetXaxis()->SetRangeUser(-150,350);
        FDC_Strip_Timing->Draw();
        FDC_Strip_Timing->SetFillColor(kGray);
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No FDC Cathode Hit times");
        text->Draw();
    }

    locCanvas->cd(5);
    gPad->SetTicks();
    gPad->SetGrid();
    if(FDC_Wire_Timing != NULL)
    {
        FDC_Wire_Timing->Draw();
        FDC_Wire_Timing->SetFillColor(kGray);
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No FDC Wire Hit times");
        text->Draw();
    }

    locCanvas->cd(6);
    gPad->SetTicks();
    gPad->SetGrid();
    if(FDC_Wire_Module_Time != NULL)
    {
      gPad->SetMargin(0.1,0.1,0.15,0.1);
      FDC_Wire_Module_Time->GetXaxis()->SetTitle();
      FDC_Wire_Module_Time->Draw("COLZ");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No FDC Wire Hit times");
        text->Draw();
    }

    /*
    if(FDC_Earliest_Time != NULL)
    {
        FDC_Earliest_Time->Draw();
        FDC_Earliest_Time->SetFillColor(kGray);
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No FDC tracks matced to SC/TOF with reasonable FOM");
        text->Draw();
    }
    */
}

