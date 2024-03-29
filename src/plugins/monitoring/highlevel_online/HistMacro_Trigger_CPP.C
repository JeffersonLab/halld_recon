//
// NOTE: The L1GTPRate histo now comes from the 
// occupancy_online plugin since it should see many
// more sync events.
//
// Guidance: --------------------------------------------
//
// This page includes a mixture of low-level and high-level
// information. Values in the table regarding numbers of
// triggers can be highly sensitive to calibration. They 
// also may be slightly different between Amporphous and
// diamond radiators. If the values differ by more than
// around 20% from the reference, try hitting the "Reset"
// button to see if they improve with new statistics.
//
// If you have concerns about the values in the table or
// any of the plots looking significantly different than
// the reference, please contact one of:
//
//  Run Coordinator:  (757) 383-5542
//
//  David Lawrence:   x5567    (office)
//                    746-6697 (cell)
//                    595-1302 (home)
//
// End Guidance: ----------------------------------------
//
// hnamepath: /occupancy/L1GTPRate
// hnamepath: /occupancy/L1livetime
// hnamepath: /highlevel/BCALVsFCAL_TrigBit1
// hnamepath: /highlevel/L1bits_gtp
// hnamepath: /highlevel/L1bits_fp
// hnamepath: /highlevel/NumTriggers
//
// e-mail: davidl@jlab.org
// e-mail: staylor@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org
//

{
	TDirectory *locTopDirectory = gDirectory;


	// Grab remaining histos from highlevel directory
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(!locDirectory)
		return;
	locDirectory->cd();

	TH2* locHist_BCALVsFCAL_TrigBit1 = (TH2*)gDirectory->Get("BCALVsFCAL_TrigBit1");
	TH1* locHist_L1bits_gtp          = (TH1*)gDirectory->Get("L1bits_gtp");
	TH1* locHist_L1bits_fp           = (TH1*)gDirectory->Get("L1bits_fp");
	TH2* locHist_NumTriggers         = (TH2*)gDirectory->Get("NumTriggers");

	// Grab a couple of histos from occupancy directory
	locDirectory = (TDirectory*)locTopDirectory->FindObjectAny("occupancy");
	TH2* locHist_L1GTPRate  = NULL;
	TH1* locHist_L1livetime = NULL;
	if(locDirectory){
		locHist_L1GTPRate = (TH2*)locDirectory->Get("L1GTPRate");
		locHist_L1livetime = (TH1*)locDirectory->Get("L1livetime");
	}

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("Kinematics", "Kinematics", 1200, 900); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(3, 1);
	
	TLatex latex;
	latex.SetTextSize(0.04);
	char str[256];

	// -------------- Left --------------
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_L1GTPRate != NULL)
	{
		locHist_L1GTPRate->GetXaxis()->SetTitleSize(0.05);
		locHist_L1GTPRate->GetYaxis()->SetTitleSize(0.04);
		locHist_L1GTPRate->GetXaxis()->SetLabelSize(0.05);
		locHist_L1GTPRate->GetYaxis()->SetLabelSize(0.05);
		locHist_L1GTPRate->SetStats(0);
		locHist_L1GTPRate->Draw("colz");
		
		sprintf(str, "from %d sync events", (uint32_t)locHist_L1GTPRate->GetEntries()/8);
		latex.DrawLatex(1.0, 101.0, str);
	}

	// -------------- Middle --------------
	
	// Hadronic trigger rate stats
	locCanvas->cd(0);
	if(locHist_NumTriggers){
		TPad *pad = (TPad*)gDirectory->FindObjectAny("trigpad1");
		if(!pad) pad = new TPad("trigpad1", "", 0.33, 0.45, 0.66, 0.95);
		pad->Draw();
		pad->cd();
		
		TLatex latex;
		latex.SetTextSize(0.05);
		latex.SetTextAlign(31);

		latex.DrawLatex(0.5, 0.9, "trig 1");
		latex.DrawLatex(0.7, 0.9, "trig 2");
		latex.DrawLatex(0.9, 0.9, "trig 4");

		latex.DrawLatex(0.3, 0.70, "Triggers");

		latex.DrawLatex(0.3, 0.50, "Hadronic");
		latex.DrawLatex(0.3, 0.45, "triggers");

		latex.DrawLatex(0.3, 0.30, "Hadronic");
		latex.DrawLatex(0.3, 0.25, "triggers in");
		latex.DrawLatex(0.3, 0.20, "coh. peak");
		
		TLine line;
		line.SetLineWidth(3.0);
		line.SetLineColor(kGray+2);
		line.DrawLine(0.05, 0.40, 0.95, 0.40);
		line.DrawLine(0.05, 0.60, 0.95, 0.60);
		line.DrawLine(0.05, 0.80, 0.95, 0.80);
		line.DrawLine(0.55, 0.15, 0.55, 0.95);
		line.DrawLine(0.75, 0.15, 0.75, 0.95);
		
		latex.SetTextSize(0.04);
		latex.SetTextAlign(21);
		char str[256];
		TH2* h = locHist_NumTriggers;

		// trig 1
		sprintf(str, "%4.3g", h->GetBinContent(1,1));
		latex.DrawLatex(0.45, 0.725, str);
		sprintf(str, "%4.3g", h->GetBinContent(1,3));
		latex.DrawLatex(0.45, 0.500, str);
		sprintf(str, "%4.3g", h->GetBinContent(1,4));
		latex.DrawLatex(0.45, 0.275, str);

		// trig 2
		sprintf(str, "%4.3g", h->GetBinContent(2,1));
		latex.DrawLatex(0.65, 0.725, str);
		sprintf(str, "%4.3g", h->GetBinContent(2,3));
		latex.DrawLatex(0.65, 0.500, str);
		sprintf(str, "%4.3g", h->GetBinContent(2,4));
		latex.DrawLatex(0.65, 0.275, str);

		// trig 4
		sprintf(str, "%4.3g", h->GetBinContent(4,1));
		latex.DrawLatex(0.85, 0.725, str);
		sprintf(str, "%4.3g", h->GetBinContent(4,3));
		latex.DrawLatex(0.85, 0.500, str);
		sprintf(str, "%4.3g", h->GetBinContent(4,4));
		latex.DrawLatex(0.85, 0.275, str);
		
		latex.SetTextSize(0.05);
		latex.SetTextColor(kRed);

		TLatex latex2;
		latex2.SetTextSize(0.035);
		latex2.SetTextAlign(21);
		latex2.SetTextColor(kRed+2);

		double all_trigs = h->GetBinContent(33,1);

		// trig 1
		double Ntrig1 = h->GetBinContent(1,1);
		sprintf(str, "(%4.1f%%)", Ntrig1/all_trigs*100.0);
		latex.DrawLatex(0.45, 0.675, str);
		latex2.DrawLatex(0.45, 0.635, "of all trigs");
 		sprintf(str, "(%4.1f%%)", h->GetBinContent(1,3)/Ntrig1*100.0);
 		latex.DrawLatex(0.45, 0.450, str);
 		latex2.DrawLatex(0.45, 0.410, "of trig 1's");
 		sprintf(str, "(%4.1f%%)", h->GetBinContent(1,4)/Ntrig1*100.0);
 		latex.DrawLatex(0.45, 0.225, str);
 		latex2.DrawLatex(0.45, 0.185, "of trig 1's");

		// trig 2
		double Ntrig2 = h->GetBinContent(2,1);
		sprintf(str, "(%4.1f%%)", Ntrig2/all_trigs*100.0);
		latex.DrawLatex(0.65, 0.675, str);
		latex2.DrawLatex(0.65, 0.635, "of all trigs");
		sprintf(str, "(%4.1f%%)", h->GetBinContent(2,3)/Ntrig2*100.0);
		latex.DrawLatex(0.65, 0.450, str);
		latex2.DrawLatex(0.65, 0.410, "of trig 2's");
		sprintf(str, "(%4.1f%%)", h->GetBinContent(2,4)/Ntrig2*100.0);
		latex.DrawLatex(0.65, 0.225, str);
		latex2.DrawLatex(0.65, 0.185, "of trig 2's");

		// trig 4
		double Ntrig4 = h->GetBinContent(4,1);
		sprintf(str, "(%4.1f%%)", Ntrig4/all_trigs*100.0);
		latex.DrawLatex(0.85, 0.675, str);
		latex2.DrawLatex(0.85, 0.635, "of all trigs");
		sprintf(str, "(%4.1f%%)", h->GetBinContent(4,3)/Ntrig4*100.0);
		latex.DrawLatex(0.85, 0.450, str);
		latex2.DrawLatex(0.85, 0.410, "of trig 4's");
		sprintf(str, "(%4.1f%%)", h->GetBinContent(4,4)/Ntrig4*100.0);
		latex.DrawLatex(0.85, 0.225, str);
		latex2.DrawLatex(0.85, 0.185, "of trig 4's");
		
	}
	
	// BCAL vs. FCAL for Trig bit 1
	locCanvas->cd(0);
	if(locHist_BCALVsFCAL_TrigBit1 != NULL)
	{
		TPad *pad = (TPad*)gDirectory->FindObjectAny("trigpad2");
		if(!pad) pad = new TPad("trigpad2", "", 0.33, 0.0, 0.66, 0.5);
		pad->Draw();
		pad->cd();

		gPad->SetTicks();
		gPad->SetGrid();
		gPad->SetLeftMargin(0.2);

		locHist_BCALVsFCAL_TrigBit1->GetXaxis()->SetTitleSize(0.05);
		locHist_BCALVsFCAL_TrigBit1->GetYaxis()->SetTitleSize(0.04);
		locHist_BCALVsFCAL_TrigBit1->SetStats(0);
		locHist_BCALVsFCAL_TrigBit1->GetYaxis()->SetTitleOffset(2.0);
		locHist_BCALVsFCAL_TrigBit1->Draw("colz");

		sprintf(str, "%d entries", (uint32_t)locHist_BCALVsFCAL_TrigBit1->GetEntries());
		latex.DrawLatex(500.0, 50000.0*1.01, str);

		gPad->SetLogz();
		gPad->Update();
	}

	// -------------- Right --------------
	locCanvas->cd(0);
	if(locHist_L1bits_gtp!=NULL && locHist_L1bits_fp!=NULL)
	{
		TPad *pad = (TPad*)gDirectory->FindObjectAny("trigpad3");
		if(!pad) pad = new TPad("trigpad1", "", 0.66, 0.5, 1.0, 1.0);
		pad->Draw();
		pad->cd();

		gPad->SetTicks();
		gPad->SetGrid();
		gPad->SetLogy();

		double max_gtp = locHist_L1bits_gtp->GetMaximum();
		double max_fp  = locHist_L1bits_fp->GetMaximum();
		double max = (max_gtp>max_fp) ? max_gtp:max_fp;
		
		const int bin_number = 9;
		const char *bin_label[bin_number] = {"Main (1)", "TOF (2)", "BCal (3)", "PS (4)", "TOF3 (6)", "FCal LED (3)", "BCal LED (9/10)", "Random (12)", "CTOF (6)"};
		TH1I *locHist_Trigger_GTP = new TH1I("locHist_Trigger_GTP", "L1 Trigger Bits", bin_number, 0, bin_number);
		TH1I *locHist_Trigger_FP = new TH1I("locHist_Trigger_FP", "", bin_number, 0, bin_number);
		// helper histograms for the individual columns
		TH1I *locHist_Trigger_alt1 = new TH1I("locHist_Trigger_alt1", "", bin_number, 0, bin_number);
		TH1I *locHist_Trigger_alt2 = new TH1I("locHist_Trigger_alt2", "", bin_number, 0, bin_number);
		TH1I *locHist_Trigger_alt3 = new TH1I("locHist_Trigger_alt3", "", bin_number, 0, bin_number);
		TH1I *locHist_Trigger_alt4 = new TH1I("locHist_Trigger_alt4", "", bin_number, 0, bin_number);
		for (int i=1; i <= bin_number; i++)
		  locHist_Trigger_GTP->GetXaxis()->SetBinLabel(i,bin_label[i-1]);

		// Main Trigger BCAL+FCAL: GTP Bit 1
		locHist_Trigger_GTP->Fill(0., locHist_L1bits_gtp->GetBinContent(1));
		// Main Trigger TOF: GTP Bit 2
		locHist_Trigger_GTP->Fill(1., locHist_L1bits_gtp->GetBinContent(2));
		locHist_Trigger_alt1->Fill(1., locHist_L1bits_gtp->GetBinContent(2));
		// BCAL Trigger: GTP Bit 3
		locHist_Trigger_GTP->Fill(2., locHist_L1bits_gtp->GetBinContent(3));
		// PS Trigger: GTP Bit 4
		locHist_Trigger_GTP->Fill(3., locHist_L1bits_gtp->GetBinContent(4));
		locHist_Trigger_alt2->Fill(3., locHist_L1bits_gtp->GetBinContent(4));
		// TOF3 Trigger: GTP Bit 6
		locHist_Trigger_GTP->Fill(4., locHist_L1bits_gtp->GetBinContent(6));

		// FCAL LED: FP Bit 3
		locHist_Trigger_FP->Fill(5., locHist_L1bits_fp->GetBinContent(3));
		locHist_Trigger_alt3->Fill(5., locHist_L1bits_fp->GetBinContent(3));

                // Add these bits.
		// BCAL LED: FP Bit 9
		locHist_Trigger_FP->Fill(6., locHist_L1bits_fp->GetBinContent(9));
		// BCAL LED: FP Bit 10
		locHist_Trigger_FP->Fill(6., locHist_L1bits_fp->GetBinContent(10));

		// Random Trigger: FP Bit 12
		locHist_Trigger_FP->Fill(7., locHist_L1bits_fp->GetBinContent(12));
		locHist_Trigger_alt4->Fill(7., locHist_L1bits_fp->GetBinContent(12));
		// CTOF: FP Bit 6
		locHist_Trigger_FP->Fill(8., locHist_L1bits_fp->GetBinContent(6));

		locHist_Trigger_GTP->SetFillColor(kOrange);
		locHist_Trigger_GTP->SetStats(0);
		locHist_Trigger_GTP->GetXaxis()->LabelsOption("v");
		locHist_Trigger_GTP->GetXaxis()->SetLabelSize(0.06);
		locHist_Trigger_GTP->GetYaxis()->SetRangeUser(1.0, max*2.0);
		locHist_Trigger_GTP->Draw("hist");
		locHist_Trigger_alt1->Draw("hist same");

		locHist_Trigger_FP->SetFillColor(kRed-4);
		locHist_Trigger_FP->Draw("hist same");
		locHist_Trigger_alt2->Draw("hist same");
		locHist_Trigger_alt3->Draw("hist same");
		locHist_Trigger_alt4->Draw("hist same");

		gPad->SetBottomMargin(0.25);
		gPad->RedrawAxis();

		TLegend *legend_gtp = new TLegend(0.5,0.85,0.7,0.9);
		TLegend *legend_fp  = new TLegend(0.7,0.85,0.9,0.9);
		legend_gtp->AddEntry(locHist_Trigger_GTP,"GTP","f");
		legend_fp->AddEntry(locHist_Trigger_FP,"FP","f");
		legend_gtp->Draw();
		legend_fp->Draw();

	}

	locCanvas->cd(0);
	if(locHist_L1livetime!=NULL)
	{
		TPad *pad = (TPad*)gDirectory->FindObjectAny("trigpad4");
		if(!pad) pad = new TPad("trigpad4", "", 0.66, 0.0, 1.0, 0.5);
		pad->Draw();
		pad->cd();

		gPad->SetTicks();
		gPad->SetGrid();

		locHist_L1livetime->SetLineColor(kGreen-3);
		locHist_L1livetime->SetFillColor(kGreen);
		locHist_L1livetime->SetFillStyle(3001);
		locHist_L1livetime->Draw();
		gPad->SetLogy();
		gPad->Update();
		
		// Move stats box
		TPaveStats *ps = (TPaveStats*)locHist_L1livetime->FindObject("stats");
		if(ps){
			ps->SetX1NDC(0.15);
			ps->SetX2NDC(0.45);
			ps->SetY1NDC(0.65);
			ps->SetY2NDC(0.85);
			ps->Draw();
		}
	}
}
