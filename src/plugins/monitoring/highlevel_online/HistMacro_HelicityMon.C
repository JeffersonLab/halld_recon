
{
	TDirectory *locTopDirectory = gDirectory;


	// Grab remaining histos from highlevel directory
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(!locDirectory)
		return;
	locDirectory->cd();

	TH1* locHist_Heli_asym_gtp       = (TH1*)gDirectory->Get("Heli_asym_gtp");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("Helicity", "Helicity", 1200, 900); //for testing
	else
		locCanvas = gPad->GetCanvas();
	//locCanvas->Divide(2, 2);
	
	TLatex latex;
	latex.SetTextSize(0.04);
	char str[256];

	//locCanvas->cd(1);
	double max = 0;
	if(locHist_Heli_asym_gtp!=NULL )
	{
		gPad->SetTicks();
		gPad->SetGrid();
		gPad->SetName("TrigAsym"); // used by RSAI in filenaming

		const int bin_number = 3;
		const char *bin_label[bin_number] = {"Main (1)", "BCal (3)", "PS (4)"};
		TH1D *locHist_Trigger_asym = new TH1D("locHist_Trigger_asym", "L1 Trigger Helicity Correlated Asymmetry;;Asymmetry  (ppm)", bin_number, 0, bin_number);
		for (int i=1; i <= bin_number; i++)
		  locHist_Trigger_asym->GetXaxis()->SetBinLabel(i,bin_label[i-1]);

		// Main Trigger BCAL+FCAL2: GTP Bit 1
		int bit = 1;
		int bin = 1;
		double num =(locHist_Heli_asym_gtp->GetBinContent(bit,1)-locHist_Heli_asym_gtp->GetBinContent(bit,2));
		double den1 = (locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2));
		double asym = 1e6*num/den1;
		double err = 1e6/sqrt((locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2)));
		//printf("bit %i %.2f +- %.2f ppm\n",bit, asym, err);
		locHist_Trigger_asym->SetBinContent(bin, asym);
		locHist_Trigger_asym->SetBinError(bin, err);
		
		// BCAL Trigger: GTP Bit 3
		bit=3; bin = 2;
		num =(locHist_Heli_asym_gtp->GetBinContent(bit,1)-locHist_Heli_asym_gtp->GetBinContent(bit,2));
		double den2 = (locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2));
		asym = 1e6*num/den2;
		err = 1e6/sqrt((locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2)));
		//printf("bit %i %.2f +- %.2f ppm\n",bit, asym, err);
		locHist_Trigger_asym->SetBinContent(bin, asym);
		locHist_Trigger_asym->SetBinError(bin, err);

		// PS Trigger: GTP Bit 4
		bit=4; bin = 3;
		num =(locHist_Heli_asym_gtp->GetBinContent(bit,1)-locHist_Heli_asym_gtp->GetBinContent(bit,2));
		double den3 = (locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2));
		asym = 1e6*num/den3;
		err = 1e6/sqrt((locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2)));
		//printf("bit %i %.2f +- %.2f ppm\n",bit, asym, err);
		locHist_Trigger_asym->SetBinContent(bin, asym);
		locHist_Trigger_asym->SetBinError(bin, err);


		locHist_Trigger_asym->SetStats(0);
		locHist_Trigger_asym->GetXaxis()->LabelsOption("v");
		locHist_Trigger_asym->GetXaxis()->SetLabelSize(0.06);
		locHist_Trigger_asym->Draw("e");

		gPad->SetBottomMargin(0.2);
		gPad->Update(); 
		float ymax = gPad->GetUymax();//locHist_Trigger_asym->GetYaxis()->Ge
		sprintf(str, "%.1e", den1);
		latex.DrawLatex(0, ymax, str);
		sprintf(str, "%.1e", den2);
		latex.DrawLatex(1, ymax, str);
		sprintf(str, "%.1e", den3);
		latex.DrawLatex(2, ymax, str);

	}

}
