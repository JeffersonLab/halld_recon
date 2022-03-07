// hnamepath: /PS_timing/PSC_tdcadcTimeDiffVsID
// hnamepath: /PS_timing/PSCRF_tdcTimeDiffVsID
// hnamepath: /PS_timing/PSRF_adcTimeDiffVsID
// hnamepath: /PS_timing/TAGHRF_tdcTimeDiffVsID

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("PS_timing");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//Get Histograms
    TH2I* PSC_TDC_ADC_Timing     = (TH2I*)gDirectory->Get("PSC_tdcadcTimeDiffVsID");
    TH2I* PSC_TDC_RF_Timing      = (TH2I*)gDirectory->Get("PSCRF_tdcTimeDiffVsID");
    TH2I* PS_ADC_RF_Timing       = (TH2I*)gDirectory->Get("PSRF_adcTimeDiffVsID");
    TH2I* TAGH_TDC_RF_Timing     = (TH2I*)gDirectory->Get("TAGHRF_tdcTimeDiffVsID");
    //TH2I* PSC_TDC_ADC_Timing     = (TH2I*)gDirectory->Get("PS_timing/PSC_tdcadcTimeDiffVsID");
    //TH2I* PSC_TDC_RF_Timing      = (TH2I*)gDirectory->Get("PS_timing/PSCRF_tdcTimeDiffVsID");
    //TH2I* PS_ADC_RF_Timing       = (TH2I*)gDirectory->Get("PS_timing/PSRF_adcTimeDiffVsID");
    //TH2I* TAGH_TDC_RF_Timing     = (TH2I*)gDirectory->Get("PS_timing/TAGHRF_tdcTimeDiffVsID");

    //Get/Make Canvas
    TCanvas *locCanvas = NULL;
    if(TVirtualPad::Pad() == NULL)
        locCanvas = new TCanvas("PSTiming", "PSTiming", 1200, 800); //for testing
    else
        locCanvas = gPad->GetCanvas();
    locCanvas->Divide(2, 2);

    //Draw
    locCanvas->cd(1);
    gPad->SetTicks();
    gPad->SetGrid();
    if(PSC_TDC_ADC_Timing != NULL)
    {
    	PSC_TDC_ADC_Timing->GetYaxis()->SetRangeUser(-15.,15.);
        PSC_TDC_ADC_Timing->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No PSC Hit Times");
        text->Draw();
    }

    locCanvas->cd(2);
    gPad->SetTicks();
    gPad->SetGrid();
    if(PS_ADC_RF_Timing != NULL)
    {
    	PS_ADC_RF_Timing->GetYaxis()->SetRangeUser(-70.,-30.);
        PS_ADC_RF_Timing->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No PS Hit Times");
        text->Draw();
    }

    locCanvas->cd(3);
    gPad->SetTicks();
    gPad->SetGrid();
    if(PSC_TDC_RF_Timing != NULL)
    {
    	PSC_TDC_RF_Timing->GetYaxis()->SetRangeUser(-70.,-30.);
        PSC_TDC_RF_Timing->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No PSC Hit Times");
        text->Draw();
    }

    locCanvas->cd(4);
    gPad->SetTicks();
    gPad->SetGrid();
    if(TAGH_TDC_RF_Timing != NULL)
    {
    	TAGH_TDC_RF_Timing->GetYaxis()->SetRangeUser(-20.,20.);
        TAGH_TDC_RF_Timing->Draw("colz");
    }
    else{
        TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
        text->AddText("No TAGH Hit Times");
        text->Draw();
    }
}

