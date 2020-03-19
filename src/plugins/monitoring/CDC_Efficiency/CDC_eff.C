
{

    //gDirectory->cd();
    //TDirectory *gDirectory = TFile::Open("hd_root.root");
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("CDC_Efficiency");
    if(!dir) return;
    dir->cd();

    TCanvas *cDOCA = new TCanvas("cDOCA", "cDOCA", 800, 600);
    cDOCA->SetGridx();
    cDOCA->SetGridy();
    TH1I *MeasDOCA = (TH1I*)(gDirectory->Get("Offline/Measured Hits Vs DOCA"));
    TH1I *ExpDOCA = (TH1I*)(gDirectory->Get("Offline/Expected Hits Vs DOCA"));
    if(MeasDOCA && ExpDOCA){
        //EffDOCA->Draw();
        TGraphAsymmErrors *EffDOCA = new  TGraphAsymmErrors(MeasDOCA, ExpDOCA, "ac");
        EffDOCA->Draw("ap");
        EffDOCA->SetMinimum(0.0);
        EffDOCA->SetMaximum(1.0);
        EffDOCA->SetTitle("CDC Per Straw Efficiency Vs. DOCA");
        EffDOCA->GetXaxis()->SetTitle("Closest distance between track and wire [cm]");
        EffDOCA->GetYaxis()->SetTitle("Efficiency");
        //cDOCA->SaveAs("cDOCA.png");
    }


}
