//
// Guidance: --------------------------------------------
//
// For the plots in the left column, the peak position should 
// be aligned with the red line to around 10 ns, and the peak 
// shape should be generally like what is seen in the reference plot. 
//
// The SC TDC-ADC distributions should be generally peaked
// around zero.  The TOF TDC-ADC alignment is done at a later point.
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
// hnamepath: /HLDetectorTiming/Physics Triggers/SC/SCHit Matched time
// hnamepath: /HLDetectorTiming/Physics Triggers/SC/SCHit TDC_ADC Difference
// hnamepath: /HLDetectorTiming/Physics Triggers/TOF/TOFHit Matched time
// hnamepath: /HLDetectorTiming/Physics Triggers/TOF/TOFHit TDC_ADC Difference
//
// e-mail: aaustreg@jlab.org
// e-mail: marki@jlab.org
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

        //Setpoints
        double nominalSCTime = -3.;
        double nominalTOFTime = 19.;

	//Get Histograms
 	TH1I* SC_Timing          = (TH1I*)gDirectory->Get("Physics Triggers/SC/SCHit Matched time");
 	TH1I* TOF_Timing         = (TH1I*)gDirectory->Get("Physics Triggers/TOF/TOFHit Matched time");
 	TH2I* SC_TDCADC_Timing   = (TH2I*)gDirectory->Get("Physics Triggers/SC/SCHit TDC_ADC Difference");
 	TH2I* TOF_TDCADC_Timing  = (TH2I*)gDirectory->Get("Physics Triggers/TOF/TOFHit TDC_ADC Difference");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("OnlinePID", "OnlinePID", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(SC_Timing != NULL)
	{
	  SC_Timing->GetXaxis()->SetRangeUser(-200,200);
	  SC_Timing->Draw();
	  SC_Timing->SetFillColor(kGray);
	  locCanvas->Update();

	  TLine *ln = new TLine(nominalSCTime, gPad->GetUymin(), nominalSCTime, gPad->GetUymax());
	  ln->SetLineColor(2);
	  ln->Draw();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No Matched SC TDC/ADC Times");
	  text->Draw();
	}

    locCanvas->cd(2);
    gPad->SetTicks();
    gPad->SetGrid();
    if(SC_TDCADC_Timing != NULL)
    {
        SC_TDCADC_Timing->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched SC TDC/ADC Times");
        text->Draw();
    }

    locCanvas->cd(3);
    gPad->SetTicks();
    gPad->SetGrid();
    if(TOF_Timing != NULL)
    {
        TOF_Timing->GetXaxis()->SetRangeUser(-200,200);
        TOF_Timing->Draw();
        TOF_Timing->SetFillColor(kGray);
	locCanvas->Update();

	TLine *ln = new TLine(nominalTOFTime, gPad->GetUymin(), nominalTOFTime, gPad->GetUymax());
	ln->SetLineColor(2);
	ln->Draw();
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched TOF TDC/ADC Times");
        text->Draw();
    }

    locCanvas->cd(4);
    gPad->SetTicks();
    gPad->SetGrid();
    if(TOF_TDCADC_Timing != NULL)
    {
        TOF_TDCADC_Timing->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No Matched TOF TDC/ADC Times");
        text->Draw();
    }
}

