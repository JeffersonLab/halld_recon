//
// Guidance: --------------------------------------------
//
// These peaks should be centered at zero, with shapes
// similar to that seen in the reference plot.
// Note that the tagger "picket fence" plots can take
// awhile to accumulate enough data to acquire the
// expected shape.
//
// Sometimes, common shifts of 2 ns are seen in all detectors.
// These are due to a change in the master RF time due
// to a rebooted crate, and will be calibrated away
// by standard processes.
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
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/TAGM - RFBunch 1D Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/Tagger - RFBunch 1D Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/SC - RF Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/FCAL - RF Time (Neutral)
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/ECAL - RF Time (Neutral)
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/TOF - RF Time
// hnamepath: /HLDetectorTiming/Physics Triggers/TRACKING/BCAL - RF Time  (Neutral)
//
// e-mail: aaustreg@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org
//

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HLDetectorTiming");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
	TH1I* TAGM_RF_Timing     = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/TAGM - RFBunch 1D Time");
	TH1I* Tagger_RF_Timing     = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/Tagger - RFBunch 1D Time");
	TH1I* SC_RF_Timing  = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/SC - RF Time");
// 	TH1I* FCAL_RF_Timing = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/FCAL - RF Time");
// 	TH1I* ECAL_RF_Timing = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/ECAL - RF Time");
//	TH1I* BCAL_RF_Timing = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/BCAL - RF Time");
	TH1I* FCAL_RF_Timing = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/FCAL - RF Time (Neutral)");
	TH1I* ECAL_RF_Timing = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/ECAL - RF Time (Neutral)");
	TH1I* BCAL_RF_Timing = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/BCAL - RF Time  (Neutral)");
	TH1I* TOF_RF_Timing  = (TH1I*)gDirectory->Get("Physics Triggers/TRACKING/TOF - RF Time");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("OnlineRF", "OnlineRF", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(3, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(SC_RF_Timing != NULL)
	{
	    SC_RF_Timing->Draw();
	    SC_RF_Timing->SetFillColor(kGray);
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No CDC tracks matched to SC with reasonable FOM");
	  text->Draw();
	}

	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	if(TOF_RF_Timing != NULL)
	{
	  TOF_RF_Timing->Draw();
	  TOF_RF_Timing->SetFillColor(kGray);
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No track matches to TOF with reasonable FOM");
	  text->Draw();
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(BCAL_RF_Timing != NULL)
	{
	  BCAL_RF_Timing->Draw();
	  BCAL_RF_Timing->SetFillColor(kGray);
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No track matches to BCAL with reasonable FOM");
	  text->Draw();
	}

	locCanvas->cd(5);
	gPad->SetTicks();
	gPad->SetGrid();
	if(FCAL_RF_Timing != NULL)
	{
	  FCAL_RF_Timing->Draw();
	  FCAL_RF_Timing->SetFillColor(kGray);
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No track matches to FCAL with reasonable FOM");
	  text->Draw();
	}

	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	if(Tagger_RF_Timing != NULL)
	{
	  Tagger_RF_Timing->Draw();
	  Tagger_RF_Timing->SetFillColor(kGray);
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No Tagger matches with reasonable FOM");
	  text->Draw();
	}
	
	locCanvas->cd(6);
	gPad->SetTicks();
	gPad->SetGrid();
	if(ECAL_RF_Timing != NULL)
	{
	  ECAL_RF_Timing->Draw();
	  ECAL_RF_Timing->SetFillColor(kGray);
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No Matches to ECAL with reasonable FOM");
	  text->Draw();
	}
}

