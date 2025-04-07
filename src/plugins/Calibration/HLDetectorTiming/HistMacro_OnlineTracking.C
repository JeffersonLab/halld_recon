//
// Guidance: --------------------------------------------
//
// The peaks in the top row should be aligned with the red
// line within 10 ns or so.  
//
// The peaks on the 2D plots should be aligned with each
// other, at a value slightly larger than zero.
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
// hnamepath: /HLDetectorTiming/Physics Triggers/CDC/CDCHit time per Straw Raw
// hnamepath: /HLDetectorTiming/Physics Triggers/FDC/FDCHit Cathode time
// hnamepath: /HLDetectorTiming/Physics Triggers/FDC/FDCHit Wire time
// hnamepath: /HLDetectorTiming/Physics Triggers/FDC/FDCHit Wire time vs. module

// e-mail: aaustreg@jlab.org
// e-mail: pentchev@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HLDetectorTiming");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Setpoints
	double nominalCDCTime = 0.;
	double nominalFDCCathodeTime = 8.;
	double nominalFDCWireTime = 8.;

	//Get Histograms
 	TH1I* CDCHit_Timing    = (TH1I*) gDirectory->Get("Physics Triggers/CDC/CDCHit time");
	TH2I* CDCHit_vs_module_Timing  = (TH2I*)gDirectory->Get("Physics Triggers/CDC/CDCHit time per Straw Raw");
 	TH1I* FDCHit_Cathode_Timing    = (TH1I*)gDirectory->Get("Physics Triggers/FDC/FDCHit Cathode time");
 	TH1I* FDCHit_Wire_Timing = (TH1I*)gDirectory->Get("Physics Triggers/FDC/FDCHit Wire time");
 	TH2I* FDCHit_Wire_vs_module_Timing  = (TH2I*)gDirectory->Get("Physics Triggers/FDC/FDCHit Wire time vs. module");

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
	if(CDCHit_Timing != NULL)
	{
	    CDCHit_Timing->GetXaxis()->SetRangeUser(-300,1100);
	    CDCHit_Timing->Draw();
	    CDCHit_Timing->SetFillColor(kGray);
	    locCanvas->Update();

	    TLine *ln = new TLine(nominalCDCTime, gPad->GetUymin(), nominalCDCTime, gPad->GetUymax());
	    ln->SetLineColor(2);
	    ln->Draw();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No CDC hits!");
	  text->Draw();
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(FDCHit_Cathode_Timing != NULL)
	{
	    FDCHit_Cathode_Timing->GetXaxis()->SetRangeUser(-100,300);
	    FDCHit_Cathode_Timing->Draw();
	    FDCHit_Cathode_Timing->SetFillColor(kGray);
	    locCanvas->Update();

	    TLine *ln = new TLine(nominalFDCCathodeTime, gPad->GetUymin(), nominalFDCCathodeTime, gPad->GetUymax());
	    ln->SetLineColor(2);
	    ln->Draw();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No FDC Cathode hits!");
	  text->Draw();
	}

	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	if(FDCHit_Wire_Timing != NULL)
	{
	    FDCHit_Wire_Timing->GetXaxis()->SetRangeUser(-300,500);
	    FDCHit_Wire_Timing->Draw();
	    FDCHit_Wire_Timing->SetFillColor(kGray);
	    locCanvas->Update();

	    TLine *ln = new TLine(nominalFDCWireTime, gPad->GetUymin(), nominalFDCWireTime, gPad->GetUymax());
	    ln->SetLineColor(2);
	    ln->Draw();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No FDC Wire hits!");
	  text->Draw();
	}

	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	if(CDCHit_vs_module_Timing != NULL)
	{
	    CDCHit_vs_module_Timing->Draw("COLZ");
	    CDCHit_vs_module_Timing->SetFillColor(kGray);
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No CDC hits!");
	  text->Draw();
	}

	locCanvas->cd(6);
	gPad->SetTicks();
	gPad->SetGrid();
	if(FDCHit_Wire_vs_module_Timing != NULL)
	{
	    FDCHit_Wire_vs_module_Timing->Draw("COLZ");
	    FDCHit_Wire_vs_module_Timing->SetFillColor(kGray);
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No FDC Wire hits!");
	  text->Draw();
	}


}

