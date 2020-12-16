// get tprofile of amp vs t histogram from CDC_amp plugin

void fithistoslices(int run, TH2D *at, int &fullrange, double x[], double mpv[], double sig[]){

  //  at->Draw("colz");

  int nbinsx = at->GetXaxis()->GetNbins();

  // find out which amp scale was used

  fullrange = 0;  // amplitude readout

  TH1D *py = at->ProjectionY("py",0,nbinsx-1);
 
  double highcounts = py->FindLastBinAbove(0);
  if (highcounts > 512) fullrange = 1;

  int fitlim_low;
  int fitlim_high;

  if (fullrange) { 

    fitlim_low = 130;   // lower limit attsum  - increased from 80
    fitlim_high = 2400;   //amp fit upper limit landau

  } else {

    fitlim_low = 16;   // lower limit attsum 
    fitlim_high = 300;   //amp fit upper limit landau

  } 

  printf("Saving fits and tree to ampx_fits_%i.root\n",run);

  TFile *fitfile = new TFile(Form("rootfiles/ampx_fits_%i.root",run),"recreate");

  TF1 *lan = new TF1("lan","landau");

  double fitpars[3]={0};

  int bin3mm = at->GetXaxis()->FindBin(0.31);  //3.1mm  
  int binxmax = at->GetXaxis()->FindBin(0.8); 

  printf("amp vs time histo has 3.1mm in bin %i\n", bin3mm);

  py = at->ProjectionY("py",0,bin3mm);
  py->SetTitle(Form("Projection for x=0 to 3mm, %s",py->GetTitle()));


  int fitstatus;

  fitstatus = py->Fit(lan,"QWR","",fitlim_low,fitlim_high); //20 to 400 for 0-511 readout


  if (fitstatus == 0) {
    mpv[0] = lan->GetParameter(1);
    sig[0] = lan->GetParameter(2);
  } else {
    cout << "0-3mm slice fitstatus " << fitstatus << endl;
    return;
  }

  lan->GetParameters(fitpars);

  fitfile->cd();
  py->SetName("amp_0_3.1mm");
  py->Write();


  int j = 1;


  double dx = at->GetXaxis()->GetBinWidth(1);
  double this_x=at->GetXaxis()->GetBinCenter(1); //0.5*dx;  // bin center

  //  cout << "dx " << dx << endl;

  int status4count=0;

  for (int i=1; i<=binxmax; i++) {

    py = at->ProjectionY("py",i,i);

    py->SetTitle(Form("Projection for bin %i x %.3f cm, %s",i,this_x,py->GetTitle()));

    lan->SetParameters(fitpars);

    // fit sometimes fails with very small step sizes and derivatives 0, visually looks ok
    // calling it again with improve option M uses a larger step and result is ok.

    fitstatus = py->Fit(lan,"QWR","",fitlim_low,fitlim_high); //20 to 400 for 0-511 readout
    if (fitstatus==4) fitstatus = py->Fit(lan,"QMWR","",fitlim_low,fitlim_high); //20 to 400 for 0-511 readout
    if (fitstatus==4) fitstatus = py->Fit(lan,"QMWR","",fitlim_low,fitlim_high); //20 to 400 for 0-511 readout

    py->SetName(Form("amp_bin_%i",i));
    py->Write();



    if ((fitstatus == 0) || (fitstatus == 4000)) { 

      mpv[j] = lan->GetParameter(1);
      sig[j] = lan->GetParameter(2);
      x[j] = this_x;

      lan->GetParameters(fitpars);

      j++;    

    } else {
      cout << "fitstatus " << fitstatus << " for bin " << i << " doca " << this_x << endl;
      if (fitstatus==4) status4count++;
    }  

    this_x += dx;

    if(status4count > 3) break;

  }

  double tdoca,tmpv,tsig;

  int i,orig_bin;

  
  TTree *tree = new TTree("T",Form("Amp fits for doca slices in run %i",run));
  tree->Branch("bin",&orig_bin,"bin/I");
  tree->Branch("doca",&tdoca,"doca/D");
  tree->Branch("mpv",&tmpv,"mpv/D");
  tree->Branch("sig",&tsig,"sig/D");


  for (i=0; i<nbinsx; i++) {
    if (mpv[i] == 0) break;
    orig_bin = i;
    tdoca = x[i];
    tmpv = mpv[i];
    tsig = sig[i];  

    tree->Fill();  
  }

  fitfile->cd();
  tree->Write();
  fitfile->Close();
  //  cout << "last element filled is " << j << endl;


}




void get_gain_doca_params(const int run=999) {


  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.4);
  gStyle->SetOptFit(1);

  const int save=1; // create files for ccdb
  const int saveplots=1; // save plot files to subdir plots
  const int saveextras=1; // list of stop point and mpv ratio

  //  char filename1[150];
  //  sprintf(filename1, "/raid12/gluex/rawdata2/Run%06i/ampxt_%06i_001.root",run,run);
  //  sprintf(filename1, "Run%06i/hd_calib_verify_Run%06i_001.root",run,run);
  //  sprintf(filename1, "/raid12/gluex/calibhists/ver14_2018/Run%06i/hd_calib_verify_Run%06i_001.root",run,run);

  //  TFile *file1 = new TFile(filename1,"r");

  // NB 40701 was at full range readout for amplitude. 
  //  41508 on are all at 0-511 scale.
  // 40945 was first production run in 2018 with similar pressure (100.7 kPa) to 40701 (100.6kPa)

  gDirectory->pwd();
  gDirectory->ls();
 
  TDirectory *fmain = (TDirectory*)gDirectory->FindObjectAny("CDC_amp");
  if (!fmain) printf("Cannot find directory CDC_amp\n"); 
  if (!fmain) return;
  fmain->cd();

  //  cout << "Opened " << filename1 << endl;

  //  TH2D *at = (TH2D*)fmain->Get("attdoca");
  TH2D *at = (TH2D*)fmain->Get("adoca30");
  if (!at) printf("Cannot find histogram attdoca\n");
  if (!at) return;

  const int nbins=200;

  double mpv[nbins] = {0};
  double sig[nbins] = {0};
  double x[nbins] = {0};
  int fullrange = 0;

  //void fithistoslices(int run, TH2D *at, int &fullrange, double x[], double mpv[], double sig[])
  fithistoslices(run,at,fullrange,x,mpv,sig);
  //  file1->Close();

  // element 0 contains the data for 0 to 3.1mm


  // need to calc the ratio of all the points to this.

  // and then work out what to scale this by to match it to the reference run (40701?) 

  //  const double idealmpv = 288.98;    // 90 degrees
  const double idealmpv = 658.499;    // 30 degrees

  // const int referenceruns[] = {40701, 40881, 40946, 41510, 41860, 42485, 42577};
  // float fitstops[] = {0.65, 0.45, 0.54, 0.45, 0.4, 0.4, 0.35};


  // const int referenceruns[] = {40701, 40881, 40946, 41005, 41089, 41041, 41221, 41510, 41860, 42485, 42577};
  // float fitstops[] = {0.65, 0.45, 0.54, 0.38, 0.28, 0.4, 0.34,  0.45, 0.4, 0.4, 0.35};

  // int nrefs = sizeof(referenceruns)/sizeof(int);

  // int usethis = nrefs-1;
  // while ((run < referenceruns[usethis]) && (usethis > 0)) usethis--;
  // fitstop = fitstops[usethis];

  // printf("using fit limit %.2f from reference run %i\n",fitstop,referenceruns[usethis]);

  // find how many nonzero mpvs there are

  double thismpv=0;
  int n = nbins-1;

  while (mpv[n]==0) n--;
  cout << n << "fits found "<< endl;

  double relmpv[nbins] = {0};
  double mpvsig[nbins] = {0};

  double mpv100 = mpv[0]; // this is the mpv of the first 3.1mm chunk
  
  double ampscale = 1.0;
  if (!fullrange) ampscale = 8.0;

  double mpvratio = mpv100*ampscale/idealmpv;
  printf("\nratio of MPV for 0-3mm for this run to run 40701 is %.3f\n\n",mpvratio);

  // Calculate mpv / mpv100 = relative mpv
  // Calculate mpv:sigma ratio  

  for (int i=0; i<n; i++) {
    relmpv[i]= mpv[i]/mpv100;
    if (sig[i]>0) mpvsig[i] = mpv[i]/sig[i];
  }

  TF1 *linfit = new TF1("linfit","pol1");
  gStyle->SetOptFit(1);
  double linfitpars[2];

  // Inspect mpv/sigma graph to find where shape of fit changes; later fits are unreliable


  /*

  // calc diffs between fit and data

  double yfit[nbins], ydiff[nbins];
  for (int i=0; i<n; i++) {
    yfit[i]= linfitpars[0] + linfitpars[1]*x[i];
    ydiff[i] = yfit[i] - relmpv[i];
  }

  //  new TCanvas;
  TGraph *gyfit = new TGraph(n,x,yfit);
  gyfit->SetMarkerColor(kTeal);
  gyfit->Draw("Psame");

  new TCanvas;
  TGraph *gydiff = new TGraph(n,x,ydiff);
  gydiff->SetMarkerColor(kTeal);

  gydiff->Draw("AP");

  */
  ///-------------------------------------------------------

  //  new TCanvas;
  TGraph *g2 = new TGraph(n,x,mpvsig);
  g2->SetTitle(Form("Run %i  MPV/sigma for Landau fitted to amplitude;doca (cm);amplitude fit MPV/sigma",run));
  //  g2->SetMinimum(0);
  g2->Draw("AP");

  g2->Fit(linfit,"R","",0.1,0.4);  //fit from 0.18 to 0.3
  linfit->GetParameters(linfitpars);

  g2->GetYaxis()->SetTitle("amplitude MPV/sigma, fitted from 0.1 to 0.4cm");

  if (saveplots) gPad->SaveAs(Form("plots/run_%i_mpvsig.png",run));

  // almost all of these start off at ~ 3.4 to 3.5 at x=0

  // exceptions (2.6 ish): runs 40898, 40903, 41155, 41201, 41386, 

  // earlier runs are flat until about 0.55mm, then add scatter and curve down
  // later runs 41499 + curve up w doca instead of down, from about 4mm on.



  double msyfit[nbins], msydiff[nbins];
  for (int i=0; i<n; i++) {
    msyfit[i]= linfitpars[0] + linfitpars[1]*x[i];
    msydiff[i] = msyfit[i] - mpvsig[i];
  }


  //  new TCanvas;
  TGraph *gmsyfit = new TGraph(n,x,msyfit);
  gmsyfit->SetMarkerColor(kMagenta);
  gmsyfit->Draw("Psame");

  // Use a gaussian width gw to smooth the mpv/sigma data


  TF1 *gg = new TF1("gg","gaus",-100,100);
  const int gw=8;
  gg->SetParameters(1,0,0.5*gw);
  double w[1+2*gw], wtot;
  for (int i=-1*gw; i<gw+1; i++) w[i+gw] = gg->Eval((float)i);


  double msmooth[nbins], msmdiff[nbins];

  for (int i=0; i<n; i++) {
    wtot = 0;
    msmooth[i] = 0;

    for (int j=i-gw;  j<=i+gw; j++) {
      if (j<0) continue;
      if (j>=n) continue;

      msmooth[i] += mpvsig[j]*w[gw+j-i];
      wtot += w[gw+j-i];

      //      if (i==7) cout << mpvsig[j] <<" " << w[gw+j-i] << endl;
    }

    msmooth[i] = msmooth[i]/wtot;
    msmdiff[i] = msyfit[i] - msmooth[i];

  }


  
  TGraph *gsmooth = new TGraph(n,x,msmooth);
  gsmooth->SetMarkerColor(66);
  gsmooth->Draw("Psame");
  

  new TCanvas;
  TGraph *gdiff2 = new TGraph(n,x,msmdiff);
  gdiff2->SetMarkerColor(kRed);
  gdiff2->SetTitle("MPV/sigma deviation from fit; DOCA (cm); linear function fitted to mpv/sig - smoothed mpv/sig value");
  gdiff2->Draw("AP");

  
  double xmax = 0.65;

  // the deviation from the fit is largest at large DOCA
  // it is small in the fit range and then diverges
  // early runs go to one extreme, late runs to the other

  // start at end of fit range (0.4) and look for diff > 0.05

  xmax = 0.65;

  for (int i=0; i<n; i++) {
    if (x[i] < 0.4) continue;

    if (fabs(msmdiff[i]) >= 0.05) xmax = x[i];
    if (fabs(msmdiff[i]) >= 0.05) break;

  }

  



  printf("New suggested stop at %.2f cm\n",xmax);

  gdiff2->SetTitle(Form("Run %i MPV/sigma deviation from fit, suggested stop at %.2f cm",run,xmax)); 

  if (saveplots) gPad->SaveAs(Form("plots/run_%i_fitdev.png",run));

  //--------------------------------------------------------------------

  // do linear fit to relmpv using new stop val

  new TCanvas;
  TGraph *grelmpv = new TGraph(n,x,relmpv);
  grelmpv->SetTitle(Form("Run %i  Fitted amplitude MPV/fitted MPV for 0-3mm;doca (cm);ratio of amplitude / amplitude for 0-3mm",run));

  grelmpv->SetMinimum(0);
  grelmpv->Draw("AP");

  float fitstart = 0.18;
  float fitstop = (float)xmax;

  grelmpv->Fit(linfit,"R","",fitstart,fitstop); 
  linfit->GetParameters(linfitpars);

  printf("Fit params, adjusted  %.4f %.4f\n",linfitpars[0]*mpvratio,linfitpars[1]*mpvratio);


  double fittedvals[n];
  for (int i=0; i<n; i++) fittedvals[i] = linfitpars[0] + linfitpars[1]*x[i];

  // compare fittedvals to relmpv just after the fit ends
  // if the diff is < 0.05, extend xmax until diff > 0.05 or xmax = 0.65

  
  // xmax = 0.65;

  for (int i=0; i<n; i++) {
    if (x[i] < xmax) continue;

    if (x[i] >= 0.65) break;
    if (fabs(fittedvals[i] - relmpv[i]) >= 0.02) break;
    xmax = x[i];

  }

  if (xmax > 0.65) xmax = 0.65;

  printf("Rel mpv is within 0.02 of fitted vals out to %.2f cm\n",xmax);



  TGraph *gnewfit = new TGraph(n,x,fittedvals);
  gnewfit->SetMarkerColor(38);

  new TCanvas;

  grelmpv->Draw("AP");
  gnewfit->Draw("Psame");
  grelmpv->Draw("same");

  TPaveText *txt = new TPaveText(0.2, 0.2, 0.4, 0.3,"NDC");
  txt->AddText(Form("max doca %.2f cm",xmax));
  txt->SetBorderSize(0);
  txt->SetFillStyle(0);
  txt->Draw();


  if (saveplots) gPad->SaveAs(Form("plots/run_%i_mpv.png",run));


  char filename3[150],filename4[150];;
  sprintf(filename3, "ccdb/gaindoca_%i.txt",run);
  if (saveextras) sprintf(filename4, "mpv3mm/mpvratio_%i.txt",run);

  printf("Writing fit parameters into %s\n",filename3);
  if (saveextras) printf("Writing fit parameters into %s and mpv ratio into %s\n",filename3,filename4);


  FILE *outfile = fopen(filename3,"w");
  //  fprintf(outfile,"0.65 0.1 1.0613 -0.4659 %.4f %.4f\n",linfitpars[0]*mpvratio,linfitpars[1]*mpvratio);
  fprintf(outfile,"%.2f 0.1 1.0617 -0.4482 %.4f %.4f\n",xmax,linfitpars[0]*mpvratio,linfitpars[1]*mpvratio);
  fclose(outfile);

  if (saveextras) {
    outfile = fopen(filename4,"w");
    fprintf(outfile,"%.4f\n",mpvratio);
    fclose(outfile);

    outfile = fopen("stoplist.txt","a");
    fprintf(outfile,"run %i mpv3mmratio %.3f fitstoppoint %.2f cm xmax %.2f\n",run,mpvratio,fitstop, xmax);
    fclose(outfile);
  }

  if (save) {
    outfile = fopen("addtoccdb.sh","a");
    fprintf(outfile,"ccdb add CDC/gain_doca_correction -r %i-%i ccdb/gaindoca_%i.txt\n",run,run,run);
    fclose(outfile);

    printf("Added params to the list in addtoccdb.sh\n");
  }
}
