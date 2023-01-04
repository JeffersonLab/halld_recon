//
// Guidance: --------------------------------------------
//
// The peaks should be generally aligned with the red line.
// The signal/background level depends on the electron beam
// current.
//
// The TAGH TDC-ADC distribution should be generally centered
// around zero, though it may be a little ratty.  Sometimes
// a shift of 32 ns is seen, this will be taken care of by
// standard calibrations.
//
// The TAGM TDC-ADC distribution should be centered around
// zero.  There are four apparent gaps in the distribution,
// they correspond to the columns which are individually 
// read out, and can be ignored.
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
// hnamepath: /HLDetectorTiming/Physics Triggers/TAGM/TAGMHit Matched time
// hnamepath: /HLDetectorTiming/Physics Triggers/TAGM/TAGMHit TDC_ADC Difference
// hnamepath: /HLDetectorTiming/Physics Triggers/TAGH/TAGHHit Matched time
// hnamepath: /HLDetectorTiming/Physics Triggers/TAGH/TAGHHit TDC_ADC Difference

// e-mail: aaustreg@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org
// e-mail: jonesrt@jlab.org

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HLDetectorTiming");
	if(!locDirectory)
		return;
	locDirectory->cd();

        //Setpoints
        double nominalTAGHTime = 18.;
        double nominalTAGMTime = 18.;	

	//Get Histograms
	TH1I* TAGH_Timing        = (TH1I*)gDirectory->Get("Physics Triggers/TAGH/TAGHHit Matched time");
	TH1I* TAGM_Timing        = (TH1I*)gDirectory->Get("Physics Triggers/TAGM/TAGMHit Matched time");
	TH2I* TAGH_TDCADC_Timing = (TH2I*)gDirectory->Get("Physics Triggers/TAGH/TAGHHit TDC_ADC Difference");
	TH2I* TAGM_TDCADC_Timing = (TH2I*)gDirectory->Get("Physics Triggers/TAGM/TAGMHit TDC_ADC Difference");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("OnlineTagger", "OnlineTagger", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(TAGH_Timing != NULL)
	{
	  TAGH_Timing->GetXaxis()->SetRangeUser(-200,200);
	  TAGH_Timing->Draw();
	  TAGH_Timing->SetFillColor(kGray);
	  locCanvas->Update();

	  TLine *ln = new TLine(nominalTAGHTime, gPad->GetUymin(), nominalTAGHTime, gPad->GetUymax());
	  ln->SetLineColor(2);
	  ln->Draw();
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
	  TAGM_Timing->GetXaxis()->SetRangeUser(-200,200);
          TAGM_Timing->Draw();
	  TAGM_Timing->SetFillColor(kGray);
	  locCanvas->Update();

	  TLine *ln = new TLine(nominalTAGMTime, gPad->GetUymin(), nominalTAGMTime, gPad->GetUymax());
	  ln->SetLineColor(2);
	  ln->Draw();
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

