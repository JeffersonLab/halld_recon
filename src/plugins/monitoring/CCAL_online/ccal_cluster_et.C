// hnamepath: /ccal/clusN
// hnamepath: /ccal/clusE
// hnamepath: /ccal/clusETot
// hnamepath: /ccal/clusT

{

  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("ccal");
  if(dir) dir->cd();

  TH1I* clusN    = (TH1I*)gDirectory->FindObjectAny( "clusN" );
  TH1I* clusE    = (TH1I*)gDirectory->FindObjectAny( "clusE" );
  TH1I* clusETot = (TH1I*)gDirectory->FindObjectAny( "clusETot" );
  TH1I* clusT    = (TH1I*)gDirectory->FindObjectAny( "clusT" );
 
  if(gPad == NULL){

    TCanvas *c1 = new TCanvas( "c1", "CCAL Monitor", 800, 800 );
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }

  if( !gPad ) return;
  TCanvas* c1 = gPad->GetCanvas();
  c1->Divide( 2, 2 );

  if( clusN ){

    clusN->SetStats( 0 );
    clusN->SetFillColor( kBlue );
    c1->cd( 1 );
    clusN->Draw();
  }
  
  if( clusE ){

    clusE->SetStats( 0 );
    clusE->SetFillColor( kBlue );
    clusE->GetXaxis()->SetRangeUser(0,12000);
    c1->cd( 2 );
    clusE->Draw();
  }

  if( clusETot ){

    clusETot->SetStats( 0 );
    clusETot->SetFillColor( kBlue );
    clusETot->GetXaxis()->SetRangeUser(0,12000);
    c1->cd( 4 );
    clusETot->Draw();
  }

  if( clusT ){

    clusT->SetStats( 0 );
    clusT->SetFillColor( kBlue );
    clusT->GetXaxis()->SetRangeUser(0,300);
    c1->cd( 3 );
    clusT->Draw();
  }

}
