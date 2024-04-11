// hnamepath: /tagm/tagmCol9
// hnamepath: /tagm/tagmCol27
// hnamepath: /tagm/tagmCol81
// hnamepath: /tagm/tagmCol99

{
  cout<< "Entered macro: TAGM_online/HistMacro_TAGMCOL.C" << endl;

  //Goto directory tagm
  TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("tagm");
  if(!locDirectory)
    return;
  locDirectory->cd();

  //fetch the histograms
  TH1I *locHistTAGMcol9  = (TH1I*)gDirectory->Get("tagmCol9");
  TH1I *locHistTAGMcol27 = (TH1I*)gDirectory->Get("tagmCol27");
  TH1I *locHistTAGMcol81 = (TH1I*)gDirectory->Get("tagmCol81");
  TH1I *locHistTAGMcol99 = (TH1I*)gDirectory->Get("tagmCol99");

  TCanvas *locCanvas = NULL;
  if(TVirtualPad::Pad() == NULL)
    locCanvas = new TCanvas("TAGM_Cols", "TAGM Columns", 1200, 800); //for testing
  else
    locCanvas = gPad->GetCanvas();
  locCanvas->Divide(2, 2);
  
  //Draw
  locCanvas->cd(1);
  gPad->SetTicks();
  gPad->SetGrid();
  if (locHistTAGMcol9 != NULL){
    TH1I* hist = locHistTAGMcol9;
    hist->SetStats(0);
    hist->GetXaxis()->SetTitleSize(0.05);
    hist->GetYaxis()->SetTitleSize(0.05);
    hist->GetXaxis()->SetLabelSize(0.05);
    hist->GetYaxis()->SetLabelSize(0.05);
    hist->Fit("gaus", "Q", "R", 1., 6.);
    hist->Draw();    
  }
  locCanvas->cd(2);
  gPad->SetTicks();
  gPad->SetGrid();
  if (locHistTAGMcol27 != NULL){
    TH1I* hist = locHistTAGMcol27;
    hist->SetStats(0);
    hist->GetXaxis()->SetTitleSize(0.05);
    hist->GetYaxis()->SetTitleSize(0.05);
    hist->GetXaxis()->SetLabelSize(0.05);
    hist->GetYaxis()->SetLabelSize(0.05);
    hist->Fit("gaus", "Q", "R", 1., 6.);
    hist->Draw();    
  }
  locCanvas->cd(3);
  gPad->SetTicks();
  gPad->SetGrid();
  if (locHistTAGMcol81 != NULL){
    TH1I* hist = locHistTAGMcol81;
    hist->SetStats(0);
    hist->GetXaxis()->SetTitleSize(0.05);
    hist->GetYaxis()->SetTitleSize(0.05);
    hist->GetXaxis()->SetLabelSize(0.05);
    hist->GetYaxis()->SetLabelSize(0.05);
    hist->Fit("gaus", "Q", "R", 1., 6.);
    hist->Draw();    
  }
  locCanvas->cd(4);
  gPad->SetTicks();
  gPad->SetGrid();
  if (locHistTAGMcol99 != NULL){
    TH1I* hist = locHistTAGMcol99;
    hist->SetStats(0);
    hist->GetXaxis()->SetTitleSize(0.05);
    hist->GetYaxis()->SetTitleSize(0.05);
    hist->GetXaxis()->SetLabelSize(0.05);
    hist->GetYaxis()->SetLabelSize(0.05);
    hist->Fit("gaus", "Q", "R", 1., 6.);
    hist->Draw();    
  }

  cout<< "Entered macro: TAGM_online/HistMacro_TAGMCOL.C" << endl;

#ifdef ROOTSPY_MACROS

  // -- The following is used by RSAI --
  cout<< "the following code for RSAI is in macro TAGM_online/HistMacro_TAGMCOL.C" << endl;

  if (rs_GetFlag("Is_RSAI") == 1) {

    int Nhits = tagmCol9->GetEntries();
    if (Nhits>1e4) {
      rs_SavePad("TAGM_Column9",1);
      rs_SavePad("TAGM_Column27",2);
      rs_SavePad("TAGM_Column81",3);
      rs_SavePad("TAGM_Column99",4);    

      rs_ResetAllMacroHistos("//HistMacro_TAGMCOL");
    }
  }


#endif

}
