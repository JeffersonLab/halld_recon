// hnamepath: /npp_hists/Hist_MissingMass_PreKinFit/MissingMass
// hnamepath: /npp_hists/Hist_InvariantMass_Pi0_PreKinFit/InvariantMass
// hnamepath: /npp_hists/Hist_KinFitResults/ConfidenceLevel
// hnamepath: /npp_hists/Hist_InvariantMass_2Pi0_KinFit/InvariantMass
// hnamepath: /npp_hists/Custom_npp_hists/t
// hnamepath: /npp_hists/Custom_npp_hists/Phi
{
	TDirectory *locInitDirectory = gDirectory;
	TDirectory *locReactionDirectory = (TDirectory*)locInitDirectory->FindObjectAny("npp_hists");
	if(locReactionDirectory == NULL)
		return;

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("npp", "npp", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(3, 2);

	TH1I* locHist_MM = (TH1I*)locReactionDirectory->Get("Hist_MissingMass_PreKinFit/MissingMass");
	TH1I* locHist_Pi0_Mass = (TH1I*)locReactionDirectory->Get("Hist_InvariantMass_Pi0_PreKinFit/InvariantMass");
	TH1I* locHist_KinFitConLev = (TH1I*)locReactionDirectory->Get("Hist_KinFitResults/ConfidenceLevel");
	TH1I* locHist_2Pi0_Mass = (TH1I*)locReactionDirectory->Get("Hist_InvariantMass_2Pi0_KinFit/InvariantMass");
	TH1I* locHist_t = (TH1I*)locReactionDirectory->Get("Custom_npp_hists/t");
	TH1I* locHist_Phi = (TH1I*)locReactionDirectory->Get("Custom_npp_hists/Phi");

	//Draw
	int locNumRebin = 4;

	//Missing Mass
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_MM != NULL)
	{
		locHist_MM->SetTitle("Missing Mass");
		//locHist_MM->Rebin(locNumRebin);
		locHist_MM->GetYaxis()->SetRangeUser(0.0, 1.05*locHist_MM->GetBinContent(locHist_MM->GetMaximumBin()));
		locHist_MM->GetXaxis()->SetTitleSize(0.05);
		locHist_MM->GetYaxis()->SetTitle("");
		locHist_MM->GetXaxis()->SetLabelSize(0.05);
		locHist_MM->GetYaxis()->SetLabelSize(0.05);
		locHist_MM->SetFillColor(kRed);
		locHist_MM->Draw("");
	}

	//Pi0 Mass
	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_Pi0_Mass != NULL)
	{
	  //locHist_Pi0_Mass->Rebin(locNumRebin);

		locHist_Pi0_Mass->SetTitle("#pi_{0} Mass");
		locHist_Pi0_Mass->GetXaxis()->SetTitleSize(0.05);
		locHist_Pi0_Mass->GetYaxis()->SetTitle("");
		locHist_Pi0_Mass->GetXaxis()->SetLabelSize(0.05);
		locHist_Pi0_Mass->GetYaxis()->SetLabelSize(0.05);
		locHist_Pi0_Mass->SetFillColor(kAzure + 1);
		locHist_Pi0_Mass->GetYaxis()->SetRangeUser(0.0, 1.05*locHist_Pi0_Mass->GetBinContent(locHist_Pi0_Mass->GetMaximumBin()));
		locHist_Pi0_Mass->Draw("");
	}

	//KinFit Confidence Level
	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLogy();
	if(locHist_KinFitConLev != NULL)
	{
		locHist_KinFitConLev->SetTitle("Confidence Level");
		locHist_KinFitConLev->GetXaxis()->SetTitleOffset(1.2);
		locHist_KinFitConLev->GetXaxis()->SetTitleSize(0.04);
		locHist_KinFitConLev->GetYaxis()->SetTitle("");
		locHist_KinFitConLev->GetXaxis()->SetLabelSize(0.05);
		locHist_KinFitConLev->GetYaxis()->SetLabelSize(0.05);
		locHist_KinFitConLev->SetFillColor(kTeal + 1);
		locHist_KinFitConLev->Draw("");
	}

	//Squared 4-Momentum Transfer -t
	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	gPad->SetLogy();
	if(locHist_t != NULL)
	{
		locHist_t->SetTitle("-t (KinFit CL > 0.01)");
		locHist_t->GetXaxis()->SetTitleSize(0.05);
		locHist_t->GetYaxis()->SetTitle("");
		locHist_t->GetXaxis()->SetLabelSize(0.05);
		locHist_t->GetYaxis()->SetLabelSize(0.05);
		locHist_t->SetFillColor(kOrange-2);
 		locHist_t->Draw("");
	}

	//2Pi0 Mass PostKinFit
	locCanvas->cd(5);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_2Pi0_Mass != NULL)
	{
	  //locHist_2Pi0_Mass->Rebin(locNumRebin);

		locHist_2Pi0_Mass->SetTitle("#pi_{0}#pi_{0} Mass (KinFit CL > 0.01)");
		locHist_2Pi0_Mass->GetXaxis()->SetTitleSize(0.05);
		locHist_2Pi0_Mass->GetYaxis()->SetTitle("");
		locHist_2Pi0_Mass->GetXaxis()->SetLabelSize(0.05);
		locHist_2Pi0_Mass->GetYaxis()->SetLabelSize(0.05);
		locHist_2Pi0_Mass->SetLineColor(kBlue);
		locHist_2Pi0_Mass->SetLineWidth(2);
		locHist_2Pi0_Mass->GetYaxis()->SetRangeUser(0.0, 1.05*locHist_2Pi0_Mass->GetBinContent(locHist_2Pi0_Mass->GetMaximumBin()));
		locHist_2Pi0_Mass->Draw("");
	}

	//Phi
	locCanvas->cd(6);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_Phi != NULL)
	{

		locHist_Phi->SetTitle("");
		locHist_Phi->GetXaxis()->SetTitleSize(0.05);
		locHist_Phi->GetYaxis()->SetTitle("");
		locHist_Phi->GetXaxis()->SetLabelSize(0.05);
		locHist_Phi->GetYaxis()->SetLabelSize(0.05);
		locHist_Phi->GetXaxis()->SetTitle("#Phi (deg)");
		locHist_Phi->GetYaxis()->SetRangeUser(0.0, 1.05*locHist_Phi->GetBinContent(locHist_Phi->GetMaximumBin()));
		locHist_Phi->SetStats(0);
		locHist_Phi->Draw("");

		// fit 1+cos(2*phi) distribution
		TF1* fit = new TF1("Fit","[0]*(1.0 + [1]*cos(2*(x + [2])/180.*3.14159))");
		fit->SetLineColor(2);
		locHist_Phi->Fit(fit, "Q", "");
		locHist_Phi->Draw("e");

		// print fit parameters to canvas
		Double_t PSigma = fit->GetParameter(1);
		Double_t PSigmaErr = fit->GetParError(1);
		Double_t Phi0 = fit->GetParameter(2);
		Double_t Phi0Err = fit->GetParError(2);
		TLatex tx;
		tx.SetTextAlign(21);
		tx.SetTextSize(0.05);
		char text[100];
		sprintf(text, "P=%0.2f#pm%0.2f", PSigma, PSigmaErr);
		tx.DrawLatex(0., locHist_Phi->GetMaximum()*0.15, text);
		sprintf(text, "#phi_{0}=%0.2f#pm%0.2f", Phi0, Phi0Err);
		tx.DrawLatex(0., locHist_Phi->GetMaximum()*0.06, text);

	}


}

