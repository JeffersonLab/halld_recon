// hnamepath: /fa125_itrig/errcount

{
	TDirectory *locTopDirectory = gDirectory;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("fa125_itrig");
	if(!locDirectory) return;

        TH2I *locHist = (TH2I*)gDirectory->Get("/fa125_itrig/errcount");
        if (!locHist) return;

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("fa125_itrig", "fa125_itrig", 700, 500); //for testing
	else
		locCanvas = gPad->GetCanvas();

	//Draw
	locCanvas->cd();
        gStyle->SetPalette(kCool);
        gStyle->SetOptStat(0);
        gStyle->SetGridStyle(1);
	gPad->SetTicks();
	gPad->SetGrid();
        gPad->SetMargin(0.1,0.13,0.1,0.1);  //LRBT

	if(locHist != NULL)
	{
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
	  auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
	  if( min_events < 1 ) min_events = 1E4;
	  if( Nevents >= min_events ) {
	    cout << "RF Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
	    rs_SavePad("fa125_itrig", 1);
	    rs_ResetAllMacroHistos("//fa125_itrig");
	  }
	}
#endif
	
}
