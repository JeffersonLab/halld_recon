//
// Guidance: --------------------------------------------
//
// All plots should have a band aligned at zero, and it
// should be the strongest band, though it can take 
// a large amount of data for this to be clear.
//
// For the TAGM 2D plot, there are four apparent gaps in 
// the distribution, they correspond to the columns which 
// are individually read out, and can be ignored.  Sometimes
// the main peak for a channel is in a different band than
// the main band, this is corrected for offline.
//
// If you have concerns about the values in the table or
// any of the plots looking significantly different than
// the reference, please contact one of:
//
//
//  Run Coordinator:  (757) 383-5542
//
//  Sean Dobbs:       (703) 887-1776 (cell)
//                    (850) 645-3812 (office)
//
// End Guidance: ----------------------------------------
//
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/TAGM - RFBunch Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/TAGH - RFBunch Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/TAGM - RFBunch 1D Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/Tagger - RFBunch 1D Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/SC - RF Time vs. Sector

// e-mail: aaustreg@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HLDetectorTiming");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
    TH1I* Tagger_RF_1D = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/Tagger - RFBunch 1D Time");
    TH2I* TAGM_RF_1D = (TH2I*)gDirectory->Get("Physics Triggers/TRACKING/TAGM - RFBunch 1D Time");
    //TH2I* TAGH_RF_2D   = (TH2I*)gDirectory->Get("TRACKING/TAGH - RFBunch Time");
    TH2I* TAGH_RF_2D   = (TH2I*)gDirectory->Get("Physics Triggers/TRACKING/SC - RF Time vs. Sector");
    TH2I* TAGM_RF_2D   = (TH2I*)gDirectory->Get("Physics Triggers/TRACKING/TAGM - RFBunch Time");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("TaggerRFAlignment", "TaggerRFAlignment", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(TAGH_RF_2D != NULL)
	{
        TAGH_RF_2D->Draw("colz");
	}
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched TAGH/RFBunch Times");
        text->Draw();
    }

    locCanvas->cd(2);
    gPad->SetTicks();
    gPad->SetGrid();
    if(TAGM_RF_1D != NULL)
    {
        TAGM_RF_1D->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched TAGM/RFBunch Times");
        text->Draw();
    }

    locCanvas->cd(3);
    gPad->SetTicks();
    gPad->SetGrid();
    if(TAGM_RF_2D != NULL)
    {
        TAGM_RF_2D->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched TAGM/RFBunch Times");
        text->Draw();
    }

    locCanvas->cd(4);
    gPad->SetTicks();
    gPad->SetGrid();
    if(Tagger_RF_1D != NULL)
    {
        Tagger_RF_1D->Draw();
        Tagger_RF_1D->SetFillColor(kGray);
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched Tagger/RFBunch Times");
        text->Draw();
    }
}

