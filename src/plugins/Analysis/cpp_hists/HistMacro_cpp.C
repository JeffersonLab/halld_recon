// hnamepath: /cpp_hists/Hist_MissingMass_PreKinFit/MissingMass
// hnamepath: /cpp_hists/Hist_KinFitResults/ConfidenceLevel
// hnamepath: /cpp_hists/Hist_InvariantMass_2Pi_KinFit/InvariantMass
// hnamepath: /cpp_hists/Custom_cpp_hists/t_rho
// hnamepath: /cpp_hists/Custom_cpp_hists/Psi_rho
{
	TDirectory *locInitDirectory = gDirectory;
	TDirectory *locReactionDirectory = (TDirectory*)locInitDirectory->FindObjectAny("cpp_hists");
	if(locReactionDirectory == NULL)
		return;

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("cpp", "cpp", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(3, 2);

	// Get overall normalization to number of triggers
	TH1D* locHist_NumEvents = (TH1D*) locReactionDirectory->Get("NumEventsSurvivedAction");
	double n_triggers = locHist_NumEvents->GetBinContent(1);

	TH1I* locHist_MM = (TH1I*)locReactionDirectory->Get("Hist_MissingMass_PreKinFit/MissingMass");
	TH1I* locHist_KinFitConLev = (TH1I*)locReactionDirectory->Get("Hist_KinFitResults/ConfidenceLevel");
	TH1I* locHist_2Pi_Mass = (TH1I*)locReactionDirectory->Get("Hist_InvariantMass_2Pi_KinFit/InvariantMass");
	TH1I* locHist_t = (TH1I*)locReactionDirectory->Get("Custom_cpp_hists/t_rho");
	TH1I* locHist_Psi = (TH1I*)locReactionDirectory->Get("Custom_cpp_hists/Psi_rho");

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

	//2Pi Mass PostKinFit
	locCanvas->cd(5);
	gPad->SetTicks();
	gPad->SetGrid();
	double n_rho = 0;
	double rho_mass = 0;
	double rho_width = 0;
	if(locHist_2Pi_Mass != NULL)
	{
	  //locHist_2Pi_Mass->Rebin(locNumRebin);

		locHist_2Pi_Mass->SetTitle("#pi^{+}#pi^{-} Mass (KinFit CL > 0.01)");
		locHist_2Pi_Mass->GetXaxis()->SetTitleSize(0.05);
		locHist_2Pi_Mass->GetYaxis()->SetTitle("");
		locHist_2Pi_Mass->GetXaxis()->SetLabelSize(0.05);
		locHist_2Pi_Mass->GetYaxis()->SetLabelSize(0.05);
		locHist_2Pi_Mass->SetLineColor(kBlue);
		locHist_2Pi_Mass->SetLineWidth(2);
		locHist_2Pi_Mass->GetYaxis()->SetRangeUser(0.0, 1.05*locHist_2Pi_Mass->GetBinContent(locHist_2Pi_Mass->GetMaximumBin()));
		locHist_2Pi_Mass->Draw("");

		// locHist_2Pi_Mass->Fit("frho", "R");
		TF1 *fexpo = new TF1("fexpo", "expo", 0.32, 1.5);
		locHist_2Pi_Mass->Fit("fexpo", "QR0");
		TF1 *fsum = new TF1("fsum", "[0]*TMath::BreitWigner(x,[1],[2]) + TMath::Exp([3]+[4]*x)", 0.32, 1.5);
		fsum->SetParameter(0,10);
		fsum->SetParameter(1,0.770);
		fsum->SetParameter(2,0.1);
		fsum->SetParameter(3,fexpo->GetParameter(0));
		fsum->SetParameter(4,fexpo->GetParameter(1));
		locHist_2Pi_Mass->Fit("fsum", "QR0");
		TF1 *frho = new TF1("frho", "[0]*TMath::BreitWigner(x,[1],[2])", 0.32, 1.5);
		frho->SetParameter(0,fsum->GetParameter(0));
		frho->SetParameter(1,fsum->GetParameter(1));
		frho->SetParameter(2,fsum->GetParameter(2));
		frho->SetLineColor(2);
		frho->SetLineWidth(1);
		frho->Draw("same");

		n_rho = fsum->Integral(0.32,1.5);
		rho_mass = fsum->GetParameter(1);
		rho_width = fsum->GetParameter(2);


	}

	//Psi
	locCanvas->cd(6);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_Psi != NULL)
	{

		locHist_Psi->SetTitle("#psi, #rho(770) mass, coherent peak");
		locHist_Psi->GetXaxis()->SetTitleSize(0.05);
		locHist_Psi->GetYaxis()->SetTitle("");
		locHist_Psi->GetXaxis()->SetLabelSize(0.05);
		locHist_Psi->GetYaxis()->SetLabelSize(0.05);
		locHist_Psi->GetXaxis()->SetTitle("#Psi (deg)");
		locHist_Psi->GetYaxis()->SetRangeUser(0.0, 1.05*locHist_Psi->GetBinContent(locHist_Psi->GetMaximumBin()));
		locHist_Psi->SetStats(0);
		locHist_Psi->Draw("");

		// fit 1+cos(2*psi) distribution
		TF1* fit = new TF1("Fit","[0]*(1.0 + [1]*cos(2*(x + [2])/180.*3.14159))");
		fit->SetLineColor(2);
		locHist_Psi->Fit(fit, "Q", "");
		locHist_Psi->Draw("e");

		// print fit parameters to canvas
		Double_t PSigma = fit->GetParameter(1);
		Double_t PSigmaErr = fit->GetParError(1);
		Double_t Psi0 = fit->GetParameter(2);
		Double_t Psi0Err = fit->GetParError(2);
		TLatex tx;
		tx.SetTextAlign(21);
		tx.SetTextSize(0.05);
		char text[100];
		sprintf(text, "P=%0.2f#pm%0.2f", PSigma, PSigmaErr);
		tx.DrawLatex(0., locHist_Psi->GetMaximum()*0.15, text);
		sprintf(text, "#psi_{0}=%0.2f#pm%0.2f", Psi0, Psi0Err);
		tx.DrawLatex(0., locHist_Psi->GetMaximum()*0.06, text);

	}

	// Print the rho mass and the number of reconstructed rhos per trigger
	locCanvas->cd(2);
	TLatex tx;
	tx.SetTextAlign(11);
	tx.SetTextSize(0.07);
	char text[100];
	sprintf(text, "M(#rho) = %0.3f GeV/c^{2}", rho_mass);
	tx.DrawLatex(0.1, 0.6, text);
	sprintf(text, "#Gamma(#rho) = %0.3f GeV/c^{2}", rho_width);
	tx.DrawLatex(0.1, 0.5, text);
	sprintf(text, "N(#rho) = %0.2f / 1M Trigger", n_rho/n_triggers*1000000);
	tx.DrawLatex(0.1, 0.4, text);


}

