// hnamepath: /ccal/comp_cratio
// hnamepath: /ccal/comp_cfbratio
// hnamepath: /ccal/comp_cfb2d
// hnamepath: /ccal/comp_pfpc
// hnamepath: /highlevel/PSPairEnergy
// hnamepath: /highlevel/L1bits_gtp
// hnamepath: /highlevel/EventInfo


{
	
	TDirectory *locdir       = (TDirectory*)gDirectory->FindObjectAny("ccal");
	TDirectory *highleveldir = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(dir) dir->cd();
	
	TH1I* hbeamX   = (TH1I*)gDirectory->FindObjectAny("beamX");
	TH1I* hbeamY   = (TH1I*)gDirectory->FindObjectAny("beamY");
	
	if(gPad == NULL){
		TCanvas *c1 = new TCanvas("c1", "CCAL Compton Monitor", 800, 800);
		c1->cd(0);
		c1->Draw();
		c1->Update();
	}
	
	if( !gPad ) return;
	TCanvas* c1 = gPad->GetCanvas();
	c1->Divide(2, 2);
	
	if( beamX ){
		
	}
	
	if( comp_cfbratio ){
		
		comp_cfbratio->SetStats(0);
		//comp_cfbratio->SetFillColor(kBlue);
		comp_cfbratio->Rebin(2);
		comp_cfbratio->GetXaxis()->SetTitle("(E_{ccal} + E_{fcal} - E_{beam}) / E_{comp}");
		comp_cfbratio->GetXaxis()->SetTitleOffset(1.3);
		comp_cfbratio->SetTitle("Energy Conservation in Compton Events");
		
		TPad *p2 = (TPad*)c1->cd(3);
		p2->SetGrid();
		//c1->cd(2);
		comp_cfbratio->Draw();
	}
	
	if( comp_pfpc ){
		
		comp_pfpc->SetStats(0);
		comp_pfpc->SetFillColor(kBlue);
		c1->cd(2);
		comp_pfpc->Draw();
	}
	
	if( comp_cfb2d ){
		
		comp_cfb2d->SetStats(0);
		comp_cfb2d->GetXaxis()->SetTitle("(E_{ccal,comp} + E_{fcal,comp}) / E_{beam}");
		comp_cfb2d->GetYaxis()->SetTitle("(E_{ccal}+E_{fcal}) / E_{beam}");
		comp_cfb2d->GetXaxis()->SetTitleOffset(1.3);
		comp_cfb2d->GetYaxis()->SetTitleOffset(1.3);
		c1->cd(4);
		comp_cfb2d->Draw("LEGO3");
	}
	
}
