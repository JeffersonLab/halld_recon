{
	TDirectory *locTopDirectory = gDirectory;


	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(!locDirectory)
		return;
	locDirectory->cd();

	TH1* locHist_L1bits_fp           = (TH1*)gDirectory->Get("L1bits_fp");
        TH1* locHist_L1bits_fp_twelvehundhits           = (TH1*)gDirectory->Get("L1bits_fp_twelvehundhits"); // BCAL LED Pseudo Trigger(1200 hits in BCAL) //

	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("locCanvas", "locCanvas", 1200, 900); //for testing
	else
		locCanvas = gPad->GetCanvas();
	
	TLatex latex;
	latex.SetTextSize(0.04);
	char str[256];

	locCanvas->cd(1);
	if(locHist_L1bits_fp !=NULL)
	{


		gPad->SetTicks();
		gPad->SetGrid();
		gPad->SetLogy();

		
		double max_fp  = locHist_L1bits_fp->GetMaximum();
		
		
		const int bin_number = 8;
		const char *bin_label[bin_number] = { "FCal LED (3)","BCAL 1200Hits", "BCal LED UP(9)", "pseudoTrig UP(9)","BCal LED DN(10)", "pseudoTrig DN(10)","Random (12)", "CTOF (6)"};
		TH1I *locHist_Trigger_pseudo = new TH1I("locHist_Trigger_pseudo", "L1 Front Panel Trigger Bits", bin_number, 0, bin_number);
		TH1I *locHist_Trigger_FP = new TH1I("locHist_Trigger_FP", "", bin_number, 0, bin_number);
	
		for (int i=1; i <= bin_number; i++)
		  locHist_Trigger_pseudo->GetXaxis()->SetBinLabel(i,bin_label[i-1]);


		// FCAL LED: FP Bit 3
		locHist_Trigger_FP->Fill(0., locHist_L1bits_fp->GetBinContent(3));
                //BCAL:1200 hits
                locHist_Trigger_FP->Fill(1., locHist_L1bits_fp_twelvehundhits->GetBinContent(1));
		// BCAL LED Upstream: FP Bit 9
		locHist_Trigger_FP->Fill(2., locHist_L1bits_fp->GetBinContent(9));
                // BCAL LED Upstream: pseudo Trigger 9
		locHist_Trigger_pseudo->Fill(3., locHist_L1bits_fp_twelvehundhits->GetBinContent(3));
		// BCAL LED Downstream: FP Bit 10
		locHist_Trigger_FP->Fill(4., locHist_L1bits_fp->GetBinContent(10));
                // BCAL LED Downstream: pseudo Trigger 10
		locHist_Trigger_pseudo->Fill(5., locHist_L1bits_fp_twelvehundhits->GetBinContent(4));
		// Random Trigger: FP Bit 12
		locHist_Trigger_FP->Fill(6., locHist_L1bits_fp->GetBinContent(12));
		// CTOF: FP Bit 6
		locHist_Trigger_FP->Fill(7., locHist_L1bits_fp->GetBinContent(6));

		locHist_Trigger_pseudo->SetFillColor(kBlue);
		locHist_Trigger_pseudo->SetStats(0);
		locHist_Trigger_pseudo->GetYaxis()->SetRangeUser(1.0, max_fp*5.0);
                locHist_Trigger_pseudo->SetLineWidth(0);
		locHist_Trigger_pseudo->Draw("bar1");
		

		locHist_Trigger_FP->SetFillColorAlpha(kRed-4, 0.25);
                locHist_Trigger_FP->SetLineWidth(0);
		locHist_Trigger_FP->Draw("bar1 same");
		

		TLegend *legend_pseudo = new TLegend(0.5,0.85,0.7,0.9);
		TLegend *legend_fp  = new TLegend(0.7,0.85,0.9,0.9);
		legend_pseudo->AddEntry(locHist_Trigger_pseudo,"pseudo Trig","f");
		legend_fp->AddEntry(locHist_Trigger_FP,"FP Trig","f");
		legend_pseudo->Draw();
		legend_fp->Draw();
	}
}
