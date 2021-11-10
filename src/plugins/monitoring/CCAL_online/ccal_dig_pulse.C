// hnamepath: /ccal/digOcc2D
// hnamepath: /ccal/digN
// hnamepath: /ccal/digPeak
// hnamepath: /ccal/digIntVsPeak

{
	
	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("ccal");
	if(dir) dir->cd();
	
	TH2F* digOcc2D   = (TH2F*)gDirectory->FindObjectAny("digOcc2D");
	TH1I* digN       = (TH1I*)gDirectory->FindObjectAny("digN");
	TH1I* digPeak   = (TH1I*)gDirectory->FindObjectAny("digPeak");
	TH2I* digIntVsPeak = (TH2I*)gDirectory->FindObjectAny("digIntVsPeak");
	
	double nEvents = ( digN ? digN->GetEntries() : 0 );
	
	if(gPad == NULL){
		
		TCanvas *c1 = new TCanvas("c1", "CCAL Monitor", 800, 800);
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	
	if( !gPad ) return;
	TCanvas* c1 = gPad->GetCanvas();
	c1->Divide(2, 2);
	
	if( digN ){
		
		digN->SetStats(0);
		digN->SetFillColor(kBlue);
		c1->cd(1);
		digN->Draw();
	}
	
	if( digOcc2D ){
		
		TH2F* digOcc2DAvg = (TH2F*)digOcc2D->Clone("digOcc2DAvg");
		digOcc2DAvg->SetTitle("CCAL Pulse Occupancy per Event");
		
		for( int x = 1; x <= digOcc2DAvg->GetNbinsX(); ++x ){
			for( int y = 1; y <= digOcc2DAvg->GetNbinsY(); ++y ){
				
				digOcc2DAvg->SetBinContent(x, y, 
					digOcc2DAvg->GetBinContent(x, y) / nEvents);
			}
		}
		
		digOcc2DAvg->SetStats(0);
		c1->cd(2);
		digOcc2DAvg->Draw("colz");
	}
	
	if( digPeak ){
		
		digPeak->SetStats(0);
		digPeak->GetXaxis()->SetTitle("Pulse Amplitude [ADC Counts]");
		digPeak->GetXaxis()->SetRangeUser(0,4096);
		digPeak->GetXaxis()->SetLabelSize(0.03);
		digPeak->GetYaxis()->SetLabelSize(0.03);
		digPeak->SetFillColor(kBlue);
		
		TPad *p3 = (TPad*)c1->cd(3);
		p3->SetLogy();
		c1->cd(3);
		digPeak->Draw();
	}
	
	if( digIntVsPeak ){
		
		digIntVsPeak->SetStats(0);
		digIntVsPeak->GetYaxis()->SetRangeUser(0,30000);
		digIntVsPeak->GetXaxis()->SetLabelSize(0.03);
		digIntVsPeak->GetYaxis()->SetTitleOffset(1.55);
		digIntVsPeak->GetYaxis()->SetLabelSize(0.025);
		
		TPad *p4 = (TPad*)c1->cd(4);
		p4->SetLogz();
		//c1->cd(4);
		digIntVsPeak->Draw("colz");
	}
	
}
