// hnamepath: /ccal/show2GMass
// hnamepath: /ccal/showPhi
// hnamepath: /ccal/showXYHigh
// hnamepath: /ccal/showOccEmax

{
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("ccal");
  if(dir) dir->cd();

  TH1I* show2GMass  = (TH1I*)gDirectory->FindObjectAny( "show2GMass" );
  TH1I* showPhi     = (TH1I*)gDirectory->FindObjectAny( "showPhi" );
  TH1I* showXYHigh  = (TH1I*)gDirectory->FindObjectAny( "showXYHigh" );
  TH1I* showOccEmax = (TH1I*)gDirectory->FindObjectAny( "showOccEmax" );
 
  if(gPad == NULL){

    TCanvas *c1 = new TCanvas( "c1", "CCAL Monitor", 800, 800 );
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }

  if( !gPad ) return;
  TCanvas* c1 = gPad->GetCanvas();
  c1->Divide( 2, 2 );

  if( show2GMass ){

    show2GMass->SetStats( 0 );
    show2GMass->SetFillColor( kBlue );
    c1->cd( 1 );
    show2GMass->GetXaxis()->SetRangeUser(0,0.5);
    show2GMass->Draw();
    TLine *line_pi0Mass=new TLine(0.1349766,0,0.1349766,show2GMass->GetMaximum());
    line_pi0Mass->SetLineColor(kRed);
    line_pi0Mass->Draw("same");
  }
  
  if( showPhi ){

    showPhi->SetStats( 0 );
    showPhi->SetFillColor( kBlue );
    c1->cd( 2 );
    showPhi->Draw();
  }

  if( showXYHigh ){

    showXYHigh->SetStats( 0 );
    TPad *p3 = (TPad*)c1->cd( 3 );
    p3->SetLogz();
    //c1->cd( 3 );
    showXYHigh->Draw( "colz" );
  }

  if( showOccEmax ){

    showOccEmax->SetStats( 0 );
    showOccEmax->SetTitle("CCAL Occup. for (E_{max} > 3 GeV) & (E_{showter}-E_{max} < 1 GeV)");
    c1->cd( 4 );
    showOccEmax->Draw( "colz" );
  }

}
