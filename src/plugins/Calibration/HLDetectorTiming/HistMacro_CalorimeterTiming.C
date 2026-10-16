// hnamepath: /HLDetectorTiming/Physics Triggers/BCAL/BCALHit ADC time
// hnamepath: /HLDetectorTiming/Physics Triggers/BCAL/BCALHit TDC time
// hnamepath: /HLDetectorTiming/Physics Triggers/BCAL/BCALHit Downstream Per Channel TDC-ADC Hit Time
// hnamepath: /HLDetectorTiming/Physics Triggers/BCAL/BCALHit Upstream Per Channel TDC-ADC Hit Time
// hnamepath: /HLDetectorTiming/Physics Triggers/FCAL/FCALHit time
// hnamepath: /HLDetectorTiming/Physics Triggers/ECAL/ECALHit time

{
	//Goto Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("HLDetectorTiming");
	if(!locDirectory)
		return;
	locDirectory->cd();


	//Get Histograms
 	TH1I* BCAL_ADC_Timing = (TH1I*)gDirectory->Get("Physics Triggers/BCAL/BCALHit ADC time");
 	TH1I* BCAL_TDC_Timing = (TH1I*)gDirectory->Get("Physics Triggers/BCAL/BCALHit TDC time");
 	TH1I* BCAL_TDC_ADC_DS_Timing = (TH1I*)gDirectory->Get("Physics Triggers/BCAL/BCALHit Downstream Per Channel TDC-ADC Hit Time");
 	TH1I* BCAL_TDC_ADC_US_Timing = (TH1I*)gDirectory->Get("Physics Triggers/BCAL/BCALHit Upstream Per Channel TDC-ADC Hit Time");
 	TH1I* FCAL_ADC_Timing = (TH1I*)gDirectory->Get("Physics Triggers/FCAL/FCALHit time");
 	TH1I* ECAL_ADC_Timing = (TH1I*)gDirectory->Get("Physics Triggers/ECAL/ECALHit time");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("OnlineCalorimeter", "OnlineCalorimeter", 1200, 800); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(3, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(FCAL_ADC_Timing != NULL)
	{
	    FCAL_ADC_Timing->GetXaxis()->SetRangeUser(-200,200);
	    FCAL_ADC_Timing->Draw();
	    FCAL_ADC_Timing->SetFillColor(kGray);
	    locCanvas->Update();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No FCAL ADC hits!");
	  text->Draw();
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(BCAL_ADC_Timing != NULL)
	{
	    BCAL_ADC_Timing->GetXaxis()->SetRangeUser(-200,200);
	    BCAL_ADC_Timing->Draw();
	    BCAL_ADC_Timing->SetFillColor(kGray);
	    locCanvas->Update();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No BCAL ADC hits!");
	  text->Draw();
	}

	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	if(BCAL_TDC_Timing != NULL)
	{
	    BCAL_TDC_Timing->GetXaxis()->SetRangeUser(-200,200);
	    BCAL_TDC_Timing->Draw();
	    BCAL_TDC_Timing->SetFillColor(kGray);
	    locCanvas->Update();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No BCAL TDC hits!");
	  text->Draw();
	}


	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	if(ECAL_ADC_Timing != NULL)
	{
	    ECAL_ADC_Timing->Rebin(4);
	    ECAL_ADC_Timing->GetXaxis()->SetRangeUser(-200,200);
	    ECAL_ADC_Timing->Draw();
	    ECAL_ADC_Timing->SetFillColor(kGray);
	    locCanvas->Update();
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No ECAL ADC hits!");
	  text->Draw();
	}


	locCanvas->cd(5);
	gPad->SetTicks();
	gPad->SetGrid();
	if(BCAL_TDC_ADC_DS_Timing != NULL)
	{
	    BCAL_TDC_ADC_DS_Timing->Draw("COLZ");
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No BCAL ADC and TDC hits!");
	  text->Draw();
	}


	locCanvas->cd(6);
	gPad->SetTicks();
	gPad->SetGrid();
	if(BCAL_TDC_ADC_US_Timing != NULL)
	{
	    BCAL_TDC_ADC_US_Timing->Draw("COLZ");
	}
	else{
	  TPaveText *text = new TPaveText(0.1, 0.4, 0.9, 0.6);
	  text->AddText("No BCAL ADC and TDC hits!");
	  text->Draw();
	}


}

