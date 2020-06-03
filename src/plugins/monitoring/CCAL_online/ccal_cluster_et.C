// hnamepath: /ccal/showN
// hnamepath: /ccal/showE
// hnamepath: /ccal/showETot
// hnamepath: /ccal/showT

{

  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("ccal");
  if(dir) dir->cd();

  TH1I* showN    = (TH1I*)gDirectory->FindObjectAny( "showN" );
  TH1I* showE    = (TH1I*)gDirectory->FindObjectAny( "showE" );
  TH1I* showETot = (TH1I*)gDirectory->FindObjectAny( "showETot" );
  TH1I* showT    = (TH1I*)gDirectory->FindObjectAny( "showT" );
 
  if(gPad == NULL){

    TCanvas *c1 = new TCanvas( "c1", "CCAL Monitor", 800, 800 );
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }

  if( !gPad ) return;
  TCanvas* c1 = gPad->GetCanvas();
  c1->Divide( 2, 2 );

  if( showN ){

    showN->SetStats( 0 );
    showN->SetFillColor( kCyan-10 );
    c1->cd( 1 );
    showN->Draw();
  }
  
  if( showE ){

    showE->SetStats( 0 );
    showE->SetFillColor( kCyan-10 );
    showE->GetXaxis()->SetRangeUser(0,12000);
    c1->cd( 2 );
    showE->Draw();
  }

  if( showETot ){

    showETot->SetStats( 0 );
    showETot->SetFillColor( kCyan-10 );
    showETot->GetXaxis()->SetRangeUser(0,12000);
    c1->cd( 4 );
    showETot->Draw();
  }

  if( showT ){

    showT->SetStats( 0 );
    showT->SetFillColor( kCyan-10 );
    showT->GetXaxis()->SetRangeUser(-100,300);
    c1->cd( 3 );
    showT->Draw();
  }

}
