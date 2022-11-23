
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /FMWPC/fmwpc_num_events
// hnamepath: /FMWPC/fmwpc_occ_layer_1
// hnamepath: /FMWPC/fmwpc_occ_layer_2
// hnamepath: /FMWPC/fmwpc_occ_layer_3
// hnamepath: /FMWPC/fmwpc_occ_layer_4
// hnamepath: /FMWPC/fmwpc_occ_layer_5
// hnamepath: /FMWPC/fmwpc_occ_layer_6

{
	// RootSpy saves the current directory and style before
	// calling the macro and restores it after so it is OK to
	// change them and not change them back.
        TDirectory *savedir = gDirectory;
	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("FMWPC");
	if(dir) dir->cd();

	TH1F *fmwpc_occ_layer_1 = (TH1F*)gDirectory->FindObjectAny("fmwpc_occ_layer_1");
	TH1F *fmwpc_occ_layer_2 = (TH1F*)gDirectory->FindObjectAny("fmwpc_occ_layer_2");
	TH1F *fmwpc_occ_layer_3 = (TH1F*)gDirectory->FindObjectAny("fmwpc_occ_layer_3");
	TH1F *fmwpc_occ_layer_4 = (TH1F*)gDirectory->FindObjectAny("fmwpc_occ_layer_4");
	TH1F *fmwpc_occ_layer_5 = (TH1F*)gDirectory->FindObjectAny("fmwpc_occ_layer_5");
	TH1F *fmwpc_occ_layer_6 = (TH1F*)gDirectory->FindObjectAny("fmwpc_occ_layer_6");

	double Nevents = 1.0;
	TH1I *fmwpc_num_events = (TH1I*)gDirectory->FindObjectAny("fmwpc_num_events");
	if(fmwpc_num_events) Nevents = (double)fmwpc_num_events->GetBinContent(1);

	// Just for testing
	if(gPad == NULL){
		TCanvas *c1 = new TCanvas("c1");
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	if(!gPad) {savedir->cd(); return;}

	TCanvas *c1 = gPad->GetCanvas();
	c1->Divide(3,2);
	
	TVirtualPad *pad1 = c1->cd(1);
	pad1->SetTicks();
	//pad1->SetLogz();
	pad1->SetLeftMargin(0.15);
	pad1->SetRightMargin(0.15);
	char hname[256];
	if(fmwpc_occ_layer_1){
	        sprintf(hname, "fmwpc_norm_layer_1");
	        TH1 *fmwpc_norm_layer_1 = (TH1*)fmwpc_occ_layer_1->Clone(hname);
		fmwpc_norm_layer_1->SetStats(0);
		fmwpc_norm_layer_1->Scale(1./Nevents);
		fmwpc_norm_layer_1->GetXaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_1->GetYaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_1->GetXaxis()->SetTitleSize(0.05);
		fmwpc_norm_layer_1->Draw("h");
	}

	TVirtualPad *pad2 = c1->cd(2);
	pad2->SetTicks();
	//pad2->SetLogz();
	pad2->SetLeftMargin(0.15);
	pad2->SetRightMargin(0.15);
	if(fmwpc_occ_layer_2){
	        sprintf(hname, "fmwpc_norm_layer_2");
	        TH1 *fmwpc_norm_layer_2 = (TH1*)fmwpc_occ_layer_2->Clone(hname);
		fmwpc_norm_layer_2->SetStats(0);
		fmwpc_norm_layer_2->Scale(1./Nevents);
		fmwpc_norm_layer_2->GetXaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_2->GetYaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_2->GetXaxis()->SetTitleSize(0.05);
		fmwpc_norm_layer_2->Draw("h");
	}

	TVirtualPad *pad3 = c1->cd(3);
	pad3->SetTicks();
	//pad3->SetLogz();
	pad3->SetLeftMargin(0.15);
	pad3->SetRightMargin(0.15);
	if(fmwpc_occ_layer_3){
	        sprintf(hname, "fmwpc_norm_layer_3");
	        TH1 *fmwpc_norm_layer_3 = (TH1*)fmwpc_occ_layer_3->Clone(hname);
		fmwpc_norm_layer_3->SetStats(0);
		fmwpc_norm_layer_3->Scale(1./Nevents);
		fmwpc_norm_layer_3->GetXaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_3->GetYaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_3->GetXaxis()->SetTitleSize(0.05);
		fmwpc_norm_layer_3->Draw("h");
	}

	TVirtualPad *pad4 = c1->cd(4);
	pad4->SetTicks();
	//pad4->SetLogz();
	pad4->SetLeftMargin(0.15);
	pad4->SetRightMargin(0.15);
	if(fmwpc_occ_layer_4){
	        sprintf(hname, "fmwpc_norm_layer_4");
	        TH1 *fmwpc_norm_layer_4 = (TH1*)fmwpc_occ_layer_4->Clone(hname);
		fmwpc_norm_layer_4->SetStats(0);
		fmwpc_norm_layer_4->Scale(1./Nevents);
		fmwpc_norm_layer_4->GetXaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_4->GetYaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_4->GetXaxis()->SetTitleSize(0.05);
		fmwpc_norm_layer_4->Draw("h");
	}

	TVirtualPad *pad5 = c1->cd(5);
	pad5->SetTicks();
	//pad5->SetLogz();
	pad5->SetLeftMargin(0.15);
	pad5->SetRightMargin(0.15);
	if(fmwpc_occ_layer_5){
	        sprintf(hname, "fmwpc_norm_layer_5");
	        TH1 *fmwpc_norm_layer_5 = (TH1*)fmwpc_occ_layer_5->Clone(hname);
		fmwpc_norm_layer_5->SetStats(0);
		fmwpc_norm_layer_5->Scale(1./Nevents);
		fmwpc_norm_layer_5->GetXaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_5->GetYaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_5->GetXaxis()->SetTitleSize(0.05);
		fmwpc_norm_layer_5->Draw("h");
	}

	TVirtualPad *pad6 = c1->cd(6);
	pad6->SetTicks();
	//pad6->SetLogz();
	pad6->SetLeftMargin(0.15);
	pad6->SetRightMargin(0.15);
	if(fmwpc_occ_layer_6){
	        sprintf(hname, "fmwpc_norm_layer_6");
	        TH1 *fmwpc_norm_layer_6 = (TH1*)fmwpc_occ_layer_6->Clone(hname);
		fmwpc_norm_layer_6->SetStats(0);
		fmwpc_norm_layer_6->Scale(1./Nevents);
		fmwpc_norm_layer_6->GetXaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_6->GetYaxis()->SetLabelSize(0.05);
		fmwpc_norm_layer_6->GetXaxis()->SetTitleSize(0.05);
		fmwpc_norm_layer_6->Draw("h");
	}

#ifdef ROOTSPY_MACROS
	// ------ The following is used by RSAI --------
	if( rs_GetFlag("Is_RSAI")==1 ){
		auto min_events = 5*rs_GetFlag("MIN_EVENTS_RSAI");
		if( min_events < 1 ) min_events = 5E5;
		if( Nevents >= min_events ) {
			cout << "FMWPC Flagging AI check after " << Nevents << " events (>=" << min_events << ")" << endl;
			rs_SavePad("FMWPC_occupancy", 0);
			rs_ResetAllMacroHistos("//FMWPC_occupancy");
		}
	}
#endif

}


