void plot_fdc_drift(){
  TCanvas *canv=new TCanvas("fcanvas","FDC Time-to-distance",1600,1000);
  canv->Divide(6,4);
  for (int i=0;i<24;i++){
    canv->cd(i+1);
    char h2name[80],h1name[80];
    sprintf(h2name,"HDVsT%d",i);
    sprintf(h1name,"HDVsT%d_1",i);
    TH2F *h2=(TH2F*)_file0->FindObjectAny(h2name);
    TH1F *h1=(TH1F*)_file0->FindObjectAny(h1name);
    h2->GetXaxis()->SetRangeUser(0,200);
    h2->GetYaxis()->SetRangeUser(0,0.6);
    h2->Draw("col");
    h1->Draw("same");
  }
  
}
