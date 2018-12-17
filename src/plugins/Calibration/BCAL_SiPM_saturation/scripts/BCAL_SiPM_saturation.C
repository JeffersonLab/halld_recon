void BCAL_SiPM_saturation(void)
{
// Plot histograms created with the BCAL_SiPM_saturation pluggin
//

#include <TRandom.h>

gROOT->Reset();
//TTree *Bfield = (TTree *) gROOT->FindObject("Bfield");
gStyle->SetPalette(1,0);
// gStyle->SetOptStat(kFALSE);
gStyle->SetOptStat(111111);
gStyle->SetPadRightMargin(0.15);
gStyle->SetPadLeftMargin(0.15);
gStyle->SetPadBottomMargin(0.15);
//

    Double_t xmin=0.;
    Double_t xmax=0.1;
    Double_t ymin=0.;
    Double_t ymax=100;
    
   // read histograms from file

    TString histfile_name;
    TString title;

    title="gensat_correct0_030746";
    histfile_name= "hd_root_particle_gun_Photons_"+title+"_000";
    // histfile_name= "hd_root_particle_gun_Photons_030731";
    // histfile_name= "BCAL_SiPM_saturation_042433_001";
    // title = "Data Run 42389_001";
    // histfile_name= "BCAL_SiPM_data_42389_001";
    TFile *histfile = new TFile(histfile_name+".root","read");


   TH1I *Ethrown = (TH1I*)gDirectory->FindObjectAny("Ethrown");    
   if( !Ethrown) { cout << "Can't find Ethrown!" << endl; return; }
   TH1I *Eshower = (TH1I*)gDirectory->FindObjectAny("Eshower");    
   if( !Eshower) { cout << "Can't find Eshower!" << endl; return; }
   TH1I *ThrownTheta = (TH1I*)gDirectory->FindObjectAny("Thrown Theta");    
   if( !ThrownTheta) { cout << "Can't find Thrown Theta!" << endl; return; }
   TH1I *NCell = (TH1I*)gDirectory->FindObjectAny("NCell");    
   if( !NCell) { cout << "Can't find NCell!" << endl; return; }
   TH1I *layer = (TH1I*)gDirectory->FindObjectAny("layer");    
   if( !layer) { cout << "Can't find layer!" << endl; return; }
   TH1I *Ept = (TH1I*)gDirectory->FindObjectAny("Ept");    
   if( !Ept) { cout << "Can't find Ept!" << endl; return; }
   TH1I *EcalcDiff = (TH1I*)gDirectory->FindObjectAny("Ecalc-Ept");    
   if( !EcalcDiff) { cout << "Can't find Ecalc-Ept!" << endl; return; }

   TH2I *Eshower_vs_Ethrown = (TH2I*)gDirectory->FindObjectAny("Eshower_vs_Ethrown");    
   if( !Eshower_vs_Ethrown) { cout << "Can't find Eshower_vs_Ethrown!" << endl; return; }
   TH2I *EDiff_vs_Ethrown = (TH2I*)gDirectory->FindObjectAny("EDiff_vs_Ethrown");    
   if( !EDiff_vs_Ethrown) { cout << "Can't find EDiff_vs_Ethrown!" << endl; return; }
   TH2I *EDiffoverEthrown_vs_Ethrown = (TH2I*)gDirectory->FindObjectAny("EDiff/Ethrown_vs_Ethrown");    
   if( !EDiffoverEthrown_vs_Ethrown) { cout << "Can't find EDiff/Ethrown_vs_Ethrown!" << endl; return; }

   TH1I *Hit_pulse_peak_layer1 = (TH1I*)gDirectory->FindObjectAny("Hit pulse_peak layer=1");    
   if( !Hit_pulse_peak_layer1) { cout << "Can't find Hit pulse_peak layer=1!" << endl; return; }
   TH1I *Hit_pulse_peak_layer2 = (TH1I*)gDirectory->FindObjectAny("Hit pulse_peak layer=2");    
   if( !Hit_pulse_peak_layer2) { cout << "Can't find Hit pulse_peak layer=2!" << endl; return; }
   TH1I *Hit_pulse_peak_layer3 = (TH1I*)gDirectory->FindObjectAny("Hit pulse_peak layer=3");    
   if( !Hit_pulse_peak_layer3) { cout << "Can't find Hit pulse_peak layer=3!" << endl; return; }
   TH1I *Hit_pulse_peak_layer4 = (TH1I*)gDirectory->FindObjectAny("Hit pulse_peak layer=4");    
   if( !Hit_pulse_peak_layer4) { cout << "Can't find Hit pulse_peak layer=4!" << endl; return; }

   TH1I *Hit_integral_layer1 = (TH1I*)gDirectory->FindObjectAny("Hit integral layer=1");    
   if( !Hit_integral_layer1) { cout << "Can't find Hit integral layer=1!" << endl; return; }
   TH1I *Hit_integral_layer2 = (TH1I*)gDirectory->FindObjectAny("Hit integral layer=2");    
   if( !Hit_integral_layer2) { cout << "Can't find Hit integral layer=2!" << endl; return; }
   TH1I *Hit_integral_layer3 = (TH1I*)gDirectory->FindObjectAny("Hit integral layer=3");    
   if( !Hit_integral_layer3) { cout << "Can't find Hit integral layer=3!" << endl; return; }
   TH1I *Hit_integral_layer4 = (TH1I*)gDirectory->FindObjectAny("Hit integral layer=4");    
   if( !Hit_integral_layer4) { cout << "Can't find Hit integral layer=4!" << endl; return; }

   TString histfile_name2= "hd_root_particle_gun_Photons_gensat_correct_030736_000";
   TFile *histfile_nocorr = new TFile(histfile_name2+".root","read");
   TH1I *Eshower_nocorr = (TH1I*)gDirectory->FindObjectAny("Eshower"); 

   // define derivative histograms

   int nbins=100;
   TH1D *Eshower_diff = new TH1D("Eshower_diff","Eshower (Corr - Uncorr)",10*nbins,0,10);
   Eshower_diff->Sumw2();
   Eshower_diff->Add(Eshower,Eshower_nocorr,1.,-1.);
   TH1D *Eshower_ave = new TH1D("Eshower_ave","Eshower (Corr+Uncorr)/2",10*nbins,0,10);
   Eshower_ave->Add(Eshower,Eshower_nocorr,0.5,0.5);
   Eshower_ave->Sumw2();
   TH1D *Eshower_diff_norm = new TH1D("Eshower_diff_norm","Eshower Diff/Ave",10*nbins,0,10);
   Eshower_diff_norm->Divide(Eshower_diff,Eshower_ave);
   Eshower_diff_norm->Sumw2();
     
    //
   TCanvas *c1 = new TCanvas("c1","c1 BCAL_SiPM_saturation",200,10,700,700);
   c1->Divide(2,2);
    
     c1->cd(1);
    Ethrown->SetTitle(title);
    // Ethrown->GetXaxis()->SetRangeUser(xmin,xmax);
    // Ethrown->GetYaxis()->SetRangeUser(ymin,ymax);
    Ethrown->GetXaxis()->SetTitleSize(0.05);
    Ethrown->GetYaxis()->SetTitleSize(0.05);
    Ethrown->GetYaxis()->SetTitle("Events");
    Ethrown->GetXaxis()->SetTitle("Ethrown");
    Ethrown->SetLineColor(2);
    Ethrown->Draw("hist");

     c1->cd(2);
    Eshower->SetTitle(title);
    // Eshower->GetXaxis()->SetRangeUser(xmin,xmax);
    // Eshower->GetYaxis()->SetRangeUser(ymin,ymax);
    Eshower->GetXaxis()->SetTitleSize(0.05);
    Eshower->GetYaxis()->SetTitleSize(0.05);
    Eshower->GetYaxis()->SetTitle("Events");
    Eshower->GetXaxis()->SetTitle("Eshower");
    Eshower->SetLineColor(2);
    Eshower->Draw("hist");
    // Eshower_nocorr->Draw("histsame");
    // Eshower_nocorr->SetLineColor(4);

     c1->cd(3);
    ThrownTheta->SetTitle(title);
    // ThrownTheta->GetXaxis()->SetRangeUser(xmin,xmax);
    // ThrownTheta->GetYaxis()->SetRangeUser(ymin,ymax);
    ThrownTheta->GetXaxis()->SetTitleSize(0.05);
    ThrownTheta->GetYaxis()->SetTitleSize(0.05);
    ThrownTheta->GetYaxis()->SetTitle("Events");
    ThrownTheta->GetXaxis()->SetTitle("ThrownTheta");
    ThrownTheta->SetLineColor(2);
    ThrownTheta->Draw("hist");

     c1->cd(4);
     layer->SetTitle(title);
    // layer->GetXaxis()->SetRangeUser(xmin,xmax);
    // layer->GetYaxis()->SetRangeUser(ymin,ymax);
    layer->GetXaxis()->SetTitleSize(0.05);
    layer->GetYaxis()->SetTitleSize(0.05);
    layer->GetYaxis()->SetTitle("Events");
    layer->GetXaxis()->SetTitle("layer");
    layer->SetLineColor(2);
    layer->Draw("hist");

   
   TCanvas *c2 = new TCanvas("c2","c2 BCAL_SiPM_saturation",200,10,700,700);
   c2->Divide(2,2);
    
     c2->cd(1);
    NCell->SetTitle("");
    // NCell->GetXaxis()->SetRangeUser(xmin,xmax);
    // NCell->GetYaxis()->SetRangeUser(ymin,ymax);
    NCell->GetXaxis()->SetTitleSize(0.05);
    NCell->GetYaxis()->SetTitleSize(0.05);
    NCell->GetYaxis()->SetTitle("Events");
    NCell->GetXaxis()->SetTitle("NCell");
    NCell->SetLineColor(2);
    NCell->Draw("hist");

     c2->cd(2);
     gPad->SetLogy();
    Ept->SetTitle("");
    // Ept->GetXaxis()->SetRangeUser(xmin,xmax);
    // Ept->GetYaxis()->SetRangeUser(ymin,ymax);
    Ept->GetXaxis()->SetTitleSize(0.05);
    Ept->GetYaxis()->SetTitleSize(0.05);
    Ept->GetYaxis()->SetTitle("Events");
    Ept->GetXaxis()->SetTitle("Ept");
    Ept->SetLineColor(2);
    Ept->Draw("hist");

     c2->cd(3);
    Eshower_nocorr->SetTitle("");
    // Eshower_nocorr->GetXaxis()->SetRangeUser(xmin,xmax);
    // Eshower_nocorr->GetYaxis()->SetRangeUser(ymin,ymax);
    Eshower_nocorr->GetXaxis()->SetTitleSize(0.05);
    Eshower_nocorr->GetYaxis()->SetTitleSize(0.05);
    Eshower_nocorr->GetYaxis()->SetTitle("Events");
    Eshower_nocorr->GetXaxis()->SetTitle("Eshower NO correction ");
    Eshower_nocorr->SetLineColor(2);
    Eshower_nocorr->Draw("hist");

     c2->cd(4);
     gPad->SetLogy();
    EcalcDiff->SetTitle("");
    // EcalcDiff->GetXaxis()->SetRangeUser(xmin,xmax);
    // EcalcDiff->GetYaxis()->SetRangeUser(ymin,ymax);
    EcalcDiff->GetXaxis()->SetTitleSize(0.05);
    EcalcDiff->GetYaxis()->SetTitleSize(0.05);
    EcalcDiff->GetYaxis()->SetTitle("Events");
    EcalcDiff->GetXaxis()->SetTitle("EcalcDiff");
    EcalcDiff->SetLineColor(2);
    EcalcDiff->SetNdivisions(505);
    EcalcDiff->Draw("hist");


   TCanvas *c3 = new TCanvas("c3","c3 BCAL_SiPM_saturation",200,10,700,700);
   c3->Divide(2,2);
    
     c3->cd(1);
    Eshower_vs_Ethrown->SetTitle(title);
    // Eshower_vs_Ethrown->GetXaxis()->SetRangeUser(xmin,xmax);
    // Eshower_vs_Ethrown->GetYaxis()->SetRangeUser(ymin,ymax);
    Eshower_vs_Ethrown->GetXaxis()->SetTitleSize(0.05);
    Eshower_vs_Ethrown->GetYaxis()->SetTitleSize(0.05);
    Eshower_vs_Ethrown->GetYaxis()->SetTitle("Eshower");
    Eshower_vs_Ethrown->GetXaxis()->SetTitle("Ethrown");
    Eshower_vs_Ethrown->SetLineColor(2);
    Eshower_vs_Ethrown->Draw("colz");

     c3->cd(2);
     // gPad->SetLogy();
    EDiff_vs_Ethrown->SetTitle(title);
    // EDiff_vs_Ethrown->GetXaxis()->SetRangeUser(xmin,xmax);
    // EDiff_vs_Ethrown->GetYaxis()->SetRangeUser(ymin,ymax);
    EDiff_vs_Ethrown->GetXaxis()->SetTitleSize(0.05);
    EDiff_vs_Ethrown->GetYaxis()->SetTitleSize(0.05);
    EDiff_vs_Ethrown->GetYaxis()->SetTitle("EDiff");
    EDiff_vs_Ethrown->GetXaxis()->SetTitle("Ethrown");
    EDiff_vs_Ethrown->SetLineColor(2);
    EDiff_vs_Ethrown->Draw("colz");

     c3->cd(3);
     // gPad->SetLogy();
    EDiffoverEthrown_vs_Ethrown->SetTitle(title);
    // EDiff overEThrown_vs_Ethrown->GetXaxis()->SetRangeUser(xmin,xmax);
    // EDiff overEThrown_vs_Ethrown->GetYaxis()->SetRangeUser(ymin,ymax);
    EDiffoverEthrown_vs_Ethrown->GetXaxis()->SetTitleSize(0.05);
    EDiffoverEthrown_vs_Ethrown->GetYaxis()->SetTitleSize(0.05);
    EDiffoverEthrown_vs_Ethrown->GetXaxis()->SetTitle("Events");
    EDiffoverEthrown_vs_Ethrown->GetYaxis()->SetTitle("EDiff/EThrown_vs_Ethrown");
    EDiffoverEthrown_vs_Ethrown->SetLineColor(2);
    EDiffoverEthrown_vs_Ethrown->Draw("colz");

    
   TCanvas *c4 = new TCanvas("c4","c4 BCAL_SiPM_saturation",200,10,700,700);
   c4->Divide(2,2);
    
     c4->cd(1);
     gPad->SetLogy();
    Hit_pulse_peak_layer1->SetTitle("");
    // Hit_pulse_peak_layer1->GetXaxis()->SetRangeUser(xmin,xmax);
    // Hit_pulse_peak_layer1->GetYaxis()->SetRangeUser(ymin,ymax);
    Hit_pulse_peak_layer1->GetXaxis()->SetTitleSize(0.05);
    Hit_pulse_peak_layer1->GetYaxis()->SetTitleSize(0.05);
    Hit_pulse_peak_layer1->GetYaxis()->SetTitle("Events");
    Hit_pulse_peak_layer1->GetXaxis()->SetTitle("Hit Peak, Layer 1(FADC counts)");
    Hit_pulse_peak_layer1->SetLineColor(2);
    Hit_pulse_peak_layer1->Draw("");
    
     c4->cd(2);
     gPad->SetLogy();
    Hit_pulse_peak_layer2->SetTitle("");
    // Hit_pulse_peak_layer2->GetXaxis()->SetRangeUser(xmin,xmax);
    // Hit_pulse_peak_layer2->GetYaxis()->SetRangeUser(ymin,ymax);
    Hit_pulse_peak_layer2->GetXaxis()->SetTitleSize(0.05);
    Hit_pulse_peak_layer2->GetYaxis()->SetTitleSize(0.05);
    Hit_pulse_peak_layer2->GetYaxis()->SetTitle("Events");
    Hit_pulse_peak_layer2->GetXaxis()->SetTitle("Hit Peak, Layer 2(FADC counts)");
    Hit_pulse_peak_layer2->SetLineColor(2);
    Hit_pulse_peak_layer2->Draw("");
    
     c4->cd(3);
     gPad->SetLogy();
    Hit_pulse_peak_layer3->SetTitle("");
    // Hit_pulse_peak_layer3->GetXaxis()->SetRangeUser(xmin,xmax);
    // Hit_pulse_peak_layer3->GetYaxis()->SetRangeUser(ymin,ymax);
    Hit_pulse_peak_layer3->GetXaxis()->SetTitleSize(0.05);
    Hit_pulse_peak_layer3->GetYaxis()->SetTitleSize(0.05);
    Hit_pulse_peak_layer3->GetYaxis()->SetTitle("Events");
    Hit_pulse_peak_layer3->GetXaxis()->SetTitle("Hit Peak, Layer 3(FADC counts)");
    Hit_pulse_peak_layer3->SetLineColor(2);
    Hit_pulse_peak_layer3->Draw("");
    
     c4->cd(4);
     gPad->SetLogy();
    Hit_pulse_peak_layer4->SetTitle("");
    // Hit_pulse_peak_layer4->GetXaxis()->SetRangeUser(xmin,xmax);
    // Hit_pulse_peak_layer4->GetYaxis()->SetRangeUser(ymin,ymax);
    Hit_pulse_peak_layer4->GetXaxis()->SetTitleSize(0.05);
    Hit_pulse_peak_layer4->GetYaxis()->SetTitleSize(0.05);
    Hit_pulse_peak_layer4->GetYaxis()->SetTitle("Events");
    Hit_pulse_peak_layer4->GetXaxis()->SetTitle("Hit Peak, Layer 4(FADC counts)");
    Hit_pulse_peak_layer4->SetLineColor(2);
    Hit_pulse_peak_layer4->Draw("");  


   TCanvas *c5 = new TCanvas("c5","c5 BCAL_SiPM_saturation",200,10,700,700);
   c5->Divide(2,2);
    
     c5->cd(1);
     gPad->SetLogy();
    Hit_integral_layer1->SetTitle("");
    // Hit_integral_layer1->GetXaxis()->SetRangeUser(xmin,xmax);
    // Hit_integral_layer1->GetYaxis()->SetRangeUser(ymin,ymax);
    Hit_integral_layer1->GetXaxis()->SetTitleSize(0.05);
    Hit_integral_layer1->GetYaxis()->SetTitleSize(0.05);
    Hit_integral_layer1->GetYaxis()->SetTitle("Events");
    Hit_integral_layer1->GetXaxis()->SetTitle("Hit Integral, Layer 1(GeV)");
    Hit_integral_layer1->SetLineColor(2);
    Hit_integral_layer1->Draw("");
    
     c5->cd(2);
     gPad->SetLogy();
    Hit_integral_layer2->SetTitle("");
    // Hit_integral_layer2->GetXaxis()->SetRangeUser(xmin,xmax);
    // Hit_integral_layer2->GetYaxis()->SetRangeUser(ymin,ymax);
    Hit_integral_layer2->GetXaxis()->SetTitleSize(0.05);
    Hit_integral_layer2->GetYaxis()->SetTitleSize(0.05);
    Hit_integral_layer2->GetYaxis()->SetTitle("Events");
    Hit_integral_layer2->GetXaxis()->SetTitle("Hit Integral, Layer 2(GeV)");
    Hit_integral_layer2->SetLineColor(2);
    Hit_integral_layer2->Draw("");
    
     c5->cd(3);
     gPad->SetLogy();
    Hit_integral_layer3->SetTitle("");
    // Hit_integral_layer3->GetXaxis()->SetRangeUser(xmin,xmax);
    // Hit_integral_layer3->GetYaxis()->SetRangeUser(ymin,ymax);
    Hit_integral_layer3->GetXaxis()->SetTitleSize(0.05);
    Hit_integral_layer3->GetYaxis()->SetTitleSize(0.05);
    Hit_integral_layer3->GetYaxis()->SetTitle("Events");
    Hit_integral_layer3->GetXaxis()->SetTitle("Hit Integral, Layer 3(GeV)");
    Hit_integral_layer3->SetLineColor(2);
    Hit_integral_layer3->Draw("");
    
     c5->cd(4);
     gPad->SetLogy();
    Hit_integral_layer4->SetTitle("");
    // Hit_integral_layer4->GetXaxis()->SetRangeUser(xmin,xmax);
    // Hit_integral_layer4->GetYaxis()->SetRangeUser(ymin,ymax);
    Hit_integral_layer4->GetXaxis()->SetTitleSize(0.05);
    Hit_integral_layer4->GetYaxis()->SetTitleSize(0.05);
    Hit_integral_layer4->GetYaxis()->SetTitle("Events");
    Hit_integral_layer4->GetXaxis()->SetTitle("Hit Integral, Layer 4(GeV)");
    Hit_integral_layer4->SetLineColor(2);
    Hit_integral_layer4->Draw("");    //

   TCanvas *c6 = new TCanvas("c6","c6 BCAL_SiPM_saturation",200,10,700,700);
   // c6->Divide(2,2);
   // c6->cd(1);
    Eshower->SetTitle("");
    // Eshower->GetXaxis()->SetRangeUser(xmin,xmax);
    // Eshower->GetYaxis()->SetRangeUser(ymin,ymax);
    Eshower->GetXaxis()->SetTitleSize(0.05);
    Eshower->GetYaxis()->SetTitleSize(0.05);
    Eshower->GetYaxis()->SetTitle("Events");
    Eshower->GetXaxis()->SetTitle("Eshower");
    Eshower->SetLineColor(2);
    Eshower->Draw("hist");
    Eshower_nocorr->Draw("histsame");
    Eshower_nocorr->SetLineColor(4);
    
    TCanvas *c7 = new TCanvas("c7","c7 BCAL_SiPM_saturation",200,10,700,700);
    c7->Divide(2,2);

    c7->cd(1);
    gPad->SetLogy();
    Eshower_ave->SetTitle("");
    // Eshower_ave->GetXaxis()->SetRangeUser(xmin,xmax);
    // Eshower_ave->GetYaxis()->SetRangeUser(ymin,ymax);
    Eshower_ave->GetXaxis()->SetTitleSize(0.05);
    Eshower_ave->GetYaxis()->SetTitleSize(0.05);
    Eshower_ave->GetYaxis()->SetTitle("Events");
    Eshower_ave->GetXaxis()->SetTitle("Eshower Ave (Corr+UnCorr)/2");
    Eshower_ave->SetLineColor(2);
    Eshower_ave->RebinX(40);
    Eshower_ave->Draw("hist");

    c7->cd(2);
    Eshower_diff->SetTitle("");
    // Eshower_diff->GetXaxis()->SetRangeUser(xmin,xmax);
    // Eshower_diff->GetYaxis()->SetRangeUser(ymin,ymax);
    Eshower_diff->GetXaxis()->SetTitleSize(0.05);
    Eshower_diff->GetYaxis()->SetTitleSize(0.05);
    Eshower_diff->GetYaxis()->SetTitle("Events");
    Eshower_diff->GetXaxis()->SetTitle("Eshower Diff (Corr-UnCorr)");
    Eshower_diff->SetLineColor(2);
    Eshower_diff->RebinX(40);
    Eshower_diff->Draw("hist");

    c7->cd(3);
    Eshower_diff_norm->SetTitle("");
    // Eshower_diff_norm->GetXaxis()->SetRangeUser(xmin,xmax);
    // Eshower_diff_norm->GetYaxis()->SetRangeUser(ymin,ymax);
    Eshower_diff_norm->GetXaxis()->SetTitleSize(0.05);
    Eshower_diff_norm->GetYaxis()->SetTitleSize(0.05);
    Eshower_diff_norm->GetYaxis()->SetTitle("Fraction");
    Eshower_diff_norm->GetXaxis()->SetTitle("Eshower Diff/Ave");
    Eshower_diff_norm->SetLineColor(2);
    Eshower_diff_norm->RebinX(40);
    Eshower_diff_norm->Draw("hist");



     
    //
   TCanvas *c8 = new TCanvas("c8","c8 BCAL_SiPM_saturation",200,10,700,700);
   c8->Divide(2,2);


   
     c8->cd(1);
     gPad->SetLogy();
    Eshower->SetTitle(title);
    // Eshower->GetXaxis()->SetRangeUser(xmin,xmax);
    // Eshower->GetYaxis()->SetRangeUser(ymin,ymax);
    Eshower->GetXaxis()->SetTitleSize(0.05);
    Eshower->GetYaxis()->SetTitleSize(0.05);
    Eshower->GetYaxis()->SetTitle("Events");
    Eshower->GetXaxis()->SetTitle("Eshower (GeV)");
    Eshower->SetLineColor(2);
    Eshower->Draw("hist");
    // Eshower_nocorr->Draw("histsame");
    // Eshower_nocorr->SetLineColor(4);


     c8->cd(2);
    NCell->SetTitle("");
    // NCell->GetXaxis()->SetRangeUser(xmin,xmax);
    // NCell->GetYaxis()->SetRangeUser(ymin,ymax);
    NCell->GetXaxis()->SetTitleSize(0.05);
    NCell->GetYaxis()->SetTitleSize(0.05);
    NCell->GetYaxis()->SetTitle("Events");
    NCell->GetXaxis()->SetTitle("NCell");
    NCell->SetLineColor(2);
    NCell->Draw("hist");

     c8->cd(3);
     gPad->SetLogy();
    Ept->SetTitle("");
    // Ept->GetXaxis()->SetRangeUser(xmin,xmax);
    // Ept->GetYaxis()->SetRangeUser(ymin,ymax);
    Ept->GetXaxis()->SetTitleSize(0.05);
    Ept->GetYaxis()->SetTitleSize(0.05);
    Ept->GetYaxis()->SetTitle("Events");
    Ept->GetXaxis()->SetTitle("Ept (GeV)");
    Ept->SetLineColor(2);
    Ept->Draw("hist");


     c8->cd(4);
     layer->SetTitle(title);
    // layer->GetXaxis()->SetRangeUser(xmin,xmax);
    // layer->GetYaxis()->SetRangeUser(ymin,ymax);
    layer->GetXaxis()->SetTitleSize(0.05);
    layer->GetYaxis()->SetTitleSize(0.05);
    layer->GetYaxis()->SetTitle("Events");
    layer->GetXaxis()->SetTitle("layer");
    layer->SetLineColor(2);
    layer->Draw("hist");




    
    /*TLatex *t1 = new TLatex(0.1,0.92,"");
    t1->SetNDC();
    t1->DrawLatex(0.2,0.82,"b)");
    t1->DrawLatex(0.2,0.75,"Four Configurations");
    t1->SetTextSize(0.07);*/
    
    
    c1->SaveAs(histfile_name+".pdf(");
    c2->SaveAs(histfile_name+".pdf");
    c3->SaveAs(histfile_name+".pdf");
    c4->SaveAs(histfile_name+".pdf");
    c5->SaveAs(histfile_name+".pdf");
    c6->SaveAs(histfile_name+".pdf");
    c7->SaveAs(histfile_name+".pdf");
    c8->SaveAs(histfile_name+".pdf)");
    
   // in->Close();

}

