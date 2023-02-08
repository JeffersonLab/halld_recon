
void FMWPC_Performance(){
  
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);

  Float_t minScale = 0.5;
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
      TF1* fdoublegaus = new TF1("fdoublegaus", "gaus(0)+gaus(3)",-100,100);
      fdoublegaus->SetParameter(1,0);
      fdoublegaus->SetParameter(2,5);
      fdoublegaus->SetParameter(4,0);
      fdoublegaus->SetParameter(5,30);
      h3->Fit("fdoublegaus","Q");
    }
  }

  TCanvas *cCorrelation = new TCanvas("cCorrelation", "Track - FMWPC Correlation", 800, 600);
  cCorrelation->Divide(3,2);

  for(unsigned int layer=1; layer<=6; layer++){
    cCorrelation->cd(layer);
    char hname1[256];
    sprintf(hname1, "hfmwpc_correlation_layer%d", layer);
    TH1D *h4 = (TH1D*)(gDirectory->Get(hname1));

    if(h4){
      h4->GetXaxis()->SetTitle("Track Position (cm)");
      h4->GetYaxis()->SetTitle("Cluster Position (cm)");
      h4->Draw("colz");
    }
  }
}

