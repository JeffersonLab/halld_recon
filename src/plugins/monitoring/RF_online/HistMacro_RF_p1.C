// hnamepath: /RF/DeltaT_RF_Itself/FDCRF_SelfDeltaT
// hnamepath: /RF/DeltaT_RF_Itself/TOFRF_SelfDeltaT
// hnamepath: /RF/DeltaT_RF_Itself/TAGHRF_SelfDeltaT
// hnamepath: /RF/DeltaT_RF_Itself/PSCRF_SelfDeltaT

{
	TDirectory *locTopDirectory = gDirectory;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("RF");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get RF DeltaT Histograms
	gDirectory->cd("DeltaT_RF_Itself");
	TH1I* locHist_FDCRF_SelfDeltaT = (TH1I*)gDirectory->Get("FDCRF_SelfDeltaT");
	TH1I* locHist_TOFRF_SelfDeltaT = (TH1I*)gDirectory->Get("TOFRF_SelfDeltaT");
	TH1I* locHist_TAGHRF_SelfDeltaT = (TH1I*)gDirectory->Get("TAGHRF_SelfDeltaT");
	TH1I* locHist_PSCRF_SelfDeltaT = (TH1I*)gDirectory->Get("PSCRF_SelfDeltaT");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("RF_p1", "RF_p1", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLogy(1);
	if(locHist_FDCRF_SelfDeltaT != NULL)
	{
		TH1I* locHist = locHist_FDCRF_SelfDeltaT;
		locHist->SetTitle("RF FDC Self timing");
		locHist->Rebin(14);

		locHist->GetXaxis()->SetTitleSize(0.05);
		locHist->GetYaxis()->SetTitleSize(0.05);
		locHist->GetXaxis()->SetLabelSize(0.05);
		locHist->GetYaxis()->SetLabelSize(0.05);
		locHist->Draw();
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLogy(1);
	if(locHist_TOFRF_SelfDeltaT != NULL)
	{
		TH1I* locHist = locHist_TOFRF_SelfDeltaT;
		locHist->SetTitle("RF TOF Self timing");
		locHist->Rebin(12);

		locHist->GetXaxis()->SetTitleSize(0.05);
		locHist->GetYaxis()->SetTitleSize(0.05);
		locHist->GetXaxis()->SetLabelSize(0.05);
		locHist->GetYaxis()->SetLabelSize(0.05);
		locHist->Draw();
	}

	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLogy(1);
	if(locHist_TAGHRF_SelfDeltaT != NULL)
	{
		TH1I* locHist = locHist_TAGHRF_SelfDeltaT;
		locHist->SetTitle("RF TAGH Self timing");
		locHist->Rebin(14);

		locHist->GetXaxis()->SetTitleSize(0.05);
		locHist->GetYaxis()->SetTitleSize(0.05);
		locHist->GetXaxis()->SetLabelSize(0.05);
		locHist->GetYaxis()->SetLabelSize(0.05);
		locHist->Draw();
	}

	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLogy(1);
	if(locHist_PSCRF_SelfDeltaT != NULL)
	{
		TH1I* locHist = locHist_PSCRF_SelfDeltaT;
		locHist->SetTitle("RF PSC Self timing");
		locHist->Rebin(14);

		locHist->GetXaxis()->SetTitleSize(0.05);
		locHist->GetYaxis()->SetTitleSize(0.05);
		locHist->GetXaxis()->SetLabelSize(0.05);
		locHist->GetYaxis()->SetLabelSize(0.05);
		locHist->Draw();
	}

#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
	  auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
	  if( min_events < 1 ) min_events = 1E5;
	  if( Nevents >= min_events ) {
	    cout << "RF Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
	    rs_SavePad("RF_FDC_selftiming", 1);
	    rs_SavePad("RF_TOF_selftiming", 2);
	    rs_SavePad("RF_TAGH_selftiming", 3);
	    rs_SavePad("RF_PSC_selftiming", 4);

	    rs_ResetAllMacroHistos("//RF_online");
	  }
	}
#endif
	
}
