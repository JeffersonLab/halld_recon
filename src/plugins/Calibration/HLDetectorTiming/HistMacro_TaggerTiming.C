// hnamepath: /HLDetectorTiming/Physics Triggers/TAGM/TAGMHit Matched time
// hnamepath: /HLDetectorTiming/Physics Triggers/TAGM/TAGMHit TDC_ADC Difference
// hnamepath: /HLDetectorTiming/Physics Triggers/TAGH/TAGHHit Matched time
// hnamepath: /HLDetectorTiming/Physics Triggers/TAGH/TAGHHit TDC_ADC Difference

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HLDetectorTiming");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
    TH1I* TAGH_Timing        = (TH1I*)gDirectory->Get("Physics Triggers/TAGH/TAGHHit Matched time");
    TH1I* TAGM_Timing        = (TH1I*)gDirectory->Get("Physics Triggers/TAGM/TAGMHit Matched time");
	TH2I* TAGH_TDCADC_Timing = (TH2I*)gDirectory->Get("Physics Triggers/TAGH/TAGHHit TDC_ADC Difference");
	TH2I* TAGM_TDCADC_Timing = (TH2I*)gDirectory->Get("Physics Triggers/TAGM/TAGMHit TDC_ADC Difference");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("TaggerTiming", "TaggerTiming", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(TAGH_Timing != NULL)
	{
        TAGH_Timing->Draw();
        TAGH_Timing->SetFillColor(kGray);
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched TAGH TDC/ADC Times");
        text->Draw();
    }

    locCanvas->cd(2);
    gPad->SetTicks();
    gPad->SetGrid();
    if(TAGH_TDCADC_Timing != NULL)
    {
        TAGH_TDCADC_Timing->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched TAGH TDC/ADC Times");
        text->Draw();
    }

    locCanvas->cd(3);
    gPad->SetTicks();
    gPad->SetGrid();
    if(TAGM_Timing != NULL)
    {
        TAGM_Timing->Draw();
        TAGM_Timing->SetFillColor(kGray);
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched TAGM TDC/ADC Times");
        text->Draw();
    }

    locCanvas->cd(4);
    gPad->SetTicks();
    gPad->SetGrid();
    if(TAGM_TDCADC_Timing != NULL)
    {
        TAGM_TDCADC_Timing->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched TAGM TDC/ADC Times");
        text->Draw();
    }
}

