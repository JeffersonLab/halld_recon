// hnamepath: /p2pi_preco/Custom_p2pi_hists/rhoDecPhipp
// hnamepath: /p2pi_preco/Custom_p2pi_hists/rhoDecPhipm
// hnamepath: /p2pi_preco/Custom_p2pi_hists/rhoDecPhimp
// hnamepath: /p2pi_preco/Custom_p2pi_hists/rhoDecPhimm
//
// e-mail: Derek.Glazier@glasgow.ac.uk
// e-mail: Peter.Hurck@glasgow.ac.uk
//

{
	TDirectory *locTopDirectory = gDirectory;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("p2pi_preco");
	if(!locDirectory)
		return;
	locDirectory->cd();

	//get CircMonitor histograms from file
	auto hist_rhoDecPhipp   = gDirectory->Get<TH1>("Custom_p2pi_hists/rhoDecPhipp");
	auto hist_rhoDecPhipm   = gDirectory->Get<TH1>("Custom_p2pi_hists/rhoDecPhipm");
	auto hist_rhoDecPhimp   = gDirectory->Get<TH1>("Custom_p2pi_hists/rhoDecPhimp");
	auto hist_rhoDecPhimm   = gDirectory->Get<TH1>("Custom_p2pi_hists/rhoDecPhimm");


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
	hist_rhoDecPhipp->SetLineColor(4);
	plotPhi(hist_rhoDecPhipp);
	locCanvas->cd(2);
	hist_rhoDecPhimm->SetLineColor(4);
	plotPhi(hist_rhoDecPhimm);
	locCanvas->cd(4);
	hist_rhoDecPhimp->SetLineColor(2);
	plotPhi(hist_rhoDecPhimp);
	locCanvas->cd(5);
	hist_rhoDecPhipm->SetLineColor(2);
	plotPhi(hist_rhoDecPhipm);


	//------------ calculate helicity asymmetry --------------
	//first combine same contributions
	hist_rhoDecPhipp->Add(hist_rhoDecPhimm);
	hist_rhoDecPhimp->Add(hist_rhoDecPhipm);

	cout<<" entries "<<hist_rhoDecPhipp->GetEntries()<<" "<<hist_rhoDecPhimp->GetEntries()<<endl;
	int nRebinIf=40000;
	int nSimpleFit=10000;
	auto sumEntries = hist_rhoDecPhipp->GetEntries() + hist_rhoDecPhimp->GetEntries();
	if( sumEntries < nRebinIf ){
	  hist_rhoDecPhipp->Rebin(2);
	  hist_rhoDecPhimp->Rebin(2);
	}
	//draw summed contributions
	locCanvas->cd(3);
	hist_rhoDecPhipp->SetTitle("Combined #phi distributions, used for asymmetry");
	hist_rhoDecPhipp->SetLineColor(4);
	hist_rhoDecPhipp->SetMarkerColor(4);
	hist_rhoDecPhipp->Draw();
	hist_rhoDecPhimp->SetLineColor(2);
	hist_rhoDecPhimp->SetMarkerColor(2);
	hist_rhoDecPhimp->Draw("same");

	//make asymmetry
	auto hist_A = hist_rhoDecPhipp->GetAsymmetry(hist_rhoDecPhimp);
	//draw asymmeytry
	locCanvas->cd(6);
	hist_A->SetTitle("#rho helicity asymmetry");
	hist_A->Draw("pe");

	//Define fit function dependeing on number of events
	TString funcForm;
	funcForm = sumEntries<nSimpleFit ? "[0] + [1]*sin(2*x[0])" : "[0]+ ([1]*sin(2*x[0]))/(1 + 0.5*[2]*cos(2*x[0])+[3]*cos(x[0]))";
	TF1 hfit2phi("x2phi",funcForm,-TMath::Pi(),TMath::Pi());
	hfit2phi.SetParameters(0,0.1,0,0,-0.3,0);
	hfit2phi.SetLineColor(2);
	hfit2phi.SetLineWidth(1);

	hist_A->Fit("x2phi","");
	hist_A->SetLineColor(1);
	hist_A->SetMarkerColor(1);

	float amp = hfit2phi.GetParameter(1);
	float amp_err = hfit2phi.GetParError(1);

	if(sumEntries<nSimpleFit){
	  sprintf(str, "SIMPLE Fit Amplitude = %f #pm %f", amp,amp_err);
	}
	else{
	  sprintf(str, "FULL Fit Amplitude = %f #pm %f", amp,amp_err);
	}

	latex.DrawLatex(-2, hist_A->GetMaximum(), str);

}
