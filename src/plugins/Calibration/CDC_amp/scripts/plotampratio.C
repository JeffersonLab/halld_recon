
// fits and generates plots from histograms in ampx_fits files created by get_gain_doca_params.C

void plotampratio(int run=0, float dmax=0, float oldp0=0, float oldp1=0){

  gStyle->SetFuncWidth(1);

  if (run>0) TFile *f = new TFile(Form("rootfiles/ampx_fits_%i.root",run),"read");

  //adoca30 = new TH2D("adoca30","CDC amplitude vs DOCA (tracks, theta 28-32 deg, z 52-78cm); DOCA (cm); amplitude - pedestal",200,0,1.0,4096,0,4096);

  // bins 0-0.005, 0.005-0.01 etc
  // 0-1mm would be 20 bins


  TH1D* a1mm = new TH1D("a1mm","CDC amplitude,  DOCA 0-1mm (tracks, theta 28-32 deg, z 52-78cm); amplitude - pedestal",4096,0,4096);

  TH1D* h;

  for (int i=1; i<=20; i++) {

    h = (TH1D*)gDirectory->Get(Form("amp_bin_%i",i));
    a1mm->Add(h);
  }

  bool scale4096 = 0;  // range is either 0-511 or 0-4095
  if (a1mm->GetBinContent(512)>0) scale4096 = 1;

  TF1 *lan = new TF1("lan","landau");

  double fitpars[3]={0};

  double fitstart=16;
  double fitstop=300;
  if (scale4096) fitstart *=8;
  if (scale4096) fitstop *=8;

  int fitstatus = a1mm->Fit(lan,"0QWR","",fitstart, fitstop);

  Double_t mpv1mm = 0;
  Double_t sig1mm = 0;
  Double_t peak1mm = 0;

  if (fitstatus == 0) {
    mpv1mm = lan->GetParameter(1);
    sig1mm = lan->GetParameter(2);
    peak1mm = mpv1mm-0.25*sig1mm;
  } else {
    cout << "0-1mm slice fitstatus " << fitstatus << endl;
    return;
  }

  lan->GetParameters(fitpars);
  
  Double_t a[150],mpv[150],sig[150], relmpv[150] ,doca[150], mpvsig[150];

  Double_t bada[150], badmpv[150], badsig[150], badrelmpv[150], baddoca[150], badmpvsig[150];

  Double_t latempvsig[150],latedoca[150],laterelmpv[150];  // mpv/sig after dmax 


  TF1* oldfn;

  bool goodfit=1;   // switch to bad as soon as first fit peak goes below threshold

  int ngood=0, nbad=0, nlate=0;

  double lastgooddoca = 0; // last doca with fitted peak above threshold

  for (int i=1; i<=150; i++) {

    h = (TH1D*)gDirectory->Get(Form("amp_bin_%i",i));
    oldfn = (TF1*)h->GetFunction("lan");

    if (oldfn->GetParameter(1) - 0.223*oldfn->GetParameter(2) <= fitstart+4) goodfit=0;

    if (goodfit) {

        a[ngood] = oldfn->GetParameter(0);
        mpv[ngood] = oldfn->GetParameter(1);
        sig[ngood] = oldfn->GetParameter(2);

        relmpv[ngood] = mpv[ngood]/mpv1mm;
        mpvsig[ngood] = mpv[ngood]/sig[ngood];
        doca[ngood] = i*0.005 - 0.0025;

        if (doca[ngood]<=dmax) lastgooddoca = doca[ngood];    

        if (doca[ngood]>dmax && nlate<20) {
          latempvsig[nlate] = mpvsig[ngood];
          latedoca[nlate] = doca[ngood];
          laterelmpv[nlate] = relmpv[ngood];
          lastgooddoca = doca[ngood];    
          nlate++;
        }

        if (scale4096) {
          a[ngood] *= 8.0;
          mpv[ngood] /= 8.0;
          sig[ngood] /= 8.0;
        }

        ngood++;    

    } else {    

        bada[nbad] = oldfn->GetParameter(0);
        badmpv[nbad] = oldfn->GetParameter(1);
        badsig[nbad] = oldfn->GetParameter(2);

        badrelmpv[nbad] = badmpv[nbad]/mpv1mm;
        badmpvsig[nbad] = badmpv[nbad]/badsig[nbad];
        baddoca[nbad] = i*0.005 - 0.0025;
    
        if (scale4096) {
          bada[nbad] *= 8.0;
          badmpv[nbad] /= 8.0;
          badsig[nbad] /= 8.0;
        }

        nbad++;  
    }    
  }
  
  printf("%i good %i bad %i late\n",ngood,nbad,nlate);


  //------    mpv/sig graph   ------------

  TGraph *g2 = new TGraph(ngood,doca,mpvsig);
  g2->SetMarkerSize(0.6);
  g2->SetMarkerStyle(20);

  TGraph *g2b = new TGraph(nbad,baddoca,badmpvsig);
  g2b->SetMarkerSize(0.6);
  g2b->SetMarkerStyle(20);
  g2b->SetMarkerColor(16);

  TGraph *g2c = new TGraph(nlate,latedoca,latempvsig);
  g2c->SetMarkerSize(0.6);
  g2c->SetMarkerStyle(20);
  g2c->SetMarkerColor(880);

  TMultiGraph *mg2 = new TMultiGraph();
  mg2->Add(g2);
  mg2->Add(g2b);
  mg2->SetTitle(Form("Run %i pulse height landau MPV/sigma vs DOCA;DOCA (cm);Pulse height MPV / sigma",run));

  mg2->Draw("AP");
  mg2->GetXaxis()->SetRangeUser(0,0.82);

  g2c->Draw("Psame");

  gPad->Update();

  gPad->SaveAs(Form("plots_mpvsig/run%i_mpvsig.png",run));


  //------   rel amp graph   ------------

  new TCanvas;

  TGraph *g = new TGraph(ngood,doca,relmpv);
  g->SetMarkerSize(0.6);
  g->SetMarkerStyle(20);

  TGraph *badg = new TGraph(nbad,baddoca,badrelmpv);
  badg->SetMarkerSize(0.4);
  badg->SetMarkerStyle(20);
  badg->SetMarkerColor(16);  

  TGraph *lateg = new TGraph(nlate,latedoca,laterelmpv);
  lateg->SetMarkerSize(0.6);
  lateg->SetMarkerStyle(20);
  lateg->SetMarkerColor(880);

  TMultiGraph *mg = new TMultiGraph();
  mg->Add(g);
  mg->Add(badg);
  mg->Add(lateg);
  mg->SetTitle(Form("Run %i relative pulse height vs DOCA;DOCA (cm);Pulse height MPV / MPV(0-1mm)",run));
  mg->Draw("AP");

  mg->GetYaxis()->SetRangeUser(0,1.1);
  mg->GetXaxis()->SetRangeUser(0,0.82);
  gPad->Update();
  //   mg->Draw("AP");

  if (dmax==0) return;

   //------------------

  // 1st line segment, 0 to 1mm
  TF1 *lin0 = new TF1("lin0","1.0",0, 0.1);
  lin0->SetNpx(200);  
  lin0->Draw("same");

  // 2nd  line segment, 1mm to dmax
  TF1 *lin1 = new TF1("lin1","[0]+[1]*x",0.1,dmax);
  g->Fit(lin1,"Q","R",0.1,dmax);

  // the extension of 2nd line segment, dashed
  TF1 *ext1 = new TF1("ext1","[0]+[1]*x",0.1,1);
  ext1->SetParameters(lin1->GetParameters());
  ext1->SetLineColor(2);
  ext1->SetLineStyle(2);
  ext1->Draw("same");
  lin1->SetNpx(200);
  lin1->Draw("same");



  Double_t xzero = 0;

  if (nlate>9){
  // fit new line segment
    TF1 *lin2 = new TF1("lin2","[0]+[1]*x",dmax,lastgooddoca);  // was dmax+0.1
    lin2->SetLineColor(880);
    lin2->SetNpx(200);
    g->Fit(lin2,"Q+","R",dmax,lastgooddoca);

    //extension of new line segment 
    TF1 *ext2 = new TF1("ext2","[0]+[1]*x",dmax,1);
    ext2->SetParameters(lin2->GetParameters());
    ext2->SetLineColor(880);
    ext2->SetLineStyle(2);
    ext2->Draw("same");
    lin2->Draw("same");


  // ext 2 crosses 0 when doca = -[0]/[1]

  xzero = -1 * lin2->GetParameter(0)/lin2->GetParameter(1);

  cout << "reach 0 at " << xzero << " dmax + " << xzero - dmax << endl;

  //  printf("new/old fit params %.2f  %.2f\n",lin1->GetParameter(0)/oldp0, lin1->GetParameter(1)/oldp1);


  FILE *outf = fopen("ampzero.txt","a");
  fprintf(outf, "%i %.3f %.3f %.3f\n",run,dmax,xzero,xzero-dmax);
  fclose(outf);

  outf = fopen("ampzero2.txt","a");
  fprintf(outf, "%i %.3f %.3f %.3f %.3f %.3f %i\n",run,dmax,xzero,xzero-dmax,lastgooddoca-dmax,lastgooddoca,nlate);
  fclose(outf);

  outf = fopen("fitparams.txt","a");
  fprintf(outf, "%i %.3f %.3f\n",run,lin1->GetParameter(0),lin1->GetParameter(1));
  fclose(outf);

  outf = fopen("fitparamratio.txt","a");
  fprintf(outf, "%i %.3f %.3f\n",run,lin1->GetParameter(0)/oldp0,lin1->GetParameter(1)/oldp1);
  fclose(outf);

  } else {

    cout << nlate << "points after dmax - not enough to fit\n";
  }

  TPaveText *pt = new TPaveText(0.55,0.6,0.89,0.85,"NDC");

  pt->AddText(Form("max DOCA for dE/dx %.2f",dmax));
  if (xzero>0) pt->AddText(Form("zero crossing at %.2f",xzero));
  pt->SetFillStyle(0);
  pt->SetBorderSize(0);
  pt->Draw();


    gPad->SaveAs(Form("plots_relamp/run%i_relamp.png",run));


}
