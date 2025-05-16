// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/ECALTrackDistanceVsP
// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/ECALTrackDistanceVsTheta
// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/PVsTheta_HasHit
// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/PVsTheta_NoHit
// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/TrackECALRowVsColumn_HasHit
// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/TrackECALRowVsColumn_NoHit
// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/TrackECALP_HasHit
// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/TrackECALP_NoHit
// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/TrackECALR_HasHit
// hnamepath: /Independent/Hist_DetectorMatching/TimeBased/ECAL/TrackECALR_NoHit
{
	double locMinNumCountsForRatio = 50.0;

	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("Hist_DetectorMatching");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
	gDirectory->cd("TimeBased/ECAL");
	TH2I* locHist_ECAL_TrackDistanceVsP = (TH2I*)gDirectory->Get("ECALTrackDistanceVsP");
	TH2I* locHist_ECAL_TrackDistanceVsTheta = (TH2I*)gDirectory->Get("ECALTrackDistanceVsTheta");
	TH2I* locHist_PVsTheta_HasHit_ECAL = (TH2I*)gDirectory->Get("PVsTheta_HasHit");
	TH2I* locHist_PVsTheta_NoHit_ECAL = (TH2I*)gDirectory->Get("PVsTheta_NoHit");
	TH2I* locHist_TrackECALRowVsColumn_HasHit_ECAL = (TH2I*)gDirectory->Get("TrackECALRowVsColumn_HasHit");
	TH2I* locHist_TrackECALRowVsColumn_NoHit_ECAL = (TH2I*)gDirectory->Get("TrackECALRowVsColumn_NoHit");
	TH1I* locHist_TrackECALP_HasHit_ECAL = (TH1I*)gDirectory->Get("TrackECALP_HasHit");
	TH1I* locHist_TrackECALP_NoHit_ECAL = (TH1I*)gDirectory->Get("TrackECALP_NoHit");
	TH1I* locHist_TrackECALR_HasHit_ECAL = (TH1I*)gDirectory->Get("TrackECALR_HasHit");
	TH1I* locHist_TrackECALR_NoHit_ECAL = (TH1I*)gDirectory->Get("TrackECALR_NoHit");

	//Get original pad margins
	double locLeftPadMargin = gStyle->GetPadLeftMargin();
	double locRightPadMargin = gStyle->GetPadRightMargin();
	double locTopPadMargin = gStyle->GetPadTopMargin();
	double locBottomPadMargin = gStyle->GetPadBottomMargin();

	//Set new pad margins
	gStyle->SetPadLeftMargin(0.15);
	gStyle->SetPadRightMargin(0.15);
//	gStyle->SetPadTopMargin(locTopPadMargin);
//	gStyle->SetPadBottomMargin(locBottomPadMargin);

	//ECAL, by element (1 plot)
	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("Matching_ECAL", "Matching_ECAL", 1200, 800);
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(3, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_ECAL_TrackDistanceVsP != NULL)
	{
		//locHist_ECAL_TrackDistanceVsP->Rebin2D(2, 2);
		locHist_ECAL_TrackDistanceVsP->GetYaxis()->SetTitleOffset(1.3);
		locHist_ECAL_TrackDistanceVsP->GetXaxis()->SetTitleSize(0.05);
		locHist_ECAL_TrackDistanceVsP->GetYaxis()->SetTitleSize(0.05);
		locHist_ECAL_TrackDistanceVsP->GetXaxis()->SetLabelSize(0.05);
		locHist_ECAL_TrackDistanceVsP->GetYaxis()->SetLabelSize(0.05);
		locHist_ECAL_TrackDistanceVsP->GetYaxis()->SetRangeUser(0,10.);
		locHist_ECAL_TrackDistanceVsP->Draw("COLZ");
		TF1* locFunc = new TF1("ECAL_LCut_VsP", "0.26+1.8/x", 0.0, 10.0);
		locFunc->Draw("SAME");
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_ECAL_TrackDistanceVsTheta != NULL)
	{
		//locHist_ECAL_TrackDistanceVsTheta->Rebin2D(2, 2);
		locHist_ECAL_TrackDistanceVsTheta->GetYaxis()->SetTitleOffset(1.3);
		locHist_ECAL_TrackDistanceVsTheta->GetXaxis()->SetTitleSize(0.05);
		locHist_ECAL_TrackDistanceVsTheta->GetYaxis()->SetTitleSize(0.05);
		locHist_ECAL_TrackDistanceVsTheta->GetXaxis()->SetLabelSize(0.05);
		locHist_ECAL_TrackDistanceVsTheta->GetYaxis()->SetLabelSize(0.05);
		locHist_ECAL_TrackDistanceVsTheta->GetXaxis()->SetRangeUser(0,10.);
		locHist_ECAL_TrackDistanceVsTheta->GetYaxis()->SetRangeUser(0,10.);
		locHist_ECAL_TrackDistanceVsTheta->Draw("COLZ");
		//		TF1* locFunc = new TF1("ECAL_LCut_VsTheta", "2.75*(1.+0.002*x*x)", 0.0, 20.0);
		//locFunc->Draw("SAME");
	}

	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	if((locHist_PVsTheta_HasHit_ECAL != NULL) && (locHist_PVsTheta_NoHit_ECAL != NULL))
	{
		locHist_PVsTheta_HasHit_ECAL->Rebin2D(8, 5); //280x250 -> 35x50
		locHist_PVsTheta_NoHit_ECAL->Rebin2D(8, 5); //280x250 -> 35x50

		TH2I* locFoundHist = locHist_PVsTheta_HasHit_ECAL;
		TH2I* locMissingHist = locHist_PVsTheta_NoHit_ECAL;
		string locHistName = string(locFoundHist->GetName()) + string("_Acceptance");
		string locHistTitle = string("Track / ECAL Match Rate;") + string(locFoundHist->GetXaxis()->GetTitle()) + string(";") + string(locFoundHist->GetYaxis()->GetTitle());
		TH2D* locAcceptanceHist = new TH2D(locHistName.c_str(), locHistTitle.c_str(), locFoundHist->GetNbinsX(), locFoundHist->GetXaxis()->GetXmin(), locFoundHist->GetXaxis()->GetXmax(), locFoundHist->GetNbinsY(), locFoundHist->GetYaxis()->GetXmin(), locFoundHist->GetYaxis()->GetXmax());
		for(Int_t loc_m = 1; loc_m <= locFoundHist->GetNbinsX(); ++loc_m)
		{
			for(Int_t loc_j = 1; loc_j <= locFoundHist->GetNbinsY(); ++loc_j)
			{
				double locNumMissing = locMissingHist->GetBinContent(loc_m, loc_j);
				double locNumFound = locFoundHist->GetBinContent(loc_m, loc_j);
				double locTotal = locNumMissing + locNumFound;
				if(!(locTotal >= locMinNumCountsForRatio))
				{
					locAcceptanceHist->SetBinContent(loc_m, loc_j, 0.0);
					locAcceptanceHist->SetBinError(loc_m, loc_j, 1.0/0.0);
					continue;
				}

				double locAcceptance = locNumFound/locTotal;
				if(!(locAcceptance > 0.0))
					locAcceptance = 0.00001; //so that it shows up on the histogram
				locAcceptanceHist->SetBinContent(loc_m, loc_j, locAcceptance);
				double locNumFoundError = sqrt(locNumFound*(1.0 - locAcceptance));

				double locAcceptanceError = locNumFoundError/locTotal;
				locAcceptanceHist->SetBinError(loc_m, loc_j, locAcceptanceError);
			}
		}
		locAcceptanceHist->SetEntries(locMissingHist->GetEntries() + locFoundHist->GetEntries());
		locAcceptanceHist->SetStats(kFALSE);
		locAcceptanceHist->GetYaxis()->SetTitleOffset(1.3);
		locAcceptanceHist->GetXaxis()->SetTitleSize(0.05);
		locAcceptanceHist->GetYaxis()->SetTitleSize(0.05);
		locAcceptanceHist->GetXaxis()->SetLabelSize(0.05);
		locAcceptanceHist->GetYaxis()->SetLabelSize(0.05);
		locAcceptanceHist->Draw("COLZ");
	}

	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	if((locHist_TrackECALRowVsColumn_HasHit_ECAL != NULL) && (locHist_TrackECALRowVsColumn_NoHit_ECAL != NULL))
	{
		TH2I* locFoundHist = locHist_TrackECALRowVsColumn_HasHit_ECAL;
		TH2I* locMissingHist = locHist_TrackECALRowVsColumn_NoHit_ECAL;
		string locHistName = string(locFoundHist->GetName()) + string("_Acceptance");
		string locHistTitle = string("ECAL Match Rate (p > 1 GeV/c);") + string(locFoundHist->GetXaxis()->GetTitle()) + string(";") + string(locFoundHist->GetYaxis()->GetTitle());
		TH2D* locAcceptanceHist = new TH2D(locHistName.c_str(), locHistTitle.c_str(), locFoundHist->GetNbinsX(), locFoundHist->GetXaxis()->GetXmin(), locFoundHist->GetXaxis()->GetXmax(), locFoundHist->GetNbinsY(), locFoundHist->GetYaxis()->GetXmin(), locFoundHist->GetYaxis()->GetXmax());
		for(Int_t loc_m = 1; loc_m <= locFoundHist->GetNbinsX(); ++loc_m)
		{
			for(Int_t loc_j = 1; loc_j <= locFoundHist->GetNbinsY(); ++loc_j)
			{
				double locNumMissing = locMissingHist->GetBinContent(loc_m, loc_j);
				double locNumFound = locFoundHist->GetBinContent(loc_m, loc_j);
				double locTotal = locNumMissing + locNumFound;
				if(!(locTotal >= locMinNumCountsForRatio))
				{
					locAcceptanceHist->SetBinContent(loc_m, loc_j, 0.0);
					locAcceptanceHist->SetBinError(loc_m, loc_j, 1.0/0.0);
					continue;
				}

				double locAcceptance = locNumFound/locTotal;
				if(!(locAcceptance > 0.0))
					locAcceptance = 0.00001; //so that it shows up on the histogram
				locAcceptanceHist->SetBinContent(loc_m, loc_j, locAcceptance);
				double locNumFoundError = sqrt(locNumFound*(1.0 - locAcceptance));

				double locAcceptanceError = locNumFoundError/locTotal;
				locAcceptanceHist->SetBinError(loc_m, loc_j, locAcceptanceError);
			}
		}
		locAcceptanceHist->SetEntries(locMissingHist->GetEntries() + locFoundHist->GetEntries());
		locAcceptanceHist->SetStats(kFALSE);
		locAcceptanceHist->GetYaxis()->SetTitleOffset(1.3);
		locAcceptanceHist->GetXaxis()->SetTitleSize(0.05);
		locAcceptanceHist->GetYaxis()->SetTitleSize(0.05);
		locAcceptanceHist->GetXaxis()->SetLabelSize(0.05);
		locAcceptanceHist->GetYaxis()->SetLabelSize(0.05);
		locAcceptanceHist->Draw("COLZ");
	}

	locCanvas->cd(5);
	gPad->SetTicks();
	gPad->SetGrid();
	if((locHist_TrackECALP_HasHit_ECAL != NULL) && (locHist_TrackECALP_NoHit_ECAL != NULL))
	{
		TH1I* locFoundHist = locHist_TrackECALP_HasHit_ECAL;
		locFoundHist->Rebin(4);
		TH1I* locMissingHist = locHist_TrackECALP_NoHit_ECAL;
		locMissingHist->Rebin(4);

		string locHistName = string(locFoundHist->GetName()) + string("_Acceptance");
		string locHistTitle = string("Track / ECAL Match Rate;") + string(locFoundHist->GetXaxis()->GetTitle());
		TH1D* locAcceptanceHist = new TH1D(locHistName.c_str(), locHistTitle.c_str(), locFoundHist->GetNbinsX(), locFoundHist->GetXaxis()->GetXmin(), locFoundHist->GetXaxis()->GetXmax());
		for(Int_t loc_m = 1; loc_m <= locFoundHist->GetNbinsX(); ++loc_m)
		{
			double locNumMissing = locMissingHist->GetBinContent(loc_m);
			double locNumFound = locFoundHist->GetBinContent(loc_m);
			double locTotal = locNumMissing + locNumFound;
			if(!(locTotal >= locMinNumCountsForRatio))
			{
				locAcceptanceHist->SetBinContent(loc_m, 0.0);
				locAcceptanceHist->SetBinError(loc_m, 0.0);
				continue;
			}

			double locAcceptance = locNumFound/locTotal;
			locAcceptanceHist->SetBinContent(loc_m, locAcceptance);
			double locNumFoundError = sqrt(locNumFound*(1.0 - locAcceptance));

			double locAcceptanceError = locNumFoundError/locTotal;
			locAcceptanceHist->SetBinError(loc_m, locAcceptanceError);
		}
		locAcceptanceHist->SetEntries(locMissingHist->GetEntries() + locFoundHist->GetEntries());
		locAcceptanceHist->SetStats(kFALSE);

		locAcceptanceHist->GetXaxis()->SetTitleSize(0.05);
		locAcceptanceHist->GetYaxis()->SetTitleSize(0.05);
		locAcceptanceHist->GetXaxis()->SetLabelSize(0.05);
		locAcceptanceHist->GetYaxis()->SetLabelSize(0.05);
		locAcceptanceHist->Draw("E1");
	}

	locCanvas->cd(6);
	gPad->SetTicks();
	gPad->SetGrid();
	if((locHist_TrackECALR_HasHit_ECAL != NULL) && (locHist_TrackECALR_NoHit_ECAL != NULL))
	{
		TH1I* locFoundHist = locHist_TrackECALR_HasHit_ECAL;
		locFoundHist->Rebin(2);
		TH1I* locMissingHist = locHist_TrackECALR_NoHit_ECAL;
		locMissingHist->Rebin(2);

		string locHistName = string(locFoundHist->GetName()) + string("_Acceptance");
		string locHistTitle = string("ECAL Match Rate (p > 1 GeV/c);") + string(locFoundHist->GetXaxis()->GetTitle());
		TH1D* locAcceptanceHist = new TH1D(locHistName.c_str(), locHistTitle.c_str(), locFoundHist->GetNbinsX(), locFoundHist->GetXaxis()->GetXmin(), locFoundHist->GetXaxis()->GetXmax());
		for(Int_t loc_m = 1; loc_m <= locFoundHist->GetNbinsX(); ++loc_m)
		{
			double locNumMissing = locMissingHist->GetBinContent(loc_m);
			double locNumFound = locFoundHist->GetBinContent(loc_m);
			double locTotal = locNumMissing + locNumFound;
			if(!(locTotal >= locMinNumCountsForRatio))
			{
				locAcceptanceHist->SetBinContent(loc_m, 0.0);
				locAcceptanceHist->SetBinError(loc_m, 0.0);
				continue;
			}

			double locAcceptance = locNumFound/locTotal;
			locAcceptanceHist->SetBinContent(loc_m, locAcceptance);
			double locNumFoundError = sqrt(locNumFound*(1.0 - locAcceptance));

			double locAcceptanceError = locNumFoundError/locTotal;
			locAcceptanceHist->SetBinError(loc_m, locAcceptanceError);
		}
		locAcceptanceHist->SetEntries(locMissingHist->GetEntries() + locFoundHist->GetEntries());
		locAcceptanceHist->SetStats(kFALSE);

		locAcceptanceHist->GetXaxis()->SetTitleSize(0.05);
		locAcceptanceHist->GetYaxis()->SetTitleSize(0.05);
		locAcceptanceHist->GetXaxis()->SetLabelSize(0.05);
		locAcceptanceHist->GetYaxis()->SetLabelSize(0.05);
		locAcceptanceHist->Draw("E1");
	}

	//Reset original pad margins
	gStyle->SetPadLeftMargin(locLeftPadMargin);
	gStyle->SetPadRightMargin(locRightPadMargin);
	gStyle->SetPadTopMargin(locTopPadMargin);
	gStyle->SetPadBottomMargin(locBottomPadMargin);
}

