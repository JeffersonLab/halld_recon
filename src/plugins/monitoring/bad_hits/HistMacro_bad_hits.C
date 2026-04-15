// hnamepath: /bad_hits/hroc
// hnamepath: /bad_hits/hroc_slot
// hnamepath: /bad_hits/num_events
//
// e-mail: njarvis@jlab.org
//
// Guidance: --------------------------------------------
//
//  This plot should be completely empty (white).
//  
//  Any filled cells indicate a data format problem with an fadc.
//
//  If there are many filled cells, stop the DAQ, reboot the bad ROC (the ROCid is on the plot's x-axis) and restart the DAQ.  If that does not clear the error, contact a DAQ expert. 
//  
// End Guidance: ----------------------------------------

{
	bool debug = false;

	TDirectory *locTopDirectory = gDirectory;
	if (debug) std::cout << "HistMacro_bad_hits: Entering..." << std::endl;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("bad_hits");
	if(!locDirectory){
		std::cout << "HistMacro_bad_hits: Unable to find bad_hits directory!" << std::endl;
		return;
	}
	
	locDirectory->cd(); // cd to bad_hits

	TH2I *locHistroc = (TH2I*)gDirectory->Get("roc");
	if (!locHistroc){
		std::cout << "HistMacro_bad_hits: Unable to find histogram!" << std::endl;
	   return;
	}

	TH2I *locHist = (TH2I*)gDirectory->Get("roc_slot");
	if (!locHist){
		std::cout << "HistMacro_bad_hits: Unable to find histogram!" << std::endl;
	   return;
	}


	// find the 3 rocids with the most problems
	int rocid[3] = {0};
	int count[3] = {0};
	
        for (int i=1; i<=locHistroc->GetXaxis()->GetNbins(); i++) {
	  int nerrors = locHistroc->GetBinContent(i);

          if (nerrors > count[0]) {
	    count[2] = count[1];
	    count[1] = count[0];
	    rocid[2] = rocid[1];
	    rocid[1] = rocid[0];

	    count[0] = nerrors;
	    rocid[0] = i;
	    
          } else if (nerrors > count[1]) {
	    count[2] = count[1];
	    rocid[2] = rocid[1];
	    count[1] = nerrors;
	    rocid[1] = i;
	    
          }else if (nerrors > count[2]) {
	    count[2] = nerrors;
	    rocid[2] = i;
          }
	}


	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("bad_hits", "bad_hits", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();

	//Draw
	if (debug) std::cout << "HistMacro_bad_hits: Drawing Canvas" << std::endl;
	locCanvas->cd();
	gStyle->SetPalette(kCool);
        gStyle->SetOptStat(0);
        gStyle->SetGridStyle(1);
	gPad->SetTicks();
	gPad->SetGrid();
        gPad->SetMargin(0.1,0.13,0.1,0.1);  //LRBT

	if(locHist != NULL)
	{
		if (debug) std::cout << "HistMacro_bad_hits: Drawing Hist" << std::endl;
		locHist->GetXaxis()->SetTitleSize(0.05);
		locHist->GetYaxis()->SetTitleSize(0.05);
		locHist->GetXaxis()->SetLabelSize(0.05);
		locHist->GetYaxis()->SetLabelSize(0.035);
		
		locHist->GetYaxis()->SetNdivisions(210);  
		locHist->GetXaxis()->SetNdivisions(15);

		if (rocid[2]>0) {
		  locHist->SetTitle(Form("Format errors in rocs %i %i %i",rocid[0],rocid[1],rocid[2]));
		} else if (rocid[1]>0) {
		  locHist->SetTitle(Form("Format errors in rocs %i %i",rocid[0],rocid[1]));
		} else if (rocid[0]>0) {
		  locHist->SetTitle(Form("Format errors in roc %i",rocid[0]));		  
		} else {
		  locHist->SetTitle("No format errors found");
		}
		
		locHist->SetStats(0);
		locHist->Draw("colz");  // don't use colz2, it is buggy in -b mode
	}


#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){

          if (debug) std::cout << "HistMacro_bad_hits: RSAI block" << std::endl;

          double Nevents = 1.0;

          TH1I *hevents = (TH1I*)gDirectory->Get("num_events");

          if(hevents){
             Nevents = (double)hevents->GetBinContent(1);
             if (debug) std::cout << "HistMacro_bad_hits: Nevents=" << Nevents << std::endl;
          }else{
             std::cout << "HistMacro_bad_hits: unable to find /bad_hits/num_events !" << std::endl;
          }

	  auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
	  if( min_events < 1 ) min_events = 1E4;
	  if( Nevents >= min_events ) {
	    cout << "RF Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
	    rs_SavePad("bad_hits", 0);
	    rs_ResetAllMacroHistos("//HistMacro_bad_hits");

	  }
	}
#endif
	
}
