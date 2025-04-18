// hnamepath: /highlevel/F1TDC_fADC_tdiff
//
// e-mail: davidl@jlab.org
// e-mail: staylor@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org
//

{
	TDirectory* locCurrentDir = gDirectory;

	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
	TH2* NumHighLevelObjects = (TH2*)gDirectory->Get("NumHighLevelObjects");
	TH2* F1TDC_fADC_tdiff = (TH2*)gDirectory->Get("F1TDC_fADC_tdiff");

	//Get/Make Canvas
	TCanvas *c1 = NULL;
	if(TVirtualPad::Pad() == NULL)
		c1 = new TCanvas("NumHighLevelObjects", "NumHighLevelObjects", 1200, 900); //for testing
	else
		c1 = gPad->GetCanvas();

	c1->Divide(1,2);
	
	if(F1TDC_fADC_tdiff){
		c1->cd(1);
		gPad->SetTicks();
		gPad->SetGrid();
		gPad->SetLogz();
 		gPad->SetBottomMargin(0.25);
 		gPad->SetLeftMargin(0.08);
 		gPad->SetRightMargin(0.12);
		F1TDC_fADC_tdiff->GetXaxis()->SetLabelSize(0.06);
		F1TDC_fADC_tdiff->GetYaxis()->SetLabelSize(0.05);
		F1TDC_fADC_tdiff->GetZaxis()->SetLabelSize(0.05);
		F1TDC_fADC_tdiff->GetXaxis()->SetLabelOffset(0.01);
		F1TDC_fADC_tdiff->GetYaxis()->SetTitleSize(0.045);
		F1TDC_fADC_tdiff->GetYaxis()->SetTitleOffset(0.7);
		F1TDC_fADC_tdiff->GetXaxis()->SetRangeUser(1,49);
		F1TDC_fADC_tdiff->LabelsOption("V","X");
		F1TDC_fADC_tdiff->SetStats(0);
		F1TDC_fADC_tdiff->Draw("COLZ");

		c1->cd(2);
		TH2* F1TDC_fADC_tdiff_bottom = (TH2*)F1TDC_fADC_tdiff->Clone();
		
		gPad->SetTicks();
		gPad->SetGrid();
		gPad->SetLogz();
 		gPad->SetBottomMargin(0.25);
 		gPad->SetLeftMargin(0.08);
 		gPad->SetRightMargin(0.12);
		F1TDC_fADC_tdiff_bottom->GetXaxis()->SetRangeUser(50,99);
		F1TDC_fADC_tdiff_bottom->SetStats(0);
		F1TDC_fADC_tdiff_bottom->Draw("COLZ");
	}

}

