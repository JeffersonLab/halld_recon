//
// Guidance: --------------------------------------------
//
/// These peaks should be centered at zero, with shapes
// similar to that seen in the reference plot.
// 
//
// If you have concerns about the values in the table or
// any of the plots looking significantly different than
// the reference, please contact one of:
//
//  Run Coordinator:  (757) 383-5542
//
//  Sean Dobbs:       (703) 887-1776 (cell)
//                    (850) 645-3812 (office)
//
// End Guidance: ----------------------------------------
//
// hnamepath: /HLDetectorTiming/Physics Triggers/CDC/CDCHit time
// hnamepath: /HLDetectorTiming/Physics Triggers/FDC/FDCHit Cathode time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/Earliest Flight-time Corrected FDC Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/Earliest CDC Time Minus Matched SC Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/FCAL - SC Target Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/TOF - SC Target Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/BCAL - SC Target Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/SC - RF Time vs. Sector
//
// e-mail: aaustreg@jlab.org
// e-mail: dalton@jlab.org
// e-mail: gleasonc@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org
// e-mail: zihlmann@jlab.org
//

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HLDetectorTiming");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
    TH1I* CDC_Timing     = (TH1I*)gDirectory->Get("Physics Triggers/CDC/CDCHit time");
    TH1I* FDC_Timing     = (TH1I*)gDirectory->Get("Physics Triggers/FDC/FDCHit Cathode time");
    TH1I* SC_RF_Timing   = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/SC - RF Time vs. Sector");
    TH1I* FDC_SC_Timing  = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/Earliest Flight-time Corrected FDC Time");
    TH1I* CDC_SC_Timing  = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/Earliest CDC Time Minus Matched SC Time");
    TH1I* FCAL_SC_Timing = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/FCAL - SC Target Time");
    TH1I* TOF_SC_Timing  = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/TOF - SC Target Time");
    TH1I* BCAL_SC_Timing = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/BCAL - SC Target Time");

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
	if(TOF_SC_Timing != NULL)
	{
	    TOF_SC_Timing->Draw();
	    TOF_SC_Timing->SetFillColor(kGray);
	}
	else{
	    TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	    text->AddText("No Matches to SC and TOF with reasonable FOM");
	    text->Draw();
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(BCAL_SC_Timing != NULL)
	{
	    BCAL_SC_Timing->Draw();
	    BCAL_SC_Timing->SetFillColor(kGray);
	}
	else{
	    TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	    text->AddText("No Matches to SC and BCAL with reasonable FOM");
	    text->Draw();
	}

	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	if(FCAL_SC_Timing != NULL)
	{
	    FCAL_SC_Timing->Draw();
	    FCAL_SC_Timing->SetFillColor(kGray);
	}
	else{
	    TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	    text->AddText("No Matches to SC and FCAL with reasonable FOM");
	    text->Draw();
	}

	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	if(SC_RF_Timing != NULL)
	{
	    SC_RF_Timing->Draw("COLZ");
	}
	else{
	    TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	    text->AddText("No SC Times matched to RF times in data");
	    text->Draw();
	}

	locCanvas->cd(5);
	gPad->SetTicks();
	gPad->SetGrid();
	if(CDC_SC_Timing != NULL)
	{
	    CDC_SC_Timing->Draw();
	    CDC_SC_Timing->SetFillColor(kGray);
	}
	else{
	    TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	    text->AddText("No CDC tracks matced to SC with reasonable FOM");
	    text->Draw();
	}

	locCanvas->cd(6);
	gPad->SetTicks();
	gPad->SetGrid();
	if(FDC_SC_Timing != NULL)
	{
	    FDC_SC_Timing->Draw();
	    FDC_SC_Timing->SetFillColor(kGray);
	}
	else{
	    TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	    text->AddText("No FDC tracks matced to SC with reasonable FOM");
	    text->Draw();
	}

}

