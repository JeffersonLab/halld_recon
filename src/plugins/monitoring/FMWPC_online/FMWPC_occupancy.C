
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
	if(fmwpc_occ_layer_1){
		fmwpc_occ_layer_1->SetStats(0);
		fmwpc_occ_layer_1->Scale(1./Nevents);
		fmwpc_occ_layer_1->Draw();
	}

	TVirtualPad *pad2 = c1->cd(2);
	pad2->SetTicks();
	//pad2->SetLogz();
	pad2->SetLeftMargin(0.15);
	pad2->SetRightMargin(0.15);
	if(fmwpc_occ_layer_2){
		fmwpc_occ_layer_2->SetStats(0);
		fmwpc_occ_layer_2->Scale(1./Nevents);
		fmwpc_occ_layer_2->Draw();
	}

	TVirtualPad *pad3 = c1->cd(3);
	pad3->SetTicks();
	//pad3->SetLogz();
	pad3->SetLeftMargin(0.15);
	pad3->SetRightMargin(0.15);
	if(fmwpc_occ_layer_3){
		fmwpc_occ_layer_3->SetStats(0);
		fmwpc_occ_layer_3->Scale(1./Nevents);
		fmwpc_occ_layer_3->Draw();
	}

	TVirtualPad *pad4 = c1->cd(4);
	pad4->SetTicks();
	//pad4->SetLogz();
	pad4->SetLeftMargin(0.15);
	pad4->SetRightMargin(0.15);
	if(fmwpc_occ_layer_4){
		fmwpc_occ_layer_4->SetStats(0);
		fmwpc_occ_layer_4->Scale(1./Nevents);
		fmwpc_occ_layer_4->Draw();
	}

	TVirtualPad *pad5 = c1->cd(5);
	pad5->SetTicks();
	//pad5->SetLogz();
	pad5->SetLeftMargin(0.15);
	pad5->SetRightMargin(0.15);
	if(fmwpc_occ_layer_5){
		fmwpc_occ_layer_5->SetStats(0);
		fmwpc_occ_layer_5->Scale(1./Nevents);
		fmwpc_occ_layer_5->Draw();
	}

	TVirtualPad *pad6 = c1->cd(6);
	pad6->SetTicks();
	//pad6->SetLogz();
	pad6->SetLeftMargin(0.15);
	pad6->SetRightMargin(0.15);
	if(fmwpc_occ_layer_6){
		fmwpc_occ_layer_6->SetStats(0);
		fmwpc_occ_layer_6->Scale(1./Nevents);
		fmwpc_occ_layer_6->Draw();
	}

}


