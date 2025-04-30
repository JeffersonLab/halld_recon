//
// NOTE: The L1GTPRate histo now comes from the 
// occupancy_online plugin since it should see many
// more sync events.
//
// Guidance: --------------------------------------------
//
// This page includes a mixture of low-level and high-level
// information. Values in the table regarding numbers of
// triggers can be highly sensitive to calibration. They 
// also may be slightly different between Amporphous and
// diamond radiators. If the values differ by more than
// around 20% from the reference, try hitting the "Reset"
// button to see if they improve with new statistics.
//
// If you have concerns about the values in the table or
// any of the plots looking significantly different than
// the reference, please contact one of:
//
//  Run Coordinator:  (757) 383-5542
//
//  David Lawrence:   x5567    (office)
//                    746-6697 (cell)
//                    595-1302 (home)
//
// End Guidance: ----------------------------------------
//
// hnamepath: /highlevel/BCALVsFCAL2_TrigBit1
// hnamepath: /highlevel/BCALVsFCAL_TrigBit1
// hnamepath: /highlevel/BCALVsECAL_TrigBit1
// hnamepath: /highlevel/ECALVsFCAL_TrigBit1
//
// e-mail: davidl@jlab.org
// e-mail: staylor@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org
//

{
	TDirectory *locTopDirectory = gDirectory;


	// Grab remaining histos from highlevel directory
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(!locDirectory)
		return;
	locDirectory->cd();

	TH2* locHist_BCALVsFCAL2_TrigBit1 = (TH2*)gDirectory->Get("BCALVsFCAL2_TrigBit1");
	TH2* locHist_BCALVsFCAL_TrigBit1 = (TH2*)gDirectory->Get("BCALVsFCAL_TrigBit1");
	TH2* locHist_BCALVsECAL_TrigBit1 = (TH2*)gDirectory->Get("BCALVsECAL_TrigBit1");
	TH2* locHist_ECALVsFCAL_TrigBit1 = (TH2*)gDirectory->Get("ECALVsFCAL_TrigBit1");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("Trigger_Energy", "Trigger Energy", 1200, 900); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 2);
	
	TLatex latex;
	latex.SetTextSize(0.04);
	char str[256];

	// ECAL vs. FCAL for Trig bit 1
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLeftMargin(0.2);

	if(locHist_ECALVsFCAL_TrigBit1 != NULL)
	{
		locHist_ECALVsFCAL_TrigBit1->GetXaxis()->SetTitleSize(0.05);
		locHist_ECALVsFCAL_TrigBit1->GetYaxis()->SetTitleSize(0.04);
		locHist_ECALVsFCAL_TrigBit1->SetStats(0);
		locHist_ECALVsFCAL_TrigBit1->GetYaxis()->SetTitleOffset(2.0);
		locHist_ECALVsFCAL_TrigBit1->Draw("colz");

		gPad->SetLogz();
		gPad->Update();
	}

	// BCAL vs. FCAL for Trig bit 1
	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLeftMargin(0.2);
	if(locHist_BCALVsFCAL_TrigBit1 != NULL)
	{
		locHist_BCALVsFCAL_TrigBit1->GetXaxis()->SetTitleSize(0.05);
		locHist_BCALVsFCAL_TrigBit1->GetYaxis()->SetTitleSize(0.04);
		locHist_BCALVsFCAL_TrigBit1->SetStats(0);
		locHist_BCALVsFCAL_TrigBit1->GetYaxis()->SetTitleOffset(2.0);
		locHist_BCALVsFCAL_TrigBit1->Draw("colz");

		gPad->SetLogz();
		gPad->Update();
	}
	
	// BCAL vs. FCAL for Trig bit 1
	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLeftMargin(0.2);

	if(locHist_BCALVsFCAL_TrigBit1 != NULL)
	{
		locHist_BCALVsFCAL2_TrigBit1->GetXaxis()->SetTitleSize(0.05);
		locHist_BCALVsFCAL2_TrigBit1->GetYaxis()->SetTitleSize(0.04);
		locHist_BCALVsFCAL2_TrigBit1->SetStats(0);
		locHist_BCALVsFCAL2_TrigBit1->GetYaxis()->SetTitleOffset(2.0);
		locHist_BCALVsFCAL2_TrigBit1->Draw("colz");

		gPad->SetLogz();
		gPad->Update();
	}

	// BCAL vs. ECAL for Trig bit 1
	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLeftMargin(0.2);

	if(locHist_BCALVsECAL_TrigBit1 != NULL)
	{
		locHist_BCALVsECAL_TrigBit1->GetXaxis()->SetTitleSize(0.05);
		locHist_BCALVsECAL_TrigBit1->GetYaxis()->SetTitleSize(0.04);
		locHist_BCALVsECAL_TrigBit1->SetStats(0);
		locHist_BCALVsECAL_TrigBit1->GetYaxis()->SetTitleOffset(2.0);
		locHist_BCALVsECAL_TrigBit1->Draw("colz");

		gPad->SetLogz();
		gPad->Update();
	}

}
