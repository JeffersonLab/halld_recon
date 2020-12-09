void twopi_primakoff(TString filename, Int_t maxev=100000)
{
// File: twopi_primakoff.C
    // Output histograms and fits generated from amp fitting of parameters.
//

  gStyle->SetPalette(1,0);
  // gStyle->SetOptStat(111111);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(111111);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);

  char string[256];
    vector <TString> sdme;
    
    /*const Int_t nparms=8;
    Double_t parms[nparms];
    Double_t parms_err[nparms];*/
    vector <double> parms;
    vector <double> parms_err;
    
    bool setscale(true);
    
    // TString filename = "twopi_primakoff_gen";
    // Double_t scale_factor=0.5;       // divide ymax/scale_factor
    // TString filename = "twopi_primakoff_DSelect";
    // TString filename = "twopi_primakoff_DSelect_thrown_mod";
    // TString filename = "twopi_primakoff_DSelect_thrown_mod2";
    Double_t scale_factor=6000000/(float)maxev;       // divide ymax/scale_factor
    cout << "Use scale_factor=" << scale_factor << " maxev=" << maxev << " setscale=" << setscale << endl;
    
    TString infile = filename+".fit2";   // file with parameters
    TFile *f = new TFile(filename+".root","read");
    
    cout << "Opening parameters file: " << infile.Data() << endl;
    cout << "Opening root file: " << (filename+".root").Data() << endl;
    
    
    TH1F *M2pigen = (TH1F*)f->Get("M2pigen");
    TH1F *M2piacc = (TH1F*)f->Get("M2piacc");
    TH1F *M2piacc_g1V00 = (TH1F*)f->Get("M2piacc_g1V00");
    TH1F *M2piacc_g1V00s = (TH1F*)f->Get("M2piacc_g1V00s");
    TH1F *M2piacc_g1V11 = (TH1F*)f->Get("M2piacc_g1V11");
    TH1F *M2piacc_g1V10 = (TH1F*)f->Get("M2piacc_g1V10");
    TH1F *M2piacc_g1V1M1 = (TH1F*)f->Get("M2piacc_g1V1-1");
    TH1F *M2pidat = (TH1F*)f->Get("M2pidat");
    TH1F *M2pibkgnd = (TH1F*)f->Get("M2pibkgnd");
    TH1F *M2pidatsub = (TH1F*)M2pidat->Clone("M2pidatsub");
    M2pidatsub->Add(M2pibkgnd,-1);

    TH1F *theta_scatgen = (TH1F*)f->Get("theta_scatgen");
    TH1F *theta_scatacc = (TH1F*)f->Get("theta_scatacc");
    TH1F *theta_scatacc_g1V00 = (TH1F*)f->Get("theta_scatacc_g1V00");
    TH1F *theta_scatacc_g1V00s = (TH1F*)f->Get("theta_scatacc_g1V00s");
    TH1F *theta_scatacc_g1V11 = (TH1F*)f->Get("theta_scatacc_g1V11");
    TH1F *theta_scatacc_g1V10 = (TH1F*)f->Get("theta_scatacc_g1V10");
      TH1F *theta_scatacc_g1V1M1 = (TH1F*)f->Get("theta_scatacc_g1V1-1");
    TH1F *theta_scatdat = (TH1F*)f->Get("theta_scatdat");
    TH1F *theta_scatbkgnd = (TH1F*)f->Get("theta_scatbkgnd");
    TH1F *theta_scatdatsub = (TH1F*)theta_scatdat->Clone("theta_scatdatsub");
    theta_scatdatsub->Add(theta_scatbkgnd,-1);
    
    
    TH1F *cosThetagen = (TH1F*)f->Get("cosThetagen");
    TH1F *cosThetaacc = (TH1F*)f->Get("cosThetaacc");
    TH1F *cosThetaacc_g1V00 = (TH1F*)f->Get("cosThetaacc_g1V00");
    TH1F *cosThetaacc_g1V00s = (TH1F*)f->Get("cosThetaacc_g1V00s");
    TH1F *cosThetaacc_g1V11 = (TH1F*)f->Get("cosThetaacc_g1V11");
    TH1F *cosThetaacc_g1V10 = (TH1F*)f->Get("cosThetaacc_g1V10");
      TH1F *cosThetaacc_g1V1M1 = (TH1F*)f->Get("cosThetaacc_g1V1-1");
    TH1F *cosThetadat = (TH1F*)f->Get("cosThetadat");
    TH1F *cosThetabkgnd = (TH1F*)f->Get("cosThetabkgnd");
    TH1F *cosThetadatsub = (TH1F*)cosThetadat->Clone("cosThetadatsub");
    cosThetadatsub->Add(cosThetabkgnd,-1);
    
    TH1F *psigen = (TH1F*)f->Get("psigen");
    TH1F *psiacc = (TH1F*)f->Get("psiacc");
    TH1F *psidat = (TH1F*)f->Get("psidat");
    TH1F *psibkgnd = (TH1F*)f->Get("psibkgnd");
    TH1F *psidatsub = (TH1F*)psidat->Clone("psidatsub");
    psidatsub->Add(psibkgnd,-1);
    
    TH1F *Phigen = (TH1F*)f->Get("Phigen");
    TH1F *Phiacc = (TH1F*)f->Get("Phiacc");
    TH1F *Phiacc_g1V00 = (TH1F*)f->Get("Phiacc_g1V00");
    TH1F *Phiacc_g1V00s = (TH1F*)f->Get("Phiacc_g1V00s");
    TH1F *Phiacc_g1V1M1 = (TH1F*)f->Get("Phiacc_g1V1-1");
      TH1F *Phiacc_g1V11 = (TH1F*)f->Get("Phiacc_g1V11");
      TH1F *Phiacc_g1V10 = (TH1F*)f->Get("Phiacc_g1V10");
    TH1F *Phidat = (TH1F*)f->Get("Phidat");
    TH1F *Phibkgnd = (TH1F*)f->Get("Phibkgnd");
    TH1F *Phidatsub = (TH1F*)Phidat->Clone("Phidatsub");
    Phidatsub->Add(Phibkgnd,-1);
    
    TH1F *phigen = (TH1F*)f->Get("phigen");
    TH1F *phiacc = (TH1F*)f->Get("phiacc");
    TH1F *phidat = (TH1F*)f->Get("phidat");
    TH1F *phibkgnd = (TH1F*)f->Get("phibkgnd");
    TH1F *phidatsub = (TH1F*)phidat->Clone("phidatsub");
    phidatsub->Add(phibkgnd,-1);
    
    TH1F *tgen = (TH1F*)f->Get("tgen");
    TH1F *tacc = (TH1F*)f->Get("tacc");
    TH1F *tdat = (TH1F*)f->Get("tdat");
    TH1F *tbkgnd = (TH1F*)f->Get("tbkgnd");
    TH1F *tdatsub = (TH1F*)tdat->Clone("tdatsub");
    tdatsub->Add(tbkgnd,-1);
    
    
   TCanvas *c0 = new TCanvas("c0", "c0",200,10,1000,700);

   c0->Divide(3,2);
    c0->cd(1);
    // gPad->SetLogy();
    Double_t xmin = 0.2;
    Double_t xmax = 0.8;
    Double_t ymin = 0/scale_factor;
    Double_t ymax = 8000/scale_factor;
        
    M2pigen->SetTitle(filename);
    M2pigen->GetXaxis()->SetRangeUser(xmin,xmax);
    M2pigen->GetYaxis()->SetRangeUser(ymin,ymax);
    M2pigen->GetXaxis()->SetTitleSize(0.05);
    M2pigen->GetYaxis()->SetTitleSize(0.05);
    M2pigen->GetXaxis()->SetTitle("M_{#pi#pi} (GeV)");
    M2pigen->SetMarkerColor(4);
    // M2piacc->Draw("samep");
    M2pidat->SetMarkerColor(1);
    M2pidat->SetLineColor(1);
    M2pidat->SetMarkerStyle(20);
    M2pidat->SetMarkerSize(0.1);
    M2pibkgnd->SetMarkerStyle(20);
    M2pibkgnd->SetMarkerSize(0.1);
    M2pibkgnd->SetMarkerColor(3);
    M2pibkgnd->SetLineColor(3);
    M2pigen->Draw("p");
    M2pidat->Draw("samep");
    M2pibkgnd->Draw("samep");
    
    TLegend *leg = new TLegend(0.4,0.65,0.75,0.85);
    leg->AddEntry(M2pigen,"Gen","lp");
    leg->AddEntry(M2pidat,"Data","lp");
    leg->AddEntry(M2pibkgnd,"Accidentals","lp");
    leg->Draw();
    
    c0->cd(2);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 20000/scale_factor;
    
    cosThetagen->SetTitle(filename);
    // cosThetagen->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) cosThetagen->GetYaxis()->SetRangeUser(ymin,ymax);
    cosThetagen->GetXaxis()->SetTitleSize(0.05);
    cosThetagen->GetYaxis()->SetTitleSize(0.05);
    cosThetagen->GetXaxis()->SetTitle("cos(#theta_{#pi})");
    cosThetagen->SetLineColor(4);
    cosThetagen->Draw("p");
    // cosThetaacc->Draw("samep");
    cosThetadat->SetMarkerColor(1);
    cosThetadat->SetLineColor(1);
    cosThetadat->SetMarkerStyle(20);
    cosThetadat->SetMarkerSize(0.1);
    cosThetabkgnd->SetMarkerStyle(20);
    cosThetabkgnd->SetMarkerSize(0.1);
    cosThetabkgnd->SetMarkerColor(3);
    cosThetabkgnd->SetLineColor(3);
    cosThetadat->Draw("samep");
    cosThetabkgnd->Draw("samep");
    
    c0->cd(5);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 20000/scale_factor;
    
    TF1 *cos2phi = new TF1("cos2phi","[0]*(1+[1]*cos(2*x))",-3.14159,3.14159);
    TF1 *cosphi = new TF1("cosphi","[0]+[1]*cos(x)",-3.14159,3.14159);
    
    psigen->SetTitle(filename);
    // psigen->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) psigen->GetYaxis()->SetRangeUser(ymin,ymax);
    psigen->GetXaxis()->SetTitleSize(0.05);
    psigen->GetYaxis()->SetTitleSize(0.05);
    psigen->GetXaxis()->SetTitle("#psi");
    psigen->SetMarkerColor(4);
    // psiacc->Draw("samep");
    psidat->SetMarkerColor(1);
    psidat->SetLineColor(1);
    psidat->SetMarkerStyle(20);
    psidat->SetMarkerSize(0.1);
    psidat->Fit(cos2phi);
    psibkgnd->SetMarkerStyle(20);
    psibkgnd->SetMarkerSize(0.1);
    psibkgnd->SetMarkerColor(3);
    psibkgnd->SetLineColor(3);
    psigen->Draw("p");
    psidat->Draw("samep");
    psibkgnd->Draw("samep");
    
    c0->cd(3);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 20000/scale_factor;
    
    Phigen->SetTitle(filename);
    // Phigen->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) Phigen->GetYaxis()->SetRangeUser(ymin,ymax);
    Phigen->GetXaxis()->SetTitleSize(0.05);
    Phigen->GetYaxis()->SetTitleSize(0.05);
    Phigen->GetXaxis()->SetTitle("#phi_{#pi#pi} (rad)");
    Phigen->SetMarkerColor(4);
    // Phiacc->Draw("samep");
    Phidat->SetMarkerColor(1);
    Phidat->SetLineColor(1);
    Phidat->SetMarkerStyle(20);
    Phidat->SetMarkerSize(0.1);
    Phidat->Fit(cos2phi);
    Phibkgnd->SetMarkerStyle(20);
    Phibkgnd->SetMarkerSize(0.1);
    Phibkgnd->SetMarkerColor(3);
    Phibkgnd->SetLineColor(3);
    Phigen->Draw("p");
    Phidat->Draw("samep");
    Phibkgnd->Draw("samep");
    
    c0->cd(4);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 20000/scale_factor;
    
    phigen->SetTitle(filename);
    // phigen->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) phigen->GetYaxis()->SetRangeUser(ymin,ymax);
    phigen->GetXaxis()->SetTitleSize(0.05);
    phigen->GetYaxis()->SetTitleSize(0.05);
    phigen->GetXaxis()->SetTitle("#phi");
    phigen->SetMarkerColor(4);
    // phiacc->Draw("samep");
    phidat->SetMarkerColor(1);
    phidat->SetLineColor(1);
    phidat->SetMarkerStyle(20);
    phidat->SetMarkerSize(0.1);
    phidat->Fit(cosphi);
    phibkgnd->SetMarkerStyle(20);
    phibkgnd->SetMarkerSize(0.1);
    phibkgnd->SetMarkerColor(3);
    phibkgnd->SetLineColor(3);
    phigen->Draw("p");
    phidat->Draw("samep");
    phibkgnd->Draw("samep");
    
    c0->cd(6);
    gPad->SetLogy(); // use  gPad->SetLogy();
    xmin = 0;
    xmax = 0.012;
    ymin = 100/scale_factor;
    ymax = 400000/scale_factor;
    
    tdat->SetTitle(filename);
    tdat->GetXaxis()->SetTitleSize(0.05);
    tdat->GetYaxis()->SetTitleSize(0.05);
    tdat->GetXaxis()->SetTitle("-t (GeV^{2})");
    tgen->GetXaxis()->SetTitle("-t (GeV^{2})");
    // tacc->Draw("samep");
    tdat->GetXaxis()->SetRangeUser(xmin,xmax);
    tdat->GetYaxis()->SetRangeUser(ymin,ymax);
    tdat->SetMarkerColor(1);
    tdat->SetLineColor(1);
    tdat->SetMarkerStyle(20);
    tdat->SetMarkerSize(0.1);
    tdat->Fit("expo","","",0.002,0.01);
    tdat->Draw("p");
    tgen->SetMarkerColor(4);
    tbkgnd->SetMarkerStyle(20);
    tbkgnd->SetMarkerSize(0.1);
    tbkgnd->SetMarkerColor(3);
    tbkgnd->SetLineColor(3);
    tgen->Draw("samep");
    tbkgnd->Draw("samep");
    
    
    TCanvas *c2 = new TCanvas("c2", "c2",200,10,1000,700);
    
    c2->Divide(3,2);
    c2->cd(1);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 1.2;
    
    // TH1F *M2piAcceptance = (TH1F*)M2piacc->Clone("M2piAcceptance");
    TH1F *M2piAcceptance = (TH1F*)M2pidat->Clone("M2piAcceptance");
    M2piAcceptance->SetTitle("Acceptance");
    // M2piAcceptance->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) M2piAcceptance->GetYaxis()->SetRangeUser(ymin,ymax);
    M2piAcceptance->Divide(M2pigen);
    M2piAcceptance->GetXaxis()->SetTitleSize(0.05);
    M2piAcceptance->GetYaxis()->SetTitleSize(0.05);
    M2piAcceptance->GetXaxis()->SetTitle("M_{#pi#pi} (GeV)");
    M2piAcceptance->SetLineColor(2);
    M2piAcceptance->Draw("p");
    
    c2->cd(2);
    // gPad->SetLogy();
    ymin = 0;
    // ymax = 1.2;
    
    // TH1F *cosThetaAcceptance = (TH1F*)cosThetaacc->Clone("cosThetaAcceptance");
    TH1F *cosThetaAcceptance = (TH1F*)cosThetadat->Clone("cosThetaAcceptance");
    cosThetaAcceptance->SetTitle("Acceptance");
    // cosThetaAcceptance->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) cosThetaAcceptance->GetYaxis()->SetRangeUser(ymin,ymax);
    cosThetaAcceptance->Divide(cosThetagen);
    cosThetaAcceptance->GetXaxis()->SetTitleSize(0.05);
    cosThetaAcceptance->GetYaxis()->SetTitleSize(0.05);
    cosThetaAcceptance->GetXaxis()->SetTitle("cos(#theta)");
    cosThetaAcceptance->SetLineColor(2);
    cosThetaAcceptance->Draw("p");
    
    c2->cd(5);
    // gPad->SetLogy();
    ymin = 0;
    // ymax = 1.2;
    
    // TH1F *psiAcceptance = (TH1F*)psiacc->Clone("psiAcceptance");
    TH1F *psiAcceptance = (TH1F*)psidat->Clone("psiAcceptance");
    psiAcceptance->SetTitle("Acceptance");
    // psiAcceptance->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) psiAcceptance->GetYaxis()->SetRangeUser(ymin,ymax);
    psiAcceptance->Divide(psigen);
    psiAcceptance->GetXaxis()->SetTitleSize(0.05);
    psiAcceptance->GetYaxis()->SetTitleSize(0.05);
    psiAcceptance->GetXaxis()->SetTitle("#psi");
    psiAcceptance->SetLineColor(2);
    psiAcceptance->Draw("p");
    
    c2->cd(3);
    // gPad->SetLogy();
    ymin = 0;
    // ymax = 1.2;
    
    // TH1F *PhiAcceptance = (TH1F*)Phiacc->Clone("PhiAcceptance");
    TH1F *PhiAcceptance = (TH1F*)Phidat->Clone("PhiAcceptance");
    PhiAcceptance->SetTitle("Acceptance");
    // PhiAcceptance->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) PhiAcceptance->GetYaxis()->SetRangeUser(ymin,ymax);
    PhiAcceptance->Divide(Phigen);
    PhiAcceptance->GetXaxis()->SetTitleSize(0.05);
    PhiAcceptance->GetYaxis()->SetTitleSize(0.05);
    PhiAcceptance->GetXaxis()->SetTitle("#phi_{#pi#pi}");
    PhiAcceptance->SetLineColor(2);
    PhiAcceptance->Draw("p");
    
    c2->cd(4);
    // gPad->SetLogy();
    ymin = 0;
    // ymax = 1.2;
    
    // TH1F *phiAcceptance = (TH1F*)phiacc->Clone("phiAcceptance");
    TH1F *phiAcceptance = (TH1F*)phidat->Clone("phiAcceptance");
    phiAcceptance->SetTitle("Acceptance");
    // phiAcceptance->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) phiAcceptance->GetYaxis()->SetRangeUser(ymin,ymax);
    phiAcceptance->Divide(phigen);
    phiAcceptance->GetXaxis()->SetTitleSize(0.05);
    phiAcceptance->GetYaxis()->SetTitleSize(0.05);
    phiAcceptance->GetXaxis()->SetTitle("#phi");
    phiAcceptance->SetLineColor(2);
    phiAcceptance->Draw("p");
    
    c2->cd(6);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 1.2;
    xmin = 0;
    xmax = 0.012;
    
    // TH1F *tAcceptance = (TH1F*)tacc->Clone("tAcceptance");
    TH1F *tAcceptance = (TH1F*)tdat->Clone("tAcceptance");
    tAcceptance->SetTitle("Acceptance");
    tAcceptance->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) tAcceptance->GetYaxis()->SetRangeUser(ymin,ymax);
    tAcceptance->Divide(tgen);
    tAcceptance->GetXaxis()->SetTitleSize(0.05);
    tAcceptance->GetYaxis()->SetTitleSize(0.05);
    tAcceptance->GetXaxis()->SetTitle("-t");
    tAcceptance->SetLineColor(2);
    tAcceptance->Draw("p");
    
    TCanvas *c1 = new TCanvas("c1", "c1",200,10,1000,700);
    
    c1->Divide(3,2);
    c1->cd(1);
    // gPad->SetLogy();
    ymin = 0/scale_factor;
    ymax = 3000/scale_factor;
    
    M2piacc->SetTitle(filename);
    M2piacc->GetXaxis()->SetRangeUser(xmin,xmax);
    M2piacc->GetYaxis()->SetRangeUser(ymin,ymax);
    M2piacc->GetXaxis()->SetTitleSize(0.05);
    M2piacc->GetYaxis()->SetTitleSize(0.05);
    M2piacc->GetXaxis()->SetTitle("M_{#pi#pi}");
    M2piacc->SetMarkerColor(2);
    M2piacc->SetLineColor(2);
    // M2piacc->Draw("samep");
    M2pidat->SetMarkerColor(1);
    M2pidat->SetLineColor(1);
    M2pidat->SetMarkerStyle(20);
    M2pidat->SetMarkerSize(0.1);
    M2pibkgnd->SetMarkerColor(3);
    M2pibkgnd->SetLineColor(3);
    M2pibkgnd->SetMarkerStyle(20);
    M2pibkgnd->SetMarkerSize(0.1);
    M2piacc->Draw("p");
    M2pidat->Draw("samep");
    M2pibkgnd->Draw("samep");
    
    TLegend *leg1 = new TLegend(0.5,0.6,0.7,0.8);
    // leg1->AddEntry(M2pigen,"Gen","lp");
    leg1->AddEntry(M2piacc,"Acc","lp");
    leg1->AddEntry(M2pidat,"Data","lp");
    leg1->AddEntry(M2pibkgnd,"Tag Acc","lp");
    leg1->Draw();
    
    c1->cd(2);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 80000/scale_factor;
    
    cosThetaacc->SetTitle(filename);
    // cosThetaacc->GetXaxis()->SetRangeUser(xmin,xmax);
    // if (setscale) cosThetaacc->GetYaxis()->SetRangeUser(ymin,ymax);
    cosThetaacc->GetYaxis()->SetRangeUser(ymin,ymax);
    cosThetaacc->GetXaxis()->SetTitleSize(0.05);
    cosThetaacc->GetYaxis()->SetTitleSize(0.05);
    cosThetaacc->GetXaxis()->SetTitle("cos(#theta)");
    cosThetaacc->SetLineColor(2);
    cosThetaacc->SetMarkerColor(2);
    cosThetaacc->Draw("p");
    // cosThetaacc->Draw("samep");
    cosThetadat->SetMarkerColor(1);
    cosThetadat->SetLineColor(1);
    cosThetadat->SetMarkerStyle(20);
    cosThetadat->SetMarkerSize(0.1);
    cosThetabkgnd->SetMarkerColor(3);
    cosThetabkgnd->SetLineColor(3);
    cosThetabkgnd->SetMarkerStyle(20);
    cosThetabkgnd->SetMarkerSize(0.1);
    cosThetadat->Draw("samep");
    cosThetabkgnd->Draw("samep");
    
    c1->cd(5);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 8000/scale_factor;
    
    psiacc->SetTitle(filename);
    // psiacc->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) {
      cout << " Setscale: ymax=" << ymax << endl;
      psiacc->GetYaxis()->SetRangeUser(ymin,ymax);
    }
    psiacc->GetXaxis()->SetTitleSize(0.05);
    psiacc->GetYaxis()->SetTitleSize(0.05);
    psiacc->GetXaxis()->SetTitle("#psi");
    psiacc->SetMarkerColor(2);
    psiacc->SetLineColor(2);
    // psiacc->Fit(cos2phi);
    psiacc->Draw("p");
    // psiacc->Draw("samep");
    psidat->SetMarkerColor(1);
    psidat->SetLineColor(1);
    psidat->SetMarkerStyle(20);
    psidat->SetMarkerSize(0.1);
    psibkgnd->SetMarkerColor(3);
    psibkgnd->SetLineColor(3);
    psibkgnd->SetMarkerStyle(20);
    psibkgnd->SetMarkerSize(0.1);
    psidat->Draw("samep");
    psibkgnd->Draw("samep");
    
    c1->cd(3);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 16000/scale_factor;
    
    Phiacc->SetTitle(filename);
    // Phiacc->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) Phiacc->GetYaxis()->SetRangeUser(ymin,ymax);
    Phiacc->GetXaxis()->SetTitleSize(0.05);
    Phiacc->GetYaxis()->SetTitleSize(0.05);
    Phiacc->GetXaxis()->SetTitle("#phi_{#pi#pi}");
    Phiacc->SetMarkerColor(2);
    Phiacc->SetLineColor(2);
    // Phiacc->Fit(cos2phi);
    Phiacc->Draw("p");
    // Phiacc->Draw("samep");
    Phidat->SetMarkerColor(1);
    Phidat->SetLineColor(1);
    Phidat->SetMarkerStyle(20);
    Phidat->SetMarkerSize(0.1);
    Phibkgnd->SetMarkerColor(3);
    Phibkgnd->SetLineColor(3);
    Phibkgnd->SetMarkerStyle(20);
    Phibkgnd->SetMarkerSize(0.1);
    Phidat->Draw("samep");
    Phibkgnd->Draw("samep");
    
    
    c1->cd(4);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 16000/scale_factor;
    
    phiacc->SetTitle(filename);
    // phiacc->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) phiacc->GetYaxis()->SetRangeUser(ymin,ymax);
    phiacc->GetXaxis()->SetTitleSize(0.05);
    phiacc->GetYaxis()->SetTitleSize(0.05);
    phiacc->GetXaxis()->SetTitle("#phi");
    phiacc->SetMarkerColor(2);
    phiacc->SetLineColor(2);
    // phiacc->Fit(cosphi);
    phiacc->Draw("p");
    // phiacc->Draw("samep");
    phidat->SetMarkerColor(1);
    phidat->SetLineColor(1);
    phidat->SetMarkerStyle(20);
    phidat->SetMarkerSize(0.1);
    phibkgnd->SetMarkerColor(3);
    phibkgnd->SetLineColor(3);
    phibkgnd->SetMarkerStyle(20);
    phibkgnd->SetMarkerSize(0.1);
    phidat->Draw("samep");
    phibkgnd->Draw("samep");
    
    c1->cd(6);
    gPad->SetLogy();  // use  gPad->SetLogy();
    xmin = 0;
    xmax = 0.012;
    ymin = 10/scale_factor;
    ymax = 400000/scale_factor;
    
    tdat->SetTitle(filename);
    tdat->GetXaxis()->SetTitleSize(0.05);
    tdat->GetYaxis()->SetTitleSize(0.05);
    tdat->GetXaxis()->SetTitle("-t (GeV^{2})");
    // tacc->Draw("samep");
    tdat->GetXaxis()->SetRangeUser(xmin,xmax);
    tdat->GetYaxis()->SetRangeUser(ymin,ymax);
    tdat->SetMarkerColor(1);
    tdat->SetLineColor(1);
    tdat->SetMarkerStyle(20);
    tdat->SetMarkerSize(0.1);
    tdat->Draw("p");
    tacc->SetMarkerColor(2);
    tacc->SetLineColor(2);
    tbkgnd->SetMarkerColor(3);
    tbkgnd->SetLineColor(3);
    tbkgnd->SetMarkerStyle(20);
    tbkgnd->SetMarkerSize(0.1);
    tacc->Draw("samep");
    tbkgnd->Draw("samep");
    
    TCanvas *c3 = new TCanvas("c3", "c3",200,10,1000,700);
    
    c3->Divide(3,2);
    c3->cd(1);
    // gPad->SetLogy();
    ymin = 100;
    ymax = 2000/scale_factor;
    
    M2piacc->SetTitle(filename);
    // M2piacc->GetXaxis()->SetRangeUser(xmin,xmax);
    // M2piacc->GetYaxis()->SetRangeUser(ymin,ymax);
    M2piacc->GetXaxis()->SetTitleSize(0.05);
    M2piacc->GetYaxis()->SetTitleSize(0.05);
    M2piacc->GetXaxis()->SetTitle("M_{#pi#pi}");
    M2piacc->SetMarkerColor(2);
    // M2piacc->Draw("samep");
    M2pidatsub->SetMarkerColor(1);
    M2pidatsub->SetLineColor(1);
    M2pidatsub->SetMarkerStyle(20);
    M2pidatsub->SetMarkerSize(0.1);
    M2pidatsub->GetXaxis()->SetTitle("M_{#pi#pi}");
    M2piacc->Draw("p");
    M2pidatsub->Draw("samep");
    
    c3->cd(2);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 8000/scale_factor;
    
    cosThetaacc->SetTitle(filename);
    // cosThetaacc->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) cosThetaacc->GetYaxis()->SetRangeUser(ymin,ymax);
    cosThetaacc->GetXaxis()->SetTitleSize(0.05);
    cosThetaacc->GetYaxis()->SetTitleSize(0.05);
    cosThetaacc->GetXaxis()->SetTitle("cos(#theta)");
    cosThetaacc->SetLineColor(2);
    cosThetaacc->Draw("p");
    // cosThetaacc->Draw("samep");
    cosThetadatsub->SetMarkerColor(1);
    cosThetadatsub->SetLineColor(1);
    cosThetadatsub->SetMarkerStyle(20);
    cosThetadatsub->SetMarkerSize(0.1);
    cosThetadatsub->Draw("samep");
    
    c3->cd(3);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 8000/scale_factor;
    
    // TF1 *cos2phi = new TF1("cos2phi","[0]*(1+[1]*cos(2*x))",-3.14159,3.14159);
    
    Phiacc->SetTitle(filename);
    // Phiacc->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) Phiacc->GetYaxis()->SetRangeUser(ymin,ymax);
    Phiacc->GetXaxis()->SetTitleSize(0.05);
    Phiacc->GetYaxis()->SetTitleSize(0.05);
    Phiacc->GetXaxis()->SetTitle("#phi_{#pi#pi}");
    Phiacc->SetMarkerColor(2);
    Phidatsub->SetMarkerColor(1);
    Phidatsub->SetLineColor(1);
    Phidatsub->SetMarkerStyle(20);
    Phidatsub->SetMarkerSize(0.1);
    Phidatsub->Fit(cos2phi);
    Phiacc->Draw("p");
    Phidatsub->Draw("samep");
    
    
    c3->cd(5);


    ymin = 0;
    ymax = 8000/scale_factor;
    
    psiacc->SetTitle(filename);
    // psiacc->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) psiacc->GetYaxis()->SetRangeUser(ymin,ymax);
    psiacc->GetXaxis()->SetTitleSize(0.05);
    psiacc->GetYaxis()->SetTitleSize(0.05);
    psiacc->GetXaxis()->SetTitle("#psi");
    psiacc->SetMarkerColor(2);
    // psiacc->Draw("samep");
    psidatsub->SetMarkerColor(1);
    psidatsub->SetLineColor(1);
    psidatsub->SetMarkerStyle(20);
    psidatsub->SetMarkerSize(0.1);
    psidatsub->Fit(cos2phi);
    psiacc->Draw("p");
    psidatsub->Draw("samep");

    
    c3->cd(4);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 8000/scale_factor;
    
    phiacc->SetTitle(filename);
    // phiacc->GetXaxis()->SetRangeUser(xmin,xmax);
    if (setscale) phiacc->GetYaxis()->SetRangeUser(ymin,ymax);
    phiacc->GetXaxis()->SetTitleSize(0.05);
    phiacc->GetYaxis()->SetTitleSize(0.05);
    phiacc->GetXaxis()->SetTitle("#phi");
    phiacc->SetMarkerColor(2);
    phiacc->SetLineColor(2);
    // phiacc->Draw("samep");
    phidatsub->SetMarkerColor(1);
    phidatsub->SetLineColor(1);
    phidatsub->SetMarkerStyle(20);
    phidatsub->SetMarkerSize(0.1);
    phidatsub->Fit(cos2phi);
    // phidatsub->Fit(cosphi);
    phiacc->Draw("p");
    phidatsub->Draw("samep");

    c3->cd(6);
    
    
    // now read and print fitted values


    cout << "*** Starting to read parameters from " << infile.Data() << endl << endl;
    
    ifstream parameters;
    parameters.open (infile.Data());
    if (!parameters) {
        cout << "ERROR: Failed to open data file= " << infile.Data() << endl;
        return;
    }
    
    TString line;
    while (line.ReadLine(parameters)){
        
        TObjArray *tokens = line.Tokenize("\t");
        Int_t ntokens = tokens->GetEntries();
        
        cout << " ntokens=" << ntokens << " line=" << line.Data() << endl;
	Int_t jmax=ntokens/3;
        for (Int_t j=0; j<jmax; j++){
	  sdme.push_back( (((TObjString*)tokens->At(3*j))->GetString()) );
          parms.push_back( (((TObjString*)tokens->At(3*j+1))->GetString()).Atof() );
          parms_err.push_back( (((TObjString*)tokens->At(3*j+2))->GetString()).Atof());
        }
        
    }   // end loop over lines
    
        sprintf (string,"AmpTool Fit\n");
        printf("string=%s",string);
        TLatex *t1 = new TLatex(0.2,0.85,string);
        t1->SetNDC();
        t1->SetTextSize(0.04);
        t1->Draw();
        
        for (Int_t j=0; j<sdme.size()-2; j++) {     // -2 to eliminate Sigma and P
            cout << sdme[j] << "=" << parms[j] << " err=" << parms_err[j] << endl;
        
            TString sdmename;
            sdmename = sdme[j];
            sprintf (string,"%s = \t%.3f #pm %.3f\n",sdmename.Data(),parms[j],parms_err[j]);
        	printf("string=%s",string);
        	TLatex *t1 = new TLatex(0.2,0.75 - 0.05*j,string);
        	t1->SetNDC();
        	t1->SetTextSize(0.04);
        	t1->Draw();
        }
        
    
    parameters.close();

    // create summary canvas for pi0 LOI
    TCanvas *c4 = new TCanvas("c4", "c4",200,10,1000,1000);
    c4->Divide(2,2);

    c4->cd(1);
    M2pigen->Draw("p");
    M2pidat->Draw("samep");

    c4->cd(2);
    M2piAcceptance->Draw("p");

    c4->cd(3);
    Phigen->Draw("p");
    Phidat->Draw("samep");

    c4->cd(4);
    gPad->SetLogy();
    tdat->Draw("p");
    tgen->Draw("samep");

    // show components for various quantities
    TCanvas *c5 = new TCanvas("c5", "c5",200,10,1000,1000);
    c5->Divide(2,2);

    ymin=0;
    ymax=1000*3/scale_factor;

    c5->cd(1);
    // M2piacc->DrawCopy();
    // M2piacc->SetLineColor(1);
    M2pidatsub->SetTitle("");
    M2pidatsub->GetXaxis()->SetRangeUser(xmin,xmax);
    M2pidatsub->GetYaxis()->SetRangeUser(ymin,ymax);
    M2pidatsub->GetXaxis()->SetTitleSize(0.05);
    M2pidatsub->GetYaxis()->SetTitleSize(0.05);
    M2pidatsub->SetLineColor(1);
    M2pidatsub->DrawCopy();
    M2pidatsub->SetMarkerColor(1);
    M2piacc_g1V00->SetLineColor(2);
    M2piacc_g1V00->Draw("same");
    M2piacc_g1V00s->SetLineColor(4);
    M2piacc_g1V00s->Draw("same");
    M2piacc_g1V11->SetLineColor(3);
    M2piacc_g1V11->Draw("same");
    M2piacc_g1V10->SetLineColor(7);
    M2piacc_g1V10->Draw("same");
    M2piacc_g1V1M1->SetLineColor(6);
    M2piacc_g1V1M1->Draw("same");

    TH1F *M2piInt;
    M2piInt = (TH1F*) M2pidatsub->Clone("M2piInt");
    M2piInt->Add(M2piacc_g1V00,-1);
    M2piInt->Add(M2piacc_g1V00s,-1);
    M2piInt->SetLineColor(3);
    // M2piInt->Draw("same");

    TLegend *leg2 = new TLegend(0.2,0.7,0.55,0.85);
    // leg2->AddEntry(M2piacc,"MC Acc","lp");
    leg2->AddEntry(M2pidatsub,"Data","lp");
    leg2->AddEntry(M2piacc_g1V00,"Primakoff","lp");
    leg2->AddEntry(M2piacc_g1V00s,"Nuclear Coherent","lp");
    leg2->AddEntry(M2piacc_g1V1M1,"Pwave 1-1","lp");
    leg2->AddEntry(M2piacc_g1V10,"Pwave 10","lp");
      leg2->AddEntry(M2piacc_g1V11,"Pwave 11","lp");
    // leg2->AddEntry(M2piInt,"Inteference","lp");
    leg2->Draw();

    
    ymin=0;
    ymax=2500*4/scale_factor;

    c5->cd(2);
    xmin = 0;
    xmax = 2;
    theta_scatdat->SetTitle("");;
    theta_scatdat->GetXaxis()->SetRangeUser(xmin,xmax);
    theta_scatdat->GetYaxis()->SetRangeUser(ymin,ymax);
    theta_scatdat->GetXaxis()->SetTitleSize(0.05);
    theta_scatdat->GetYaxis()->SetTitleSize(0.05);
    theta_scatdat->GetXaxis()->SetTitle("#theta_{#pi#pi}");
    theta_scatdat->SetMarkerColor(1);
    theta_scatdat->SetLineColor(1);
    theta_scatdat->Draw();
    theta_scatdat->SetLineColor(1);
    theta_scatacc_g1V00->SetLineColor(2);
    theta_scatacc_g1V00->Draw("same");
    theta_scatacc_g1V00s->SetLineColor(4);
    theta_scatacc_g1V00s->Draw("same");
    theta_scatacc_g1V11->SetLineColor(3);
    theta_scatacc_g1V11->Draw("same");
    theta_scatacc_g1V10->SetLineColor(7);
    theta_scatacc_g1V10->Draw("same");
    theta_scatacc_g1V1M1->SetLineColor(6);
    theta_scatacc_g1V1M1->Draw("same");

    TH1F *theta_scatInt;
    theta_scatInt = (TH1F*) theta_scatdat->Clone("theta_scatInt");
    theta_scatInt->Add(theta_scatacc_g1V00,-1);
    theta_scatInt->Add(theta_scatacc_g1V00s,-1);
    theta_scatInt->SetLineColor(3);
    // theta_scatInt->Draw("same");

    leg2->Draw();

    Double_t sum_data = theta_scatdat->Integral();
    Double_t sum_g1V00 = theta_scatacc_g1V00->Integral();
    Double_t sum_g1V00s = theta_scatacc_g1V00s->Integral();
    Double_t sum_g1V11 = theta_scatacc_g1V11->Integral();
    Double_t sum_g1V10 = theta_scatacc_g1V10->Integral();
    Double_t sum_g1V1M1 = theta_scatacc_g1V1M1->Integral();
    Double_t sum_Int = theta_scatInt->Integral();

    cout << endl;
    cout << " Integrals: Data=" << sum_data << " g1V00=" << sum_g1V00 << " g1V00s=" << sum_g1V00s << " sum_g1V11=" << sum_g1V11  << " sum_g1V10=" << sum_g1V10  << " sum_g1V1M1=" << sum_g1V1M1 << endl;
    cout << " Fractions: Data=" << sum_data/sum_g1V00 << " g1V00=" << sum_g1V00/sum_g1V00 << " g1V00s=" << sum_g1V00s/sum_g1V00 << " sum_g1V11=" << sum_g1V11/sum_g1V00 << " sum_g1V10=" << sum_g1V10/sum_g1V00 << " sum_g1V1M1=" << sum_g1V1M1/sum_g1V00 << endl;
    cout << endl;

    ymin=0;
    ymax=10000/scale_factor;

    c5->cd(3);
    Phidat->SetLineColor(1);
    Phidat->GetYaxis()->SetRangeUser(ymin,ymax);
    Phidat->GetXaxis()->SetTitleSize(0.05);
    Phidat->GetYaxis()->SetTitleSize(0.05);
    Phidat->SetTitle("");
    Phidat->GetXaxis()->SetTitle("#phi_{#pi#pi}");
    Phidat->DrawCopy();
    Phiacc_g1V00->SetLineColor(2);
    Phiacc_g1V00->Draw("same");
    Phiacc_g1V00s->SetLineColor(4);
    Phiacc_g1V00s->Draw("same");
    Phiacc_g1V11->SetLineColor(3);
    Phiacc_g1V11->Draw("same");
    Phiacc_g1V10->SetLineColor(7);
    Phiacc_g1V10->Draw("same");
    Phiacc_g1V1M1->SetLineColor(6);
    Phiacc_g1V1M1->Draw("same");

    TH1F *PhiaccInt;
    PhiaccInt = (TH1F*) Phidat->Clone("PhiaccInt");
    PhiaccInt->Add(Phiacc_g1V00,-1);
    PhiaccInt->Add(Phiacc_g1V00s,-1);
    PhiaccInt->SetLineColor(3);
    // PhiaccInt->Draw("same");

    leg2->Draw();

    ymin=0;
    ymax=4000*2/scale_factor;
    c5->cd(4);
    cosThetadat->SetLineColor(1);
    cosThetadat->GetYaxis()->SetRangeUser(ymin,ymax);
    cosThetadat->GetXaxis()->SetTitleSize(0.05);
    cosThetadat->GetYaxis()->SetTitleSize(0.05);
    cosThetadat->SetTitle("");
    cosThetadat->GetXaxis()->SetTitle("cos(#theta_{#pi})");
    cosThetadat->DrawCopy();
    cosThetaacc_g1V00->SetLineColor(2);
    cosThetaacc_g1V00->Draw("same");
    cosThetaacc_g1V00s->SetLineColor(4);
    cosThetaacc_g1V00s->Draw("same");
    cosThetaacc_g1V11->SetLineColor(3);
    cosThetaacc_g1V11->Draw("same");
    cosThetaacc_g1V10->SetLineColor(7);
    cosThetaacc_g1V10->Draw("same");
    cosThetaacc_g1V1M1->SetLineColor(6);
    cosThetaacc_g1V1M1->Draw("same");

    TH1F *cosThetaInt;
    cosThetaInt = (TH1F*) cosThetadat->Clone("cosThetaInt");
    cosThetaInt->Add(cosThetaacc_g1V00,-1);
    cosThetaInt->Add(cosThetaacc_g1V00s,-1);
    cosThetaInt->SetLineColor(3);
    // cosThetaInt->Draw("same");


    TCanvas *c6 = new TCanvas("c6", "c6",200,10,1000,1000);
    M2piacc_g1V00->Draw();
    M2piacc_g1V00->GetXaxis()->SetTitle("M_{#pi#pi}");


    leg2->Draw();

    c4->SaveAs(filename+"_sum.pdf");
    c5->SaveAs(filename+"_decomposition.pdf");
    c6->SaveAs(filename+"_M2piacc_g1V00.C");



    c0->SaveAs(filename+".pdf(");
    c1->SaveAs(filename+".pdf");
    c2->SaveAs(filename+".pdf");
    c3->SaveAs(filename+".pdf");
    c4->SaveAs(filename+".pdf");
    c5->SaveAs(filename+".pdf)");
}
