
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /DAQ/daq_words_per_event

void daq_words_compare_by_crate(string fname1="hdeveio_scan_071562.root", string fname2="hdeveio_scan_071563.root")
{
	TFile *f1 = new TFile(fname1.c_str());
	TH1D *daq_words_per_event1 = (TH1D*)gROOT->FindObject("daq_words_per_event");
	if(!daq_words_per_event1){
		cout << "Can't find 'daq_words_per_event' histogram in" << fname1 << " !" << endl;
		return;
	}

	TFile *f2 = new TFile(fname2.c_str());
	TH1D *daq_words_per_event2 = (TH1D*)gROOT->FindObject("daq_words_per_event");
	if(!daq_words_per_event2){
		cout << "Can't find 'daq_words_per_event' histogram in" << fname2 << " !" << endl;
		return;
	}
	
	TCanvas *c1 = new TCanvas("c1", "", 1800, 800);
	c1->Draw();

	gPad->SetTopMargin(0.13);
	gPad->SetBottomMargin(0.3);
	gPad->SetLeftMargin(0.05);
	gPad->SetRightMargin(0.02);
	gPad->GetCanvas()->SetTicks();
	gPad->GetCanvas()->SetLogy();
	
	// Set max so maximum bin (not including total words bin
	// which should be highest) is at 55%
	
	double mid1 = daq_words_per_event1->GetMaximum();
	double mid2 = daq_words_per_event2->GetMaximum();
	double mid = mid1>mid2 ? mid1:mid2;
	double min = 1.0E0;
	double max = pow(10.0, (1.0/0.65)*(log10(mid)-log10(min)) + log10(min));
	cout << "mid1="<<mid1<< " mid2="<<mid2<<" mid="<<mid<<" max="<<max<<endl;
	
	daq_words_per_event1->SetStats(0);
	daq_words_per_event1->SetYTitle("32-bit words per EVIO event block");
	daq_words_per_event1->GetYaxis()->SetTitleOffset(0.70);
	daq_words_per_event1->GetYaxis()->SetRangeUser(min, max);
	daq_words_per_event1->SetFillColor(kBlue);
	daq_words_per_event1->SetBarWidth(0.43);
	daq_words_per_event1->SetBarOffset(0.07);
	daq_words_per_event1->Draw("bar");

	daq_words_per_event2->SetFillColor(kRed);
	daq_words_per_event2->SetBarWidth(0.43);
	daq_words_per_event2->SetBarOffset(0.5);
	daq_words_per_event2->Draw("same bar");
	
	double ylab = max;
	
	TLatex latex;
	latex.SetTextSize(0.030);
	latex.SetTextAngle(90.0);
	
	Int_t Nbins = daq_words_per_event1->GetNbinsX();
	double Nword_tot1 = daq_words_per_event1->Integral(1, Nbins);
	double Nword_tot2 = daq_words_per_event2->Integral(1, Nbins);
	double sum = 0;
	for(int ibin=1; ibin<=Nbins; ibin++){
		
		double x = daq_words_per_event1->GetXaxis()->GetBinCenter(ibin);
		double Nwords = daq_words_per_event1->GetBinContent(ibin);
		double percent = 100.0 * Nwords / Nword_tot1;
		
		if(Nwords == 0) continue;
		
		sum += Nwords;

		char str[256];
		sprintf(str, "%4.1f%%", percent);
		if(ibin < Nbins-2) sprintf(str, "%s _{(%3.1f%% total)}", str, 100.0*sum/Nword_tot1);
		latex.SetTextAlign(32);
		latex.SetTextColor(kBlue);
		latex.SetTextSize(0.030);
		latex.DrawLatex( x, ylab, str);

		x = daq_words_per_event2->GetXaxis()->GetBinCenter(ibin);
		Nwords = daq_words_per_event2->GetBinContent(ibin);
		percent = 100.0 * Nwords / Nword_tot2;
		sprintf(str, "%4.1f%%", percent);
		latex.SetTextAlign(12);
		latex.SetTextColor(kRed);
		latex.SetTextSize(0.020);
		latex.DrawLatex( x, ylab, str);
	}
	
	// Print avg. event sizes
	double event_size1 = Nword_tot1*4.0/1024.0;
	double event_size2 = Nword_tot2*4.0/1024.0;
	char es_str1[256];
	char es_str2[256];
	sprintf(es_str1, "%4.1f kB/evt", event_size1);
	sprintf(es_str2, "%4.1f kB/evt", event_size2);
	cout << "event_size1: " << event_size1 << endl;
	cout << "event_size2: " << event_size2 << endl;
	//double x = daq_words_per_event1->GetXaxis()->GetBinCenter(Nbins-1);
	//double dx = daq_words_per_event1->GetXaxis()->GetBinWidth(1);
	//double y = pow(10.0, 0.55*(log10(mid)-log10(min)) + log10(mid));
	//latex.SetTextAngle(0);
	//latex.SetTextAlign(12);
	//latex.SetTextSize(0.020);
	//latex.SetTextColor(kBlue);
	//latex.DrawLatex( x-dx/1.5, y, es_str1);
	//latex.SetTextAlign(12);
	//latex.SetTextSize(0.020);
	//latex.SetTextColor(kRed);
	//latex.DrawLatex( x, y, es_str2);
	
	
	double y_one_percent = 0.01*Nword_tot1;
	TLine *lin = new TLine(0.0, y_one_percent, (double)Nbins, y_one_percent);
	lin->SetLineColor(kMagenta);
	lin->Draw();

	cout << "            sum: " << sum << endl;
	cout << "      Nword_tot: " << Nword_tot1 << endl;
	cout << "        missing: " << (Nword_tot1-sum)/Nword_tot1*100.0 << "%" << endl;
	cout << "     Event Size: " << (double)Nword_tot1*4.0/1024.0 << " kB/event" << endl;

	latex.SetTextAngle(0);
	latex.SetTextAlign(11);
	latex.SetTextSize(0.025);
	latex.SetTextColor(kBlue);
	ylab = pow(10.0, 1.17*log10(max/min) + log10(min));
	latex.DrawLatex(60.0, ylab, fname1.c_str());
	latex.DrawLatex(80.0, ylab, es_str1);

	latex.SetTextColor(kRed);
	ylab = pow(10.0, 1.12*log10(max/min) + log10(min));
	latex.DrawLatex(60.0, ylab, fname2.c_str());
	latex.DrawLatex(80.0, ylab, es_str2);

	c1->SaveAs("daq_words_compare_by_crate.png");
	c1->SaveAs("daq_words_compare_by_crate.pdf");
	c1->SaveAs("daq_words_compare_by_crate_src.C");
}

