// hnamepath: /cpp_itrig/errcount
// hnamepath: /cpp_itrig/num_events
//
// e-mail: njarvis@jlab.org
//
// Guidance: --------------------------------------------
//
//  This plot should be completely empty (white).
//  
//  Any filled (pink) cells indicate a problem with an fadc.
//  The cells are filled if there are large inconsistencies in event count or in trigger times between an fadc and its TI, or between one crate and the rest. 
//
//  Large numbers of mismatches indicate that data from different triggers are being packaged together into the same event, becoming useless.  It could be caused by a misconfiguration during startup or by a hardware problem. 
//
//  If there are filled cells, stop the DAQ, reboot the bad ROC (the ROCid is on the plot's x-axis) and restart the DAQ.  If that does not clear the error, contact a DAQ expert. 
//  
// End Guidance: ----------------------------------------

{
	TDirectory *locTopDirectory = gDirectory;
	std::cout << "HistMacro_cpp_itrig: Entering..." << std::endl;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("cpp_itrig");
	if(!locDirectory){
		std::cout << "HistMacro_cpp_itrig: Unable to find cpp_itrig directory!" << std::endl;
		return;
	}
	
	locDirectory->cd(); // cd to cpp_itrig

	TH2I *locHist = (TH2I*)gDirectory->Get("errcount");
	if (!locHist){
		std::cout << "HistMacro_cpp_itrig: Unable to find errcount!" << std::endl;
	   return;
	}

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("cpp_itrig", "cpp_itrig", 700, 500); //for testing
	else
		locCanvas = gPad->GetCanvas();

	//Draw
	std::cout << "HistMacro_cpp_itrig: Drawing Canvas" << std::endl;
	locCanvas->cd();
        gStyle->SetPalette(kCool);
        gStyle->SetOptStat(0);
        gStyle->SetGridStyle(1);
	gPad->SetTicks();
	gPad->SetGrid();
        gPad->SetMargin(0.1,0.13,0.1,0.1);  //LRBT

	if(locHist != NULL)
	{
      std::cout << "HistMacro_cpp_itrig: Drawing Hist" << std::endl;
		locHist->GetXaxis()->SetTitleSize(0.05);
		locHist->GetYaxis()->SetTitleSize(0.05);
		locHist->GetXaxis()->SetLabelSize(0.05);
		locHist->GetYaxis()->SetLabelSize(0.035);
		locHist->GetYaxis()->SetNdivisions(17);
		locHist->GetYaxis()->CenterLabels(1);
		locHist->Draw("colz");  // don't use colz2, it is buggy in -b mode
	}

}
