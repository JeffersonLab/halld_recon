void plot_MWPChits (TString tag="Circle_dead")
{
// File: plot_MWPChits.C
    // Read output of cppMWPC plugin and plot histograms.
//

  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(111111);
  gStyle->SetOptFit(111111);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
    
    // TString tag="CL6e-7";
    // TString tag="CL01";
    TString index;
    
    // TFile *inData = new TFile("hd_root_particle_gun_031007_000.root","read");
    // TFile *inData = new TFile("hd_root_particle_gun_031009_000.root","read");  // 2deg +/-0.1 deg
    // TFile *inData = new TFile("hd_root_particle_gun_031011_000.root","read");
    // TFile *inData = new TFile("hd_root_particle_gun_031012_000.root","read");    // 2deg +/- 3.5 deg
    TFile *inData = new TFile("hd_root_particle_gun_031018.root","read");    // 2deg +/- 3.95 deg; Dead wires
    TDirectory *dir = (TDirectory *)gDirectory->FindObjectAny("FMWPC");
    if (dir) dir->cd();
    
    const Int_t nFMWPCchambers=6;
    TH2D *FMWPCwiresT[nFMWPCchambers];
    TH2D *FMWPCwiresE[nFMWPCchambers];
    TH1D *FMWPCwiresT_py[nFMWPCchambers];   // time projection
    TH1D *FMWPCwiresE_py[nFMWPCchambers];   // energy projection
    
    for (Int_t jj=0; jj<nFMWPCchambers; jj++) {
        index.Form("FMWPCwiresT%d",jj);
        cout << "jj=" << jj << "index=" << index << endl;
        FMWPCwiresT[jj] = (TH2D*) gDirectory->FindObjectAny(index);
        index.Form("FMWPCwiresE%d",jj);
        cout << "jj=" << jj << "index=" << index << endl;
        FMWPCwiresE[jj] = (TH2D*) gDirectory->FindObjectAny(index);
    }
    
    TH2D *h2_V1_vs_H2 = (TH2D*) gDirectory->FindObjectAny("h2_V1_vs_H2");
    TH2D *h2_V3_vs_H4 = (TH2D*) gDirectory->FindObjectAny("h2_V3_vs_H4");
    TH2D *h2_V5_vs_H4 = (TH2D*) gDirectory->FindObjectAny("h2_V5_vs_H4");
    TH2D *h2_V6_vs_H4 = (TH2D*) gDirectory->FindObjectAny("h2_V6_vs_H4");
    
    TH2D *h2_V3_vs_V1 = (TH2D*) gDirectory->FindObjectAny("h2_V3_vs_V1");
    TH2D *h2_H2_vs_H4 = (TH2D*) gDirectory->FindObjectAny("h2_H2_vs_H4");
    TH2D *h2_V3_vs_V5 = (TH2D*) gDirectory->FindObjectAny("h2_V3_vs_V5");
    TH2D *h2_V5_vs_V6 = (TH2D*) gDirectory->FindObjectAny("h2_V5_vs_V6");
    
    Double_t xmin=0.;
    Double_t xmax=2.0;
    Double_t ymin=0;
    Double_t ymax=1.5;
    
    TCanvas *c0 = new TCanvas("c0", "c0",200,10,1300,700);
    c0->Divide(4,2);
    TString text;
    text.Form("X range=%.1f-%.1f GeV",xmin,xmax);
    TLatex *t1 = new TLatex(0.3,0.45,text);
    
    c0->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogz();
    // h2_V1_vs_H2->SetTitle("");
    // h2_V1_vs_H2->GetXaxis()->SetRangeUser(xmin,xmax);
    // h2_V1_vs_H2->GetYaxis()->SetRangeUser(ymin,ymax);
    h2_V1_vs_H2->GetXaxis()->SetTitleSize(0.05);
    h2_V1_vs_H2->GetYaxis()->SetTitleSize(0.05);
    //h2_V1_vs_H2->GetXaxis()->SetTitle("");
    //h2_V1_vs_H2->GetYaxis()->SetTitle("");
    h2_V1_vs_H2->SetLineColor(2);
    h2_V1_vs_H2->SetMarkerColor(2);
    h2_V1_vs_H2->SetMarkerStyle(20);
    h2_V1_vs_H2->SetMarkerSize(0.5);
    h2_V1_vs_H2->Draw("colz");
    
    c0->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogz();
    // h2_V3_vs_H4->SetTitle("");
    // h2_V3_vs_H4->GetXaxis()->SetRangeUser(xmin,xmax);
    // h2_V3_vs_H4->GetYaxis()->SetRangeUser(ymin,ymax);
    h2_V3_vs_H4->GetXaxis()->SetTitleSize(0.05);
    h2_V3_vs_H4->GetYaxis()->SetTitleSize(0.05);
    //h2_V3_vs_H4->GetXaxis()->SetTitle("");
    //h2_V3_vs_H4->GetYaxis()->SetTitle("");
    h2_V3_vs_H4->SetLineColor(2);
    h2_V3_vs_H4->SetMarkerColor(2);
    h2_V3_vs_H4->SetMarkerStyle(20);
    h2_V3_vs_H4->SetMarkerSize(0.5);
    h2_V3_vs_H4->Draw("colz");
    
    c0->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogz();
    // h2_V5_vs_H4->SetTitle("");
    // h2_V5_vs_H4->GetXaxis()->SetRangeUser(xmin,xmax);
    // h2_V5_vs_H4->GetYaxis()->SetRangeUser(ymin,ymax);
    h2_V5_vs_H4->GetXaxis()->SetTitleSize(0.05);
    h2_V5_vs_H4->GetYaxis()->SetTitleSize(0.05);
    //h2_V5_vs_H4->GetXaxis()->SetTitle("");
    //h2_V5_vs_H4->GetYaxis()->SetTitle("");
    h2_V5_vs_H4->SetLineColor(2);
    h2_V5_vs_H4->SetMarkerColor(2);
    h2_V5_vs_H4->SetMarkerStyle(20);
    h2_V5_vs_H4->SetMarkerSize(0.5);
    h2_V5_vs_H4->Draw("colz");
    
    c0->cd(4);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogz();
    // h2_V6_vs_H4->SetTitle("");
    // h2_V6_vs_H4->GetXaxis()->SetRangeUser(xmin,xmax);
    // h2_V6_vs_H4->GetYaxis()->SetRangeUser(ymin,ymax);
    h2_V6_vs_H4->GetXaxis()->SetTitleSize(0.05);
    h2_V6_vs_H4->GetYaxis()->SetTitleSize(0.05);
    //h2_V6_vs_H4->GetXaxis()->SetTitle("");
    //h2_V6_vs_H4->GetYaxis()->SetTitle("");
    h2_V6_vs_H4->SetLineColor(2);
    h2_V6_vs_H4->SetMarkerColor(2);
    h2_V6_vs_H4->SetMarkerStyle(20);
    h2_V6_vs_H4->SetMarkerSize(0.5);
    h2_V6_vs_H4->Draw("colz");
    
    c0->cd(5);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogz();
    // h2_V3_vs_V1->SetTitle("");
    // h2_V3_vs_V1->GetXaxis()->SetRangeUser(xmin,xmax);
    // h2_V3_vs_V1->GetYaxis()->SetRangeUser(ymin,ymax);
    h2_V3_vs_V1->GetXaxis()->SetTitleSize(0.05);
    h2_V3_vs_V1->GetYaxis()->SetTitleSize(0.05);
    //h2_V3_vs_V1->GetXaxis()->SetTitle("");
    //h2_V3_vs_V1->GetYaxis()->SetTitle("");
    h2_V3_vs_V1->SetLineColor(2);
    h2_V3_vs_V1->SetMarkerColor(2);
    h2_V3_vs_V1->SetMarkerStyle(20);
    h2_V3_vs_V1->SetMarkerSize(0.5);
    h2_V3_vs_V1->Draw("colz");
    
    c0->cd(6);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogz();
    // h2_H2_vs_H4->SetTitle("");
    // h2_H2_vs_H4->GetXaxis()->SetRangeUser(xmin,xmax);
    // h2_H2_vs_H4->GetYaxis()->SetRangeUser(ymin,ymax);
    h2_H2_vs_H4->GetXaxis()->SetTitleSize(0.05);
    h2_H2_vs_H4->GetYaxis()->SetTitleSize(0.05);
    //h2_H2_vs_H4->GetXaxis()->SetTitle("");
    //h2_H2_vs_H4->GetYaxis()->SetTitle("");
    h2_H2_vs_H4->SetLineColor(2);
    h2_H2_vs_H4->SetMarkerColor(2);
    h2_H2_vs_H4->SetMarkerStyle(20);
    h2_H2_vs_H4->SetMarkerSize(0.5);
    h2_H2_vs_H4->Draw("colz");
    
    c0->cd(7);
    gPad->SetGridx();
    gPad->SetGridy();
    // h2_V3_vs_V5->SetTitle("");
    // h2_V3_vs_V5->GetXaxis()->SetRangeUser(xmin,xmax);
    // h2_V3_vs_V5->GetYaxis()->SetRangeUser(ymin,ymax);
    h2_V3_vs_V5->GetXaxis()->SetTitleSize(0.05);
    h2_V3_vs_V5->GetYaxis()->SetTitleSize(0.05);
    //h2_V3_vs_V5->GetXaxis()->SetTitle("");
    //h2_V3_vs_V5->GetYaxis()->SetTitle("");
    h2_V3_vs_V5->SetLineColor(2);
    h2_V3_vs_V5->SetMarkerColor(2);
    h2_V3_vs_V5->SetMarkerStyle(20);
    h2_V3_vs_V5->SetMarkerSize(0.5);
    h2_V3_vs_V5->Draw("colz");
    
    c0->cd(8);
    gPad->SetGridx();
    gPad->SetGridy();
    // h2_V5_vs_V6->SetTitle("");
    // h2_V5_vs_V6->GetXaxis()->SetRangeUser(xmin,xmax);
    // h2_V5_vs_V6->GetYaxis()->SetRangeUser(ymin,ymax);
    h2_V5_vs_V6->GetXaxis()->SetTitleSize(0.05);
    h2_V5_vs_V6->GetYaxis()->SetTitleSize(0.05);
    //h2_V5_vs_V6->GetXaxis()->SetTitle("");
    //h2_V5_vs_V6->GetYaxis()->SetTitle("");
    h2_V5_vs_V6->SetLineColor(2);
    h2_V5_vs_V6->SetMarkerColor(2);
    h2_V5_vs_V6->SetMarkerStyle(20);
    h2_V5_vs_V6->SetMarkerSize(0.5);
    h2_V5_vs_V6->Draw("colz");
    
    
    TCanvas *c1 = new TCanvas("c1", "c1",200,10,1300,700);
    c1->Divide(nFMWPCchambers,2);
    
    for (Int_t jj=0; jj<nFMWPCchambers; jj++) {
        c1->cd(jj+1);
        gPad->SetGridx();
        gPad->SetGridy();
        // FMWPCwiresT[jj]->SetTitle("");
        // FMWPCwiresT[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
        // FMWPCwiresT[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
        FMWPCwiresT[jj]->GetXaxis()->SetTitleSize(0.05);
        FMWPCwiresT[jj]->GetYaxis()->SetTitleSize(0.05);
        FMWPCwiresT[jj]->GetXaxis()->SetTitle("Wire #");
        FMWPCwiresT[jj]->GetYaxis()->SetTitle("Drift Time (ns)");
        FMWPCwiresT[jj]->SetLineColor(2);
        FMWPCwiresT[jj]->SetMarkerColor(2);
        FMWPCwiresT[jj]->SetMarkerStyle(20);
        FMWPCwiresT[jj]->SetMarkerSize(0.5);
        FMWPCwiresT[jj]->Draw("colz");
        
        text.Form("MWPC #%d",jj+1);
        t1->SetNDC();
        t1->SetTextSize(0.07);
        t1->DrawLatex(0.3,0.8,text);
        text.Form("Drift Times");
        t1->SetNDC();
        t1->SetTextSize(0.07);
        t1->DrawLatex(0.3,0.75,text);
    
        c1->cd(jj+7);
        gPad->SetGridx();
        gPad->SetGridy();
        // FMWPCwiresE[jj]->SetTitle("");
        // FMWPCwiresE[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
        // FMWPCwiresE[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
        FMWPCwiresE[jj]->GetXaxis()->SetTitleSize(0.05);
        FMWPCwiresE[jj]->GetYaxis()->SetTitleSize(0.05);
        FMWPCwiresE[jj]->GetXaxis()->SetTitle("Wire Nunber");
        FMWPCwiresE[jj]->GetYaxis()->SetTitle("Pulse Integral (counts)");
        FMWPCwiresE[jj]->SetLineColor(2);
        FMWPCwiresE[jj]->SetMarkerColor(2);
        FMWPCwiresE[jj]->SetMarkerStyle(20);
        FMWPCwiresE[jj]->SetMarkerSize(0.5);
        FMWPCwiresE[jj]->Draw("colz");
        
        text.Form("MWPC #%d",jj+1);
        t1->SetNDC();
        t1->SetTextSize(0.07);
        t1->DrawLatex(0.3,0.8,text);
        text.Form("Pulse Integral");
        t1->SetNDC();
        t1->SetTextSize(0.07);
        t1->DrawLatex(0.3,0.75,text);
    }
    
    
    
    TCanvas *c2 = new TCanvas("c2", "c2",200,10,1300,700);
    c2->Divide(nFMWPCchambers,2);
    
    for (Int_t jj=0; jj<nFMWPCchambers; jj++) {
        c2->cd(jj+1);
        gPad->SetGridx();
        gPad->SetGridy();
        // FMWPCwiresT_py[jj]->SetTitle("");
        // FMWPCwiresT_py[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
        // FMWPCwiresT_py[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
        FMWPCwiresT_py[jj] = FMWPCwiresT[jj]->ProjectionY();
        FMWPCwiresT_py[jj]->GetXaxis()->SetTitleSize(0.05);
        FMWPCwiresT_py[jj]->GetYaxis()->SetTitleSize(0.05);
        FMWPCwiresT_py[jj]->GetXaxis()->SetTitle("Drift Time (ns)");
        FMWPCwiresT_py[jj]->SetLineColor(2);
        FMWPCwiresT_py[jj]->SetMarkerColor(2);
        FMWPCwiresT_py[jj]->SetMarkerStyle(20);
        FMWPCwiresT_py[jj]->SetMarkerSize(0.5);
        FMWPCwiresT_py[jj]->Draw("");
        
        text.Form("MWPC #%d",jj+1);
        t1->SetNDC();
        t1->SetTextSize(0.07);
        t1->DrawLatex(0.3,0.8,text);
        text.Form("Mean Drift Time");
        t1->SetNDC();
        t1->SetTextSize(0.07);
        t1->DrawLatex(0.3,0.75,text);
        
        c2->cd(jj+7);
        gPad->SetGridx();
        gPad->SetGridy();
        // FMWPCwiresE_py[jj]->SetTitle("");
        // FMWPCwiresE_py[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
        // FMWPCwiresE_py[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
        FMWPCwiresE_py[jj] = FMWPCwiresE[jj]->ProjectionY();
        FMWPCwiresE_py[jj]->GetXaxis()->SetTitleSize(0.05);
        FMWPCwiresE_py[jj]->GetYaxis()->SetTitleSize(0.05);
        FMWPCwiresE_py[jj]->GetXaxis()->SetTitle("Pulse Integral (counts)");
        FMWPCwiresE_py[jj]->SetLineColor(2);
        FMWPCwiresE_py[jj]->SetMarkerColor(2);
        FMWPCwiresE_py[jj]->SetMarkerStyle(20);
        FMWPCwiresE_py[jj]->SetMarkerSize(0.5);
        FMWPCwiresE_py[jj]->Draw("");
        
        text.Form("MWPC #%d",jj+1);
        t1->SetNDC();
        t1->SetTextSize(0.07);
        t1->DrawLatex(0.3,0.8,text);
        text.Form("Mean Pulse Integral");
        t1->SetNDC();
        t1->SetTextSize(0.07);
        t1->DrawLatex(0.3,0.75,text);    }
    
    
    /*TLegend *leg = new TLegend(0.6,0.25,0.85,0.4);
    leg->AddEntry(h2_V1_vs_H2,"Data","p");
    leg->Draw();
    
    t1->SetNDC();
    t1->SetTextSize(0.04);
    t1->DrawLatex(0.3,0.40,tag);*/
    
    
    c0->SaveAs("plot_MWPChits_"+tag+".pdf(");
    c1->SaveAs("plot_MWPChits_"+tag+".pdf");
    c2->SaveAs("plot_MWPChits_"+tag+".pdf)");
    
}
