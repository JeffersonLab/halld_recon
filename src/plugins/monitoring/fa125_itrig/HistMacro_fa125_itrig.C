// hnamepath: /fa125_itrig/errcount
// hnamepath: /fa125_itrig/num_events
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
	std::cout << "HistMacro_fa125_itrig: Entering..." << std::endl;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("fa125_itrig");
	if(!locDirectory){
		std::cout << "HistMacro_fa125_itrig: Unable to find fa125_itrig directory!" << std::endl;
		return;
	}
	
	locDirectory->cd(); // cd to fa125_itrig

	TH2I *locHist = (TH2I*)gDirectory->Get("errcount");
	if (!locHist){
		std::cout << "HistMacro_fa125_itrig: Unable to find errcount!" << std::endl;
	   return;
	}

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("fa125_itrig", "fa125_itrig", 700, 500); //for testing
	else
		locCanvas = gPad->GetCanvas();

	//Draw
	std::cout << "HistMacro_fa125_itrig: Drawing Canvas" << std::endl;
	locCanvas->cd();
        gStyle->SetPalette(kCool);
        gStyle->SetOptStat(0);
        gStyle->SetGridStyle(1);
	gPad->SetTicks();
	gPad->SetGrid();
        gPad->SetMargin(0.1,0.13,0.1,0.1);  //LRBT

	if(locHist != NULL)
	{
      std::cout << "HistMacro_fa125_itrig: Drawing Hist" << std::endl;
		locHist->GetXaxis()->SetTitleSize(0.05);
		locHist->GetYaxis()->SetTitleSize(0.05);
		locHist->GetXaxis()->SetLabelSize(0.05);
		locHist->GetYaxis()->SetLabelSize(0.035);
		locHist->GetYaxis()->SetNdivisions(17);
		locHist->GetYaxis()->CenterLabels(1);
		locHist->Draw("colz");  // don't use colz2, it is buggy in -b mode
	}


#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){

          std::cout << "HistMacro_fa125_itrig: RSAI block" << std::endl;

          double Nevents = 1.0;

          TH1I *hevents = (TH1I*)gDirectory->Get("num_events");

          if(hevents){
             Nevents = (double)num_events->GetBinContent(1);
             std::cout << "HistMacro_fa125_itrig: Nevents=" << Nevents << std::endl;
          }else{
             std::cout << "HistMacro_fa125_itrig: unable to find /fa125_itrig/num_events !" << std::endl;
          }

	  auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
	  if( min_events < 1 ) min_events = 1E4;
	  if( Nevents >= min_events ) {
	    cout << "RF Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
	    rs_SavePad("fa125_itrig", 0);
	    rs_ResetAllMacroHistos("//HistMacro_fa125_itrig");

	  }
	}
#endif
	
}
