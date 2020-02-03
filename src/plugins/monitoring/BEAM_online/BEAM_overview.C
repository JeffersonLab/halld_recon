
// deltaTall microscope counter time differneces RANDOM trigers
// PStagmEnergyInTme
// PStagmEnergyOutOfTme
// PStaghEnergyInTme
// PStaghEnergyOutOfTme


{

  TDirectory *BEAMdir = (TDirectory*)gDirectory->FindObjectAny("BEAM");

  if(!BEAMdir) return;
        
  BEAMdir->cd();


  TH2D *deltaTall = (TH2D*)gDirectory->FindObjectAny("deltaTall");
  TH1D *dtp = deltaTall->ProjectionX("dtp",1,10);
  dtp->SetTitle("Microscope counters Random trigger timing");
  dtp->GetXaxis()->SetTitle("#Deltat [ns]");
  
  TH1D *PStagmEnergyInTme = (TH1D*)gDirectory->FindObjectAny("PStagmEnergyInTme");
  TH1D *PStagmEnergyOutOfTme = (TH1D*)gDirectory->FindObjectAny("PStagmEnergyOutOfTme");
  
  TH1D *PStaghEnergyInTme = (TH1D*)gDirectory->FindObjectAny("PStagmEnergyInTme");
  TH1D *PStaghEnergyOutOfTme = (TH1D*)gDirectory->FindObjectAny("PStagmEnergyOutOfTme");


  TCanvas *cBEAM = new TCanvas("cBEAM","Beam Conditions TAGGER-PS Matches", 900, 600 );

  cBEAM->Divide(1,3);

  cBEAM->cd(1);
  dtp->Draw();

  int binl = dtp->FindBin(0.0-2.004);
  int binh = dtp->FindBin(0.0+2.004);
  double Ic = dtp->Integral(binl, binh);
  int binll = dtp->FindBin(0.0-2.004 - 10.*4.008);
  int binhh = dtp->FindBin(0.0+2.004 + 10.*4.008);
  double Il = dtp->Integral(binll, binl-1);
  double Ih = dtp->Integral(binh+1, binhh);
  double BG = (Il+Ih)/2./10.;
  double Rrand = Ic/BG;
  char text0[128];
  sprintf(text0, "Scaling: %5.2f",Scale);
  TText *t0 = new TText(0.15, 0.7, text0);
  t0->SetTextSize(0.1);
  t0->SetNDC();
  t0->Draw();


  cBEAM->cd(2);
  PStagmEnergyOutOfTime->Scale(1./20.);
  PStagmEnergyInTime->Draw();

  double ISignalL = PStagmEnergyInTime->Integral(700, 750);
  double IBackgrL = PStagmEnergyOutOfTime->Integral(700, 750);
  double ScaleL = ISignalL/IBackgrL;
  double ScaleLErr = TMath::Sqrt(ISignalL/IBackgrL/IBackgrL*(1.+ScaleL));
  double ISignalR = PStagmEnergyInTime->Integral(1180, 1400);
  double IBackgrR = PStagmEnergyOutOfTime->Integral(1180, 1400);
  double ScaleR = ISignalR/IBackgrR;
  double ScaleRErr = TMath::Sqrt(ISignalR/IBackgrR/IBackgrR*(1.+ScaleR));
  double ScaleErr = (1./ScaleLErr/ScaleLErr + 1./ScaleRErr/ScaleRErr);
  double Scale = (ScaleL/ScaleLErr/ScaleLErr + ScaleR/ScaleRErr/ScaleRErr)/ScaleErr;
  ScaleErr = TMath::Sqrt(1./ScaleErr);
  PStagmEnergyOutOfTime->Scale(Scale);
  PStagmEnergyOutOfTime->SetLineColor(2);
  PStagmEnergyOutOfTime->Draw("same");
  gPad->SetLogy(1);
  gPad->SetGrid();
  char text1[128];
  sprintf(text1, "Scaling: %5.2f",Scale);
  TText *t1 = new TText(0.2, 0.7, text1);
  t1->SetTextSize(0.1);
  t1->SetNDC();
  t1->Draw();


  cBEAM->cd(3);
  PStaghEnergyOutOfTime->Scale(1./20.);
  PStaghEnergyInTime->Draw();
  PStaghEnergyOutOfTime->SetLineColor(7);
  PStaghEnergyOutOfTime->Draw("same");
  gPad->SetLogy(1);
  gPad->SetGrid();
  gPad->Update();
  //getchar();
  ISignalL = PStaghEnergyInTime->Integral(700, 750);
  IBackgrL = PStaghEnergyOutOfTime->Integral(700, 750);
  ScaleL = ISignalL/IBackgrL;
  ScaleLErr = TMath::Sqrt(ISignalL/IBackgrL/IBackgrL*(1.+ScaleL));
  ISignalR = PStaghEnergyInTime->Integral(1180, 1400);
  IBackgrR = PStaghEnergyOutOfTime->Integral(1180, 1400);
  ScaleR = ISignalR/IBackgrR;
  ScaleRErr = TMath::Sqrt(ISignalR/IBackgrR/IBackgrR*(1.+ScaleR));
  ScaleErr = (1./ScaleLErr/ScaleLErr + 1./ScaleRErr/ScaleRErr);
  Scale = (ScaleL/ScaleLErr/ScaleLErr + ScaleR/ScaleRErr/ScaleRErr)/ScaleErr;
  ScaleErr = TMath::Sqrt(1./ScaleErr);


  TH1D* h = (TH1D*)PStaghEnergyOutOfTime->Clone();
  h->Scale(Scale);
  h->SetLineColor(2);
  h->Draw("same");

  sprintf(text1, "Scaling: %5.2f",Scale);
  TText *t2 = new TText(0.2, 0.7, text1);
  t2->SetTextSize(0.1);
  t2->SetNDC();
  t2->Draw();

}
