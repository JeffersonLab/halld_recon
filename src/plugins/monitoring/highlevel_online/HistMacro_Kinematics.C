// hnamepath: /highlevel/BetaVsP_TOF
// hnamepath: /highlevel/PSPairEnergy
//
// e-mail: davidl@jlab.org
// e-mail: staylor@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org
//

{
	TDirectory *locTopDirectory = gDirectory;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//TH2* locHist_PVsTheta_Tracks   = (TH2*)gDirectory->Get("PVsTheta_Tracks");
	TH2* BetaVsP                   = (TH2*)gDirectory->Get("BetaVsP_TOF");
	//TH2* locHist_PhiVsTheta_Tracks = (TH2*)gDirectory->Get("PhiVsTheta_Tracks");
	TH1* locHist_PSPairEnergy      = (TH1*)gDirectory->Get("PSPairEnergy");


	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("Kinematics", "Kinematics", 1200, 600); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 1);

	TLatex latex;
	latex.SetTextSize(0.04);
	char str[256];

	//------------ P vs. Theta --------------
// 	locCanvas->cd(1);
// 	gPad->SetTicks();
// 	gPad->SetGrid();
// 	if(locHist_PVsTheta_Tracks != NULL)
// 	{
// 		locHist_PVsTheta_Tracks->GetXaxis()->SetTitleSize(0.05);
// 		locHist_PVsTheta_Tracks->GetYaxis()->SetTitleSize(0.04);
// 		locHist_PVsTheta_Tracks->GetXaxis()->SetLabelSize(0.05);
// 		locHist_PVsTheta_Tracks->GetYaxis()->SetLabelSize(0.05);
// 		locHist_PVsTheta_Tracks->SetStats(0);
// 		locHist_PVsTheta_Tracks->Draw("colz");
// 
// 		sprintf(str, "%d entries", (uint32_t)locHist_PVsTheta_Tracks->GetEntries());
// 		latex.DrawLatex(10.0, 12.2, str);
// 	}

	//------------ PS energy --------------
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetName("PSEnergy"); // used by RSAI in filenaming
	double Nevents = 1.0;
	if(locHist_PSPairEnergy != NULL)
	{
		Nevents = (double)locHist_PSPairEnergy->Integral(0,450);
		locHist_PSPairEnergy->GetXaxis()->SetTitleSize(0.05);
		locHist_PSPairEnergy->GetYaxis()->SetTitleSize(0.04);
		locHist_PSPairEnergy->GetXaxis()->SetLabelSize(0.05);
		locHist_PSPairEnergy->GetYaxis()->SetLabelSize(0.05);
		// GlueX
		locHist_PSPairEnergy->GetXaxis()->SetRangeUser(6.,12.);
		// CPP
		// locHist_PSPairEnergy->GetXaxis()->SetRangeUser(3.5,8.5);
		locHist_PSPairEnergy->SetStats(0);
		locHist_PSPairEnergy->Draw();
		
		double Epeak = locHist_PSPairEnergy->GetBinCenter(locHist_PSPairEnergy->GetMaximumBin());

		sprintf(str, "Epeak: %3.2f GeV", Epeak);
		latex.DrawLatex(9.5, locHist_PSPairEnergy->GetMaximum()*0.85, str);
	}

	//------------ Beta vs. P --------------
	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(BetaVsP != NULL)
	{
		BetaVsP->GetXaxis()->SetTitleSize(0.05);
		BetaVsP->GetYaxis()->SetTitleSize(0.045);
		BetaVsP->GetXaxis()->SetLabelSize(0.05);
		BetaVsP->GetYaxis()->SetLabelSize(0.05);
		BetaVsP->SetStats(0);
		BetaVsP->Draw("colz");
		gPad->SetLogz();
		gPad->Update();
	}

	//------------ Phi vs. Theta --------------
// 	locCanvas->cd(4);
// 	gPad->SetTicks();
// 	gPad->SetGrid();
// 	if(locHist_PhiVsTheta_Tracks != NULL)
// 	{
// 		locHist_PhiVsTheta_Tracks->GetXaxis()->SetTitleSize(0.05);
// 		locHist_PhiVsTheta_Tracks->GetYaxis()->SetTitleSize(0.04);
// 		locHist_PhiVsTheta_Tracks->GetXaxis()->SetLabelSize(0.05);
// 		locHist_PhiVsTheta_Tracks->GetYaxis()->SetLabelSize(0.05);
// 		locHist_PhiVsTheta_Tracks->SetStats(0);
// 		locHist_PhiVsTheta_Tracks->Draw("colz");
// 
// 		sprintf(str, "%d entries", (uint32_t)locHist_PhiVsTheta_Tracks->GetEntries());
// 		latex.DrawLatex(10.0, 185.0, str);
// 	}
#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
		auto min_events = rs_GetFlag("MIN_EVENTS_RSAI");
		//if( min_events < 1 )
                min_events = 1E4;
		if( Nevents >= min_events ) {
			cout << "PS Pair Energy AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
			rs_SavePad("Kinematics", 1);
			rs_ResetAllMacroHistos("//HistMacro_Kinematics");
		}
	}
#endif
}
