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
		locCanvas = new TCanvas("Trigger", "Trigger", 1200, 900); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 2);
	
	TLatex latex;
	latex.SetTextSize(0.04);
	char str[256];

	locCanvas->cd(1);
	if(locHist_L1bits_gtp!=NULL && locHist_L1bits_fp!=NULL)
	{
		gPad->SetTicks();
		gPad->SetGrid();
		gPad->SetLogy();

		double max_gtp = locHist_L1bits_gtp->GetMaximum();
		double max_fp  = locHist_L1bits_fp->GetMaximum();
		double max = (max_gtp>max_fp) ? max_gtp:max_fp;

		const int bin_number = 8;
		const char *bin_label[bin_number] = {"Main (1)", "BCal (3)", "PS (4)", "FCal LED (3)", "ECal LED (5)", "ECal #alpha (6)", "Random (12)", "DIRC LED (15)"};
		TH1I *locHist_Trigger_GTP = new TH1I("locHist_Trigger_GTP", "L1 Trigger Bits", bin_number, 0, bin_number);
		TH1I *locHist_Trigger_FP = new TH1I("locHist_Trigger_FP", "", bin_number, 0, bin_number);
		for (int i=1; i <= bin_number; i++)
		  locHist_Trigger_GTP->GetXaxis()->SetBinLabel(i,bin_label[i-1]);

		// Main Trigger BCAL+FCAL: GTP Bit 1
		locHist_Trigger_GTP->Fill(0., locHist_L1bits_gtp->GetBinContent(1));
		// BCAL Trigger: GTP Bit 3
		locHist_Trigger_GTP->Fill(1., locHist_L1bits_gtp->GetBinContent(3));
		// PS Trigger: GTP Bit 4
		locHist_Trigger_GTP->Fill(2., locHist_L1bits_gtp->GetBinContent(4));

		// FCAL LED: FP Bit 3
		locHist_Trigger_FP->Fill(3., locHist_L1bits_fp->GetBinContent(3));

		// ECAL LED: FP Bit 5
		locHist_Trigger_FP->Fill(4., locHist_L1bits_fp->GetBinContent(5));
		// ECAL alpha: FP Bit 6
		locHist_Trigger_FP->Fill(5., locHist_L1bits_fp->GetBinContent(6));

		// Random Trigger: FP Bit 12
		locHist_Trigger_FP->Fill(6., locHist_L1bits_fp->GetBinContent(12));
		// DIRC LED: FP Bit 15
		locHist_Trigger_FP->Fill(7., locHist_L1bits_fp->GetBinContent(15));

		locHist_Trigger_GTP->SetFillColor(kOrange);
		locHist_Trigger_GTP->SetStats(0);
		locHist_Trigger_GTP->GetXaxis()->LabelsOption("v");
		locHist_Trigger_GTP->GetXaxis()->SetLabelSize(0.06);
		locHist_Trigger_GTP->GetYaxis()->SetRangeUser(1.0, max*2.0);
		locHist_Trigger_GTP->Draw("hist");

		locHist_Trigger_FP->SetFillColor(kRed-4);
		locHist_Trigger_FP->Draw("hist same");

		gPad->SetBottomMargin(0.26);
		gPad->RedrawAxis();

		TLegend *legend_gtp = new TLegend(0.5,0.85,0.7,0.9);
		TLegend *legend_fp  = new TLegend(0.7,0.85,0.9,0.9);
		legend_gtp->AddEntry(locHist_Trigger_GTP,"GTP","f");
		legend_fp->AddEntry(locHist_Trigger_FP,"FP","f");
		legend_gtp->Draw();
		legend_fp->Draw();

	}

	// Hadronic trigger rate stats
	locCanvas->cd(2);
	if(locHist_NumTriggers){
		TLatex latex;
		latex.SetTextSize(0.05);
		latex.SetTextAlign(31);

		latex.DrawLatex(0.5, 0.9, "trig 1");
		latex.DrawLatex(0.7, 0.9, "trig 3");		
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

		// trig 3
		sprintf(str, "%4.3g", h->GetBinContent(3,1));
		latex.DrawLatex(0.65, 0.725, str);
		sprintf(str, "%4.3g", h->GetBinContent(3,3));
		latex.DrawLatex(0.65, 0.500, str);
		sprintf(str, "%4.3g", h->GetBinContent(3,4));
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

		// trig 3
		double Ntrig3 = h->GetBinContent(3,1);
		sprintf(str, "(%4.1f%%)", Ntrig3/all_trigs*100.0);
		latex.DrawLatex(0.65, 0.675, str);
		latex2.DrawLatex(0.65, 0.635, "of all trigs");
		sprintf(str, "(%4.1f%%)", h->GetBinContent(3,3)/Ntrig3*100.0);
		latex.DrawLatex(0.65, 0.450, str);
		latex2.DrawLatex(0.65, 0.410, "of trig 3's");
		sprintf(str, "(%4.1f%%)", h->GetBinContent(3,4)/Ntrig3*100.0);
		latex.DrawLatex(0.65, 0.225, str);
		latex2.DrawLatex(0.65, 0.185, "of trig 3's");

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
	

	locCanvas->cd(3);
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

	locCanvas->cd(4);
	if(locHist_L1livetime!=NULL)
	{
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
