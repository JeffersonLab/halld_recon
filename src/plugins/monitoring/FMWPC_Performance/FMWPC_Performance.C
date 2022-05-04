
void FMWPC_Performance(){
  
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);

  Float_t minScale = 0.8;
  Float_t maxScale = 1.0;    

  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("FMWPC_Performance");
  if(!dir) return;
  dir->cd();

  gDirectory->cd("Efficiency");
  TCanvas *cEfficiency = new TCanvas("cEfficiency", "FMWPC Efficiency", 800, 600);
  cEfficiency->Divide(3,2);

  for(unsigned int layer=1; layer<=6; layer++){
    cEfficiency->cd(layer);
    char hname1[256];
    sprintf(hname1, "hfmwpc_expected_layer%d", layer);
    TH2D *h1 = (TH2D*)(gDirectory->Get(hname1));
    char hname2[256];
    sprintf(hname2, "hfmwpc_measured_layer%d", layer);
    TH1 *h2 = (TH2D*)(gDirectory->Get(hname2));
      
    if(h1 && h2){
      h2->Divide(h1);
      h2->SetMinimum(minScale);
      h2->SetMaximum(maxScale);
      h2->GetXaxis()->SetTitle("X Position (cm)");
      h2->GetYaxis()->SetTitle("Y Position (cm)");
      h2->SetStats(0);
      h2->Draw("colz");
    }
  }

  gDirectory->cd("../Alignment");
  TCanvas *cResolution = new TCanvas("cResolution", "FMWPC Resolution", 800, 600);
  cResolution->Divide(3,2);

  for(unsigned int layer=1; layer<=6; layer++){
    cResolution->cd(layer);
    char hname1[256];
    sprintf(hname1, "hfmwpc_residual_layer%d", layer);
    TH1D *h3 = (TH1D*)(gDirectory->Get(hname1));
    
    if(h3){
      h3->GetXaxis()->SetTitle("Residual(Track - Cluster) (cm)");
      h3->Draw();
      h3->Fit("gaus","Q");
    }
  }
}

