// hnamepath: /fa125_temp/temp
//
// e-mail: njarvis@jlab.org
//
// Guidance: --------------------------------------------
//
//  This plot indicates fa125 temperature, it should be well below 185F.
//  
//  
// End Guidance: ----------------------------------------

{
	TDirectory *locTopDirectory = gDirectory;
	std::cout << "HistMacro_fa125_temp: Entering..." << std::endl;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("fa125_temp");
	if(!locDirectory){
		std::cout << "HistMacro_fa125_temp: Unable to find fa125_temp directory!" << std::endl;
		return;
	}
	
	locDirectory->cd(); // cd to fa125_temp

	TH2I *locHist = (TH2I*)gDirectory->Get("temp");
	if (!locHist){
		std::cout << "HistMacro_fa125_temp: Unable to find temperature histogram!" << std::endl;
	   return;
	}

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("fa125_temp", "fa125_temp", 700, 500); //for testing
	else
		locCanvas = gPad->GetCanvas();

	//Draw
	std::cout << "HistMacro_fa125_temp: Drawing Canvas" << std::endl;
	locCanvas->cd();
        gStyle->SetGridStyle(1);
	gPad->SetTicks();
	gPad->SetGrid();
        gPad->SetMargin(0.1,0.13,0.1,0.1);  //LRBT

	if(locHist != NULL)
	{
      std::cout << "HistMacro_fa125_temp: Drawing Hist" << std::endl;
		locHist->GetXaxis()->SetTitleSize(0.05);
		locHist->GetYaxis()->SetTitleSize(0.05);
		locHist->GetXaxis()->SetLabelSize(0.05);
		locHist->GetYaxis()->SetLabelSize(0.035);
		locHist->GetYaxis()->SetNdivisions(17);
		locHist->GetYaxis()->CenterLabels(1);
		locHist->SetStats(0);
		locHist->SetContour(50);
		locHist->SetMinimum(60);
		locHist->SetMaximum(120);

		locHist->Draw("colz");  // don't use colz2, it is buggy in -b mode
	}


#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){

          std::cout << "HistMacro_fa125_temp: RSAI block" << std::endl;

          double Nevents = locHist->GetEntries();

	  auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
	  if( min_events < 1 ) min_events = 50;
	  if( Nevents >= min_events ) {
	    cout << "RS Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
	    rs_SavePad("fa125_temp", 0);
	    rs_ResetAllMacroHistos("//HistMacro_fa125_temp");

	  }
	}
#endif
	
}
