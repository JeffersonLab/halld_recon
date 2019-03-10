// hnamepath: /ccal/clus2GMass
// hnamepath: /ccal/clusPhi
// hnamepath: /ccal/clusXYHigh
// hnamepath: /ccal/clusXYLow

{
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("ccal");
  if(dir) dir->cd();

  TH1I* clus2GMass  = (TH1I*)gDirectory->FindObjectAny( "clus2GMass" );
  TH1I* clusPhi     = (TH1I*)gDirectory->FindObjectAny( "clusPhi" );
  TH1I* clusXYHigh  = (TH1I*)gDirectory->FindObjectAny( "clusXYHigh" );
  TH1I* clusOccEmax = (TH1I*)gDirectory->FindObjectAny( "clusOccEmax" );
 
  if(gPad == NULL){

    TCanvas *c1 = new TCanvas( "c1", "CCAL Monitor", 800, 800 );
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }

  if( !gPad ) return;
  TCanvas* c1 = gPad->GetCanvas();
  c1->Divide( 2, 2 );

  if( clus2GMass ){

    clus2GMass->SetStats( 0 );
    clus2GMass->SetFillColor( kBlue );
    c1->cd( 1 );
    clus2GMass->GetXaxis()->SetRangeUser(0,0.5);
    clus2GMass->Draw();
    TLine *line_pi0Mass=new TLine(0.1349766,0,0.1349766,clus2GMass->GetMaximum());
    line_pi0Mass->SetLineColor(kRed);
    line_pi0Mass->Draw("same");
  }
  
  if( clusPhi ){

    clusPhi->SetStats( 0 );
    clusPhi->SetFillColor( kBlue );
    c1->cd( 2 );
    clusPhi->Draw();
  }

  if( clusXYHigh ){

    clusXYHigh->SetStats( 0 );
    TPad *p3 = (TPad*)c1->cd( 3 );
    p3->SetLogz();
    //c1->cd( 3 );
    clusXYHigh->Draw( "colz" );
  }

  if( clusOccEmax ){

    clusOccEmax->SetStats( 0 );
    clusOccEmax->SetTitle("CCAL Occup. for (E_{max} > 3 GeV) & (E_{cluster}-E_{max} < 1 GeV)");
    c1->cd( 4 );
    clusOccEmax->Draw( "colz" );
  }

}
