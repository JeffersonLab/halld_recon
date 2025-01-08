// hnamepath: /highlevel/rhoDecPhipp
// hnamepath: /highlevel/rhoDecPhipm
// hnamepath: /highlevel/rhoDecPhimp
// hnamepath: /highlevel/rhoDecPhimm
//
// e-mail: Derek.Glazier@glasgow.ac.uk
// e-mail: Peter.Hurck@glasgow.ac.uk
//

{
	TDirectory *locTopDirectory = gDirectory;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//get CircMonitor histograms from file
	auto hist_rhoDecPhipp   = gDirectory->Get<TH1>("rhoDecPhipp");
	auto hist_rhoDecPhipm   = gDirectory->Get<TH1>("rhoDecPhipm");
	auto hist_rhoDecPhimp   = gDirectory->Get<TH1>("rhoDecPhimp");
	auto hist_rhoDecPhimm   = gDirectory->Get<TH1>("rhoDecPhimm");


	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL){
		locCanvas = new TCanvas("CircMonitor", "CircMonitor", 1200, 600); //for testing
	}
	else{
	  locCanvas = gPad->GetCanvas();
	}
	locCanvas->Divide(3, 2);

	TLatex latex;
	latex.SetTextSize(0.04);
	char str[256];


	//------------ phi histogram plotter --------------
	auto plotPhi=[](TH1* hist){
	  gPad->SetTicks();
	  gPad->SetGrid();
	  if(hist!= NULL)
	    {
	      hist->GetXaxis()->SetTitleSize(0.05);
	      hist->GetYaxis()->SetTitleSize(0.04);
	      hist->GetXaxis()->SetLabelSize(0.05);
	      hist->GetYaxis()->SetLabelSize(0.05);
	      hist->SetMinimum(0);
	      hist->SetStats(0);
	      hist->DrawCopy();
	      hist->Sumw2();
	    }
	};

	//Plot the 4 phi histograms
	locCanvas->cd(1);
	plotPhi(hist_rhoDecPhipp);
	locCanvas->cd(2);
	plotPhi(hist_rhoDecPhipm);
	locCanvas->cd(4);
	plotPhi(hist_rhoDecPhimm);
	locCanvas->cd(5);
	plotPhi(hist_rhoDecPhimp);


	//------------ calculate helicity asymmetry --------------
	//first combine same contributions
	hist_rhoDecPhipp->Add(hist_rhoDecPhimm);
	hist_rhoDecPhimp->Add(hist_rhoDecPhipm);

	//draw summed contributions
	locCanvas->cd(3);
	hist_rhoDecPhipp->SetTitle("Combined #phi distributions, used for asymmetry");
	hist_rhoDecPhipp->Draw();
	hist_rhoDecPhimp->SetLineColor(2);
	hist_rhoDecPhimp->Draw("same");

	//make asymmetry
	auto hist_A = hist_rhoDecPhipp->GetAsymmetry(hist_rhoDecPhimp);
	//draw asymmeytry
	locCanvas->cd(6);
	hist_A->SetTitle("#rho helicity asymmetry");
	hist_A->Draw("pe");

	TF1 hfit2phi("x2phi","[0]+ ([1]*cos(2*x[0]+[2])+[3]*cos(x[0]+[4]))/(1 + [5]*cos(2*x[0]+[6])+[7]*cos(x[0]+[8]))",-TMath::Pi(),TMath::Pi());
	hfit2phi.SetParameters(0,0,1,0,0.1,0);
	hist_A->Fit("x2phi","Q");
	//h1a->Draw("same");
	cout<<hist_A->GetYaxis()->GetXmax()<<endl;
	float amp = hfit2phi.GetParameter(1);
	float amp_err = hfit2phi.GetParError(1);
	sprintf(str, "Fit Amplitude = %f #pm %f", amp,amp_err);
	latex.DrawLatex(-2, hist_A->GetMaximum(), str);

}
