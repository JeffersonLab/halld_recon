// hnamepath: /ccal/comp_cxy
// hnamepath: /ccal/comp_fxy
// hnamepath: /ccal/comp_bcdt
// hnamepath: /ccal/comp_fcdt

{
	
	TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("ccal");
	if(dir) dir->cd();
	
	TH2F* comp_cxy  = (TH2F*)gDirectory->FindObjectAny("comp_cxy");
	TH2F* comp_fxy  = (TH2F*)gDirectory->FindObjectAny("comp_fxy");
	TH1F* comp_bcdt = (TH1F*)gDirectory->FindObjectAny("comp_bcdt");
	TH1F* comp_fcdt = (TH1F*)gDirectory->FindObjectAny("comp_fcdt");
	
	if(gPad == NULL){
		
		TCanvas *c1 = new TCanvas( "c1", "CCAL Compton Monitor", 800, 800 );
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	
	if( !gPad ) return;
	TCanvas* c1 = gPad->GetCanvas();
	c1->Divide(2, 2);
	
	if( comp_cxy ){
		
		comp_cxy->SetStats(0);
		comp_cxy->GetXaxis()->SetRangeUser(-12.,12.);
		comp_cxy->GetXaxis()->SetTitle("x_{ccal} [cm]");
		comp_cxy->GetXaxis()->SetTitleSize(0.045);
		comp_cxy->GetYaxis()->SetRangeUser(-12.,12.);
		comp_cxy->GetYaxis()->SetTitle("y_{ccal} [cm]");
		comp_cxy->GetYaxis()->SetTitleSize(0.045);
		
		//TPad *p1 = (TPad*)c1->cd(1);
		//p1->SetLogz(); p1->cd();
		c1->cd( 1 );
		comp_cxy->Draw("COLZ");
	}
	
	if( comp_fxy ){
		
		comp_fxy->SetStats(0);
		comp_cxy->GetYaxis()->SetRangeUser(-50.,50.);
		comp_fxy->GetXaxis()->SetTitle("x_{fcal} [cm]");
		comp_fxy->GetXaxis()->SetTitleSize(0.045);
		comp_cxy->GetYaxis()->SetRangeUser(-50.,50.);
		comp_fxy->GetYaxis()->SetTitle("y_{fcal} [cm]");
		comp_fxy->GetYaxis()->SetTitleSize(0.045);
		
		//TPad *p2 = (TPad*)c1->cd(2);
		//p1->SetLogz(); p1->cd();
		c1->cd( 2 );
		comp_fxy->Draw("COLZ");
	}
	
	if( comp_bcdt ){
		
		comp_bcdt->SetStats(0);
		comp_bcdt->SetLineColor(kRed);
		comp_bcdt->GetXaxis()->SetRangeUser(-40,40);
		comp_bcdt->SetTitle("CCAL Shower Time - Beam Time");
		comp_bcdt->GetXaxis()->SetTitle("t_{ccal} - t_{beam} [ns]");
		comp_bcdt->GetXaxis()->SetTitleSize(0.045);
		comp_bcdt->GetXaxis()->SetTitleOffset(1.0);
		comp_bcdt->GetYaxis()->SetTitleOffset(1.6);
		
		c1->cd(3);
		comp_bcdt->Draw();
	}
	
	if( comp_fcdt ){
		
		comp_fcdt->SetStats(0);
		comp_fcdt->GetXaxis()->SetRangeUser(-20,20);
		comp_fcdt->GetXaxis()->SetTitle("t_{ccal} - t_{fcal} [ns]");
		comp_fcdt->GetXaxis()->SetTitleSize(0.045);
		comp_bcdt->GetXaxis()->SetTitleOffset(1.0);
		comp_bcdt->GetYaxis()->SetTitleOffset(1.6);
		c1->cd(4);
		comp_fcdt->Draw();
	}
	
}
