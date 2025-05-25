
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
		locCanvas = new TCanvas("Helicity", "Helicity", 900, 600); //for testing
	else
		locCanvas = gPad->GetCanvas();
	gPad->SetMargin(0.2,0.1,0.2,0.2); // LRBT

	TLatex latex;
	latex.SetTextSize(0.04);
	char str[256];

	//locCanvas->cd(1);
	//double max = 0;
	if(locHist_Heli_asym_gtp!=NULL ) {
	    Long64_t nentries = locHist_Heli_asym_gtp->GetEntries();
	    if (nentries<100) {
		TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
		text->AddText("Too few entries");
		text->Draw();
	    } else {
		gPad->SetTicks();
		gPad->SetGrid();
		gPad->SetName("TrigAsym"); // used by RSAI in filenaming

		const int bin_number = 3;
		const char *bin_label[bin_number] = {"Main (1)", "BCal (3)", "PS (4)"};
		TH1D *locHist_Trigger_asym = new TH1D("locHist_Trigger_asym", "L1 Trigger Helicity Correlated Asymmetry;;Asymmetry  (%)",
						      bin_number, 0, bin_number);
		for (int i=1; i <= bin_number; i++)
		  locHist_Trigger_asym->GetXaxis()->SetBinLabel(i,bin_label[i-1]);

		// Main Trigger BCAL+FCAL2: GTP Bit 1
		int bit = 1;
		int bin = 1;
		double num =(locHist_Heli_asym_gtp->GetBinContent(bit,1)-locHist_Heli_asym_gtp->GetBinContent(bit,2));
		double den1 = (locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2));
		double asym = 1e0*num/den1;
		double err = 1e0/sqrt((locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2)));
		double sig1 = abs(asym/err);
		//printf("bit %i %.2f +- %.2f ppm\n",bit, asym, err);
		locHist_Trigger_asym->SetBinContent(bin, asym);
		locHist_Trigger_asym->SetBinError(bin, err);
		
		// BCAL Trigger: GTP Bit 3
		bit=3; bin = 2;
		num =(locHist_Heli_asym_gtp->GetBinContent(bit,1)-locHist_Heli_asym_gtp->GetBinContent(bit,2));
		double den2 = (locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2));
		asym = 1e0*num/den2;
		err = 1e0/sqrt((locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2)));
		double sig2 = abs(asym/err);

		//printf("bit %i %.2f +- %.2f ppm\n",bit, asym, err);
		locHist_Trigger_asym->SetBinContent(bin, asym);
		locHist_Trigger_asym->SetBinError(bin, err);

		// PS Trigger: GTP Bit 4
		bit=4; bin = 3;
		num =(locHist_Heli_asym_gtp->GetBinContent(bit,1)-locHist_Heli_asym_gtp->GetBinContent(bit,2));
		double den3 = (locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2));
		asym = 1e0*num/den3;
		err = 1e0/sqrt((locHist_Heli_asym_gtp->GetBinContent(bit,1)+locHist_Heli_asym_gtp->GetBinContent(bit,2)));
		double sig3 = abs(asym/err);
		//printf("bit %i %.2f +- %.2f ppm  %.1f\n",bit, asym, err, sig3);
		locHist_Trigger_asym->SetBinContent(bin, asym);
		locHist_Trigger_asym->SetBinError(bin, err);

		locHist_Trigger_asym->SetStats(0);
		locHist_Trigger_asym->GetXaxis()->LabelsOption("v");
		locHist_Trigger_asym->GetXaxis()->SetLabelSize(0.06);
		locHist_Trigger_asym->SetMarkerStyle(20);
		locHist_Trigger_asym->SetMarkerSize(1);
		locHist_Trigger_asym->SetMarkerColor(kBlue);
		locHist_Trigger_asym->SetLineColor(kBlue);
		locHist_Trigger_asym->Draw("p");
		gPad->Update(); 
		float min = locHist_Trigger_asym->GetMinimum();
		float max = locHist_Trigger_asym->GetMaximum();
		float ymax = gPad->GetUymax();
		float ymin = gPad->GetUymin();
		printf("%f %f %f %f\n",max,min,ymax,ymin);
		if (abs(max)<2e-3 && abs(min)<2e-3) {
		    locHist_Trigger_asym->Scale(1e6);
		    locHist_Trigger_asym->GetYaxis()->SetTitle("Asymmetry  (ppm)");
		} else {
		    locHist_Trigger_asym->Scale(1e2);
		}
		if (ymin>0) locHist_Trigger_asym->SetMinimum(0);
		if (ymax<0) locHist_Trigger_asym->SetMaximum(0);
		gPad->Modified();
		gPad->Update(); 
		min = locHist_Trigger_asym->GetMinimum();
		max = locHist_Trigger_asym->GetMaximum();
		ymax = gPad->GetUymax();
		ymin = gPad->GetUymin();
		printf("%f %f %f %f\n",max,min,ymax,ymin);
		sprintf(str, "%.1e    %.1f#sigma", den1,sig1);
		latex.DrawLatex(0.1, ymax*1.02, str);
		sprintf(str, "%.1e    %.1f#sigma", den2,sig2);
		latex.DrawLatex(1.1, ymax*1.02, str);
		sprintf(str, "%.1e    %.1f#sigma", den3,sig3);
		latex.DrawLatex(2.1, ymax*1.02, str);

	    }

	}
}
