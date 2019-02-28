// hnamepath:  /ccal/clusN
// hnamepath:  /ccal/clusE
// hnamepath:  /ccal/clusT0
// hnamepath:  /ccal/clusTmT0

{

  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("ccal");
  if(dir) dir->cd();

  TH1I* comp_cratio   = (TH1I*)gDirectory->FindObjectAny( "comp_cratio" );
  TH1I* comp_cfbratio = (TH1I*)gDirectory->FindObjectAny( "comp_cfbratio" );
  TH1I* comp_cfb2d    = (TH1I*)gDirectory->FindObjectAny( "comp_cfb2d" );
  TH1I* comp_pfpc     = (TH1I*)gDirectory->FindObjectAny( "comp_pfpc" );
 
  if(gPad == NULL){

    TCanvas *c1 = new TCanvas( "c1", "CCAL Compton Monitor", 800, 800 );
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }

  if( !gPad ) return;
  TCanvas* c1 = gPad->GetCanvas();
  c1->Divide( 2, 2 );

  if( comp_cratio ){

    comp_cratio->SetStats( 0 );
    //comp_cratio->SetFillColor( kBlue );
    comp_cratio->GetXaxis()->SetTitle("E_{ccal} / E_{comp}");
    comp_cratio->GetXaxis()->SetTitleOffset(1.3);
    TPad *p1 = (TPad*)c1->cd( 1 );
    p1->SetGrid();
    //c1->cd( 1 );
    comp_cratio->Draw();
  }
  
  if( comp_cfbratio ){

    comp_cfbratio->SetStats( 0 );
    //comp_cfbratio->SetFillColor( kBlue );
    comp_cfbratio->GetXaxis()->SetTitle("(E_{ccal}+E_{fcal}-E_{beam}) / E_{comp}");
    comp_cfbratio->GetXaxis()->SetTitleOffset(1.3);
    comp_cfbratio->SetTitle("Energy Conservation in Compton Events");
    TPad *p2 = (TPad*)c1->cd( 3 );
    p2->SetGrid();
    //c1->cd( 2 );
    comp_cfbratio->Draw();
  }
  
  if( comp_pfpc ){

    comp_pfpc->SetStats( 0 );
    comp_pfpc->SetFillColor( kBlue );
    c1->cd( 2 );
    comp_pfpc->Draw();
  }

  if( comp_cfb2d ){

    comp_cfb2d->SetStats( 0 );
    comp_cfb2d->GetXaxis()->SetTitle("(E_{ccal,comp}+E_{fcal,comp}) / E_{beam}");
    comp_cfb2d->GetYaxis()->SetTitle("(E_{ccal}+E_{fcal}) / E_{beam}");
    comp_cfb2d->GetXaxis()->SetTitleOffset(1.3);
    comp_cfb2d->GetYaxis()->SetTitleOffset(1.3);
    c1->cd( 4 );
    comp_cfb2d->Draw("LEGO3");
  }

}
