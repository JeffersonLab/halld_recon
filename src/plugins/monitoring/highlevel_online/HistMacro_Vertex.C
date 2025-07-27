// hnamepath: /highlevel/EventVertexZ
// hnamepath: /highlevel/EventVertexYVsX
// hnamepath: /highlevel/EventVertexXVsZ
// hnamepath: /highlevel/EventVertexYVsZ
//
// e-mail: davidl@jlab.org
// e-mail: staylor@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org
//

//EventVertexZ
//EventVertexYVsX
//EventVertexXVsZ
//EventVertexYVsZ

{
	TDirectory *locTopDirectory = gDirectory;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(!locDirectory)
		return;
	locDirectory->cd();

	TH1* locHist_EventVertexZ = (TH1*)gDirectory->Get("EventVertexZ");
	TH2* locHist_EventVertexYVsX = (TH2*)gDirectory->Get("EventVertexYVsX");
	TH2* locHist_EventVertexXVsZ = (TH2*)gDirectory->Get("EventVertexXVsZ");
	TH2* locHist_EventVertexYVsZ = (TH2*)gDirectory->Get("EventVertexYVsZ");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("Vertex", "Vertex", 696, 472); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 2);

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_EventVertexZ != NULL)
	{
		locHist_EventVertexZ->SetStats(0);
		locHist_EventVertexZ->GetXaxis()->SetTitleSize(0.05);
		locHist_EventVertexZ->GetYaxis()->SetTitleSize(0.05);
		locHist_EventVertexZ->GetXaxis()->SetLabelSize(0.05);
		locHist_EventVertexZ->GetYaxis()->SetLabelSize(0.035);
		locHist_EventVertexZ->GetXaxis()->SetRangeUser(48,88);
		locHist_EventVertexZ->Draw();
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_EventVertexYVsX != NULL)
	{
		locHist_EventVertexYVsX->SetStats(0);
		locHist_EventVertexYVsX->GetXaxis()->SetTitleSize(0.05);
		locHist_EventVertexYVsX->GetYaxis()->SetTitleSize(0.045);
		locHist_EventVertexYVsX->GetXaxis()->SetLabelSize(0.05);
		locHist_EventVertexYVsX->GetYaxis()->SetLabelSize(0.05);
		//locHist_EventVertexYVsX->Rebin2D(2,2);
		locHist_EventVertexYVsX->GetXaxis()->SetRangeUser(-2,2);
		locHist_EventVertexYVsX->GetYaxis()->SetRangeUser(-2,2);
		locHist_EventVertexYVsX->Draw("colz");
		gPad->Update();
	}

	locCanvas->cd(3);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_EventVertexXVsZ != NULL)
	{
		locHist_EventVertexXVsZ->SetStats(0);
		locHist_EventVertexXVsZ->GetXaxis()->SetTitleSize(0.05);
		locHist_EventVertexXVsZ->GetYaxis()->SetTitleSize(0.045);
		locHist_EventVertexXVsZ->GetXaxis()->SetLabelSize(0.05);
		locHist_EventVertexXVsZ->GetYaxis()->SetLabelSize(0.05);
		locHist_EventVertexXVsZ->Draw("colz");
		//gPad->SetLogz();
		gPad->Update();
	}

	locCanvas->cd(4);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_EventVertexYVsZ != NULL)
	{
		locHist_EventVertexYVsZ->SetStats(0);
		locHist_EventVertexYVsZ->GetXaxis()->SetTitleSize(0.05);
		locHist_EventVertexYVsZ->GetYaxis()->SetTitleSize(0.045);
		locHist_EventVertexYVsZ->GetXaxis()->SetLabelSize(0.05);
		locHist_EventVertexYVsZ->GetYaxis()->SetLabelSize(0.05);
		locHist_EventVertexYVsZ->Draw("colz");
		//gPad->SetLogz();
		gPad->Update();
	}



}
