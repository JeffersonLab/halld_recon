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
//  Any filled cells indicate a problem with an evio data bank, or unlocked TDCs.
//
//  If there are many filled cells in one ROC, rebooting it might help.  Stop the DAQ, reboot the bad ROC (the ROCid is on the plot's x-axis) and restart the DAQ. 
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


        TH1I *hevents = (TH1I*)gDirectory->Get("num_events");
	if (!hevents) {
             std::cout << "HistMacro_bad_hits: unable to find /bad_hits/num_events !" << std::endl;
	     return;
	}

        double Nevents = (double)hevents->GetBinContent(1);
        if (debug) std::cout << "HistMacro_bad_hits: Nevents=" << Nevents << std::endl;

	
	double total_errors = locHistroc->GetEntries();

        TString htitle = "Errors in data bank";

	int num_banks = 0;
	for (int i=1; i<=locHistroc->GetNbinsX(); i++) {
	  if (locHistroc->GetBinContent(i) >0) num_banks++;
	}

	if (num_banks > 1) htitle.Append("s");
	htitle.Append(" ");

	
	// NB there is no bin 0.  GetBinCenter(1) = 0
	
	if (total_errors > 0) {

	  int special_bank_errs = 0;

	  for (int i=1 ; i<7 ; i++) {
	    special_bank_errs += locHistroc->GetBinContent(i);
	  }

	  if (special_bank_errs > 0) {
	  
	    TString banknames[6] = {"0 (swap) ","1 (TS scalers) ","2 (f250 scalers) ","3 (EPICS) ","4 (BOR) ","5 (Triggers) "};
  	    for (int i=1; i<7 ; i++) {
	      if (locHistroc->GetBinContent(i) > 0) htitle.Append(banknames[i-1]);
	    }

	    if (special_bank_errs < (int)total_errors) htitle.Append("and ");
	  }

	  if (special_bank_errs < (int)total_errors) {

	    htitle.Append("from ROC");
		      
	    // find the 3 rocids with the most problems
	    int rocid[3] = {0};
	    int count[3] = {0};
	
	    for (int i=7; i<=locHistroc->GetXaxis()->GetNbins(); i++) {
	      int nerrors = locHistroc->GetBinContent(i);

	      if (nerrors > count[0]) {
		count[2] = count[1];
		count[1] = count[0];
		rocid[2] = rocid[1];
		rocid[1] = rocid[0];

		count[0] = nerrors;
		rocid[0] = i-1;
	    
	      } else if (nerrors > count[1]) {
		count[2] = count[1];
		rocid[2] = rocid[1];
		count[1] = nerrors;
		rocid[1] = i-1;
	    
	      } else if (nerrors > count[2]) {
		count[2] = nerrors;
		rocid[2] = i-1;
	      }
	    }

	    if (count[1] > 0) htitle.Append("s");
	    htitle.Append(" ");
	    
	    int sum_rocs_counted = 0;
	    for (int i=0; i<3 ; i++) {
	      if (count[i]>0) {
		if (i>0) htitle.Append(" and ");
	        htitle.Append(Form("%i",rocid[i]));
	      }
	      sum_rocs_counted += count[i];
	    } 

	    if (special_bank_errs + sum_rocs_counted  < (int)total_errors) htitle.Append(" etc");

	  }

	  htitle.Append(Form(".  Events processed: %.0f",Nevents));

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

		if (total_errors > 0) {
		  locHist->SetTitle(htitle);
		} else {
		  locHist->SetTitle(Form("No errors found.  Events processed: %.0f",Nevents));
		}
		
		locHist->SetStats(0);
		locHist->Draw("colz");  // don't use colz2, it is buggy in -b mode
	}


#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){

          if (debug) std::cout << "HistMacro_bad_hits: RSAI block" << std::endl;


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
