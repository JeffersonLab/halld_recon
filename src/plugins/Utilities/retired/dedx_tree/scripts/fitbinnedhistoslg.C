
// Fits dedx vs p histograms in root file (name hard-coded)
// Uses a Gaussian to fit the proton peak and a Landau*Gaussian convolution for the pion peak
// Writes results into a tree and saves pictures of each fitted histo to simfitpics and simbadfitpics

// uses these directories
//    mkdir simfitpics
//    mkdir simbadfitpics

//    mkdir simbadfitpics/p
//    mkdir simbadfitpics/pi
//    mkdir simfitpics/p
//    mkdir simfitpics/pi


#include "langaus.C"       // this contains the Landau*Gaussian fit function (obtained from root's example code )


TF1 *onelangaufit(Char_t *FunName, TH1D *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF)
{

   Int_t i;

   TF1 *ffitold = (TF1*)gROOT->GetListOfFunctions()->FindObject(FunName);
   if (ffitold) delete ffitold;

   TF1 *ffit = new TF1(FunName,langaufun,fitrange[0],fitrange[1],4);

   ffit->SetNpx(100);
   ffit->SetParameters(startvalues);

   ffit->SetParNames("Width","MP","Area","GSigma");

   for (i=0; i<4; i++) {
      ffit->SetParLimits(i, parlimitslo[i], parlimitshi[i]);
   }

   his->Fit(FunName,"RB0WE");   // fit within specified range, use ParLimits, do not plot

   ffit->GetParameters(fitparams);    // obtain fit parameters
   for (i=0; i<4; i++) {
      fiterrors[i] = ffit->GetParError(i);     // obtain fit parameter errors
   }
   ChiSqr[0] = ffit->GetChisquare();  // obtain chi^2
   NDF[0] = ffit->GetNDF();           // obtain ndf

   return (ffit);              // return fit function

}


void fitbinnedhistoslg(void) {


  // graph style options
  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.5);
  gStyle->SetOptFit(1111);
  gStyle->SetOptStat(0);
  gStyle->SetOptLogz(1);



  bool saveplots=1;   // save plot png files

  if (saveplots) {
    bool missingdir = 0;
    char dirneeded[6][20] = {"simfitpics","simfitpics/p","simfitpics/pi","simbadfitpics","simbadfitpics/p","simbadfitpics/pi"};
    for (int i=0; i<6; i++) {
      if (!gSystem->OpenDirectory(dirneeded[i])) {
	  printf("mkdir %s\n",dirneeded[i]);
          missingdir = 1;
      }
    }
    if (missingdir) gApplication->Terminate();
  }


  bool sim=1;   //  if 1, fit simulated data - cuts are not theta-dependent

  // set to 1 to fit protons, pions, 0 to skip
  bool fitp = 1;
  bool fitpi = 1;


  TFile *f;     // file of binned histos

  if (sim) f = new TFile("/home/njarvis/online/dedx/16hits/sim_binned_histos_16hits_83files.root","READ");

 if (!sim) f = new TFile("/home/njarvis/online/dedx/16hits/binned_histos_16hits_337files.root","READ");


  // 161 histos of each q+ and q-  , dedxp+q+[n]

 // For 20+ hits, settled on range 45 degrees to 130 degrees.  
 // For 16+ hits, fitting 20 to 150 degrees

 const int nhistos=1+140-10; //1+130-45; //161;  // # dedx vs p histos to process, ie # theta values, ultimately 161
 const int npbins=100;  // # bin in p fitted, ie #p values , ultimately 100
 const int firsthisto=10; //35; //45 degrees  //0; // ultimately 0, index of first histo to process


    // p axis starts at -0.025, bin width is 0.05. 
    // bin 1 center is 0
    // bin 2 center is 0.05
    // bin 3 center is 0.1

  float pstart = 0.05;  // should be 0.05    // start bin value for momentum 
  float dp=0.05; // should be 0.05;         // increment

  //  TH2D *dedxp[nhistos];
  TH2D *hpos;
  TH2D *hneg;

  
  // fit functions

  TF1 *g = new TF1("gaus","gaus",0,25);
  TF1 *lan = new TF1("landau","landau",0,25);

  g->SetLineColor(kBlue);

  TH1D *ypro;

  double fitstat1, fitstat2, pars1[3],pars2[3],chisq1,chisq2;
  int ndof1,ndof2;

  float tstart,tstop;

  float theta;
  float p;

  int pid;   // 0: deuteron   1:proton   2:kaon   3:pion   4:merged p+pi band above 1.2GeV

  const float minpeakheight=10.0;  // min acceptable peak height

  const int nneededtofit=150; // entries in a histo needed to fit.
  const int nneededtoproject=500; // entries in a histo needed to cut.

  // error tolerances

  const float meanabserrtol=0.1; // keep fits with error on the mean less than this
  const float meanfracerrtol=0.01; // keep fits with error on the mean less than this

  const float meanfracerrtol_d=0.1; // for d band: keep fits with error on the mean less than this
  const float meanfracerrtol_k=0.1; // for k band: keep fits with error on the mean less than this

  // lines cutting between the bands. 
  // the parameter values depend on theta and are set later

  float pcutmax = 4.0;
  TF1 *fp = new TF1("fp","exp([0]*(x + [1])) + [2]", 0.0, pcutmax);

  float kcutmax = 0.85;
  TF1 *fk = new TF1("fk","exp([0]*(x + [1])) + [2]", 0.0, kcutmax);

  float picutmax = 0.425;
  TF1 *fpi = new TF1("fpi","exp([0]*(x + [1])) + [2]", 0.0, picutmax);


  // set up the output tree for the results

  TTree *t = new TTree("t","fit results");

  t->Branch("p",&p,"p/F");
  t->Branch("theta",&theta,"theta/F");

  double nentries;
  double counts;  // in the region for the pid of interest
  t->Branch("n",&counts,"n/D");
  t->Branch("pid",&pid,"pid/I");


  // arrays to contain results from the fit function
  double fitstat, fitpars[4],fiterrs[4];
  char parnames[4][10] ={"ht","mean","sigma","gsigma"}; 
  char errnames[4][10] ={"err_h","err_m","err_s","err_gs"}; 


  for (int i=0; i<4; i++) {
    t->Branch(parnames[i],&fitpars[i],Form("%s/D",parnames[i]));
    t->Branch(errnames[i],&fiterrs[i],Form("%s/D",errnames[i]));
  }

  // LG fit params repeated temporarily
    Double_t fpars_lg[4]; 
    Double_t fpe[4]; 


  for (int i=0; i<4; i++) t->Branch(Form("fp%i",i),&fpars_lg[i],Form("fp%i/D",i));
  for (int i=0; i<4; i++) t->Branch(Form("fpe%i",i),&fpe[i],Form("fpe%i/D",i));


  //  text file to contain list of fit failures for the pi peak
  //  FILE *listbadpi = fopen("badpifits.txt","w");

  // histogram bin number is theta - 10

  for (int ihisto=firsthisto; ihisto<firsthisto+nhistos; ihisto++) {

    // load in the theta-dependent cut line params for k (top of k band) and p (top of p band). 
    // might also need to check these for pi band (bottom of k) 
    // can only see the k band for a small range of p & theta


    if (sim) {    //use the 45 degree fits for all theta
        fp->SetParameters(-4.6, -0.98, 5.2); 
        fk->SetParameters(-4.6,-0.69,2.7); 
        fpi->SetParameters(-6.5, -0.43, 2.75);

    } else if (fabs(ihisto-80) < 10) {     // ihisto 71-89
        fp->SetParameters(-4.7, -0.88, 4.0);     // good for 90 degr
        fk->SetParameters(-4.8,-0.65,2.3);    // good for 90 degr 
        fpi->SetParameters(-6.5, -0.4, 2.4);
	
    } else if (fabs(ihisto-80) < 20) {    // ihisto 61-70, 90-99 
        fp->SetParameters(-4.6, -0.90, 4.8);  // 80 degr
	fk->SetParameters(-5.0,-0.64,3.0);
        fpi->SetParameters(-6.5, -0.4, 2.8);

    } else if (fabs(ihisto-80) < 30) {  // ihisto 51-60, 100-109
        fp->SetParameters(-4.6, -0.95, 4.8);  // 70 degr
        fk->SetParameters(-4.8,-0.69,2.7);
        fpi->SetParameters(-6.5, -0.43, 2.75);

    } else if (fabs(ihisto-80) < 40) {  // ihisto 41-50, 110-119
        fp->SetParameters(-4.6, -0.98, 5.2);  // 51-60 degr
        fk->SetParameters(-4.6,-0.7,2.3); // good at 60 degr
        fpi->SetParameters(-6.5, -0.43, 2.75);

    } else if (fabs(ihisto-80) < 50) {  // ihisto 31-40, 120-129
        fp->SetParameters(-4.6, -0.98, 5.2);  // 41-50 degr
        fk->SetParameters(-4.6,-0.69,2.7); // good at 50 degr
        fpi->SetParameters(-6.5, -0.43, 2.75);

    } else { //if (fabs(ihisto-80) < 55) {    // ihisto < 31, > 129
        fp->SetParameters(-4.6, -0.98, 5.2);  // up to 40 degr
        fk->SetParameters(-5.0, -0.7, 2.9);
        fpi->SetParameters(-6.5, -0.43, 2.75);

    }

    // read in the q+ histogram from the root file of binned histos

    f->cd();

    hpos = (TH2D*)gDirectory->Get(Form("dedxp_q+[%i]",ihisto));

    cout << hpos->GetTitle() << " entries " << hpos->GetEntries() << endl;

    if (hpos->GetEntries() < nneededtoproject) continue;

    //    new TCanvas;
    //  hpos->Draw("colz");

    sscanf(hpos->GetTitle(),"theta %f to %f degrees",&tstart,&tstop);
    theta = 0.5*(tstart+tstop);

    if (hpos->GetEntries() < nneededtoproject) continue;

    bool good;
    bool checkfits = 0; // switch off fit quality checks

    // langaus fit params
    Double_t fr[2];  
    Double_t psv[4], pisv[4], pllo[4], plhi[4]; //, fpe[4];  //using variants of fp[]
    //    TF1* fitlg;
    //    Double_t fpars_lg[4] = {0};
    Double_t lgpeak, lgwidth;
    Char_t funcname[100];   //name for current fit function
    sprintf(funcname,"LanGaus");

    Double_t chisqr;
    int ndf;  

    // scan through p bins
    for (int ip = 0; ip < npbins; ip++) {

      p = pstart + ip*dp;

      double pbin = hpos->GetXaxis()->FindBin(p);
      ypro = hpos->ProjectionY("p",pbin,pbin);

      ypro->SetTitle(Form("q+ %s momentum %.2f GeV/c",hpos->GetTitle(),p));

      nentries = ypro->GetEntries();

      //cout << "p " << p << " theta " << theta << " " << "entries " << nentries << endl;

      if (nentries < nneededtofit) continue;

      double yrangemax = ypro->GetYaxis()->GetXmax();

      // use dE/dx cut func to separate the bands, find the max bin either side

      new TCanvas; ypro->DrawCopy();

      // cut functions

      double pitop =  fpi->Eval(p,0,0); 
      double ktop =  fk->Eval(p,0,0); 
      double ptop =  fp->Eval(p,0,0); 
      double dtop =  ptop + 6;

      if (p > picutmax) pitop = ktop;



      // find counts between these bins

      double pcounts = ypro->Integral(ypro->GetXaxis()->FindBin(ktop),ypro->GetXaxis()->FindBin(ptop));
      double picounts = ypro->Integral(0,ypro->GetXaxis()->FindBin(pitop));


      // set fit results to 0 in case the fit is switched off or there aren't enough counts 
      fitstat = 1;

      for (int i=0; i<4; i++) {
        fitpars[i] = 0;
        fiterrs[i] = 0;   
        fpars_lg[i] = 0;
        fpe[i] = 0;
      }



      // ------------------ protons -------------------------------

       printf("\n\n\n p %.2f GeV/c theta %2f degrees\n\n",p,theta);
       printf("Counts in peak regions: p %f pi %f\n", pcounts, picounts);

      bool unfittable = 0;

      // unfittable because the p and pi are too close
      if (fabs(p-0.75)<0.01 && (theta > 67 && theta < 82)) unfittable = 1;
      if (fabs(p-0.80)<0.01 && (theta > 66 && theta < 83)) unfittable = 1;
      if (fabs(p-0.85)<0.01 && (theta > 64 && theta < 80)) unfittable = 1;
      
      // unfittable due to stats
      if (fabs(p-0.80)<0.01 && (theta > 99 && theta < 102)) unfittable = 1;
      if (fabs(p-0.85)<0.01 && (theta > 79 && theta < 84)) unfittable = 1;


      pid = 1;

      if (fitp && !unfittable && p<=kcutmax && pcounts>nneededtofit) {

        // find ktop and ptop, fit data between ktop and ptop with gauss

        g->SetParameter(1, 0.5*(ktop+ptop));
        g->SetParameter(2,1);

        fitstat = ypro->Fit(g,"","",ktop,ptop);

        //cout << "p fit, ktop  " << ktop << " ptop " << ptop << endl;

        good = 1;

        if (!fitstat) {

          g->GetParameters(&fitpars[0]);
          for (int i=0; i<3; i++) fiterrs[i] = g->GetParError(i);      

          // check peak is within range
          if (fitpars[1] < ktop) good = 0;
          if (fitpars[1] > ptop) good = 0;

          // check peak height
          if (fitpars[0] < minpeakheight ) good = 0;  //not enough counts

          // check error:param ratio
          for (int i=0; i<3; i++) if (fiterrs[i] >= fitpars[i]) good = 0;  
  

	  if (good && checkfits) {

    	    if (fitpars[1] < ktop + 1.0*fitpars[2] ) good = 0;  // peak & cut within 1sigma
    	    if (fitpars[1] < ktop + 1.0*fitpars[2] ) printf("mean - ktop %.2f < limit %.2f\n",ktop,1.0*fitpars[2]);
  
            if (fiterrs[1] >= meanabserrtol) printf("error on the mean %.2f > limit %.2f\n",fiterrs[1],meanabserrtol); 
            if (fiterrs[1] >= meanabserrtol) good = 0;
  
            if (fiterrs[1]/fitpars[1] >= meanfracerrtol) printf("err:mean %.2f > limit %.2f\n",fiterrs[1]/fitpars[1],meanfracerrtol);
            if (fiterrs[1]/fitpars[1] >= meanfracerrtol) good = 0;
  	  }
        } else {
        
          good = 0;

        }

        if (saveplots) ypro->GetYaxis()->SetRangeUser(0,1.5*fitpars[0]);
        if (saveplots) gPad->Update();

        if (good) {

          counts = pcounts;
          t->Fill();

          if (saveplots) gPad->SaveAs(Form("simfitpics/p/dedx_p%.2f_theta%.0f.png",p,theta));
          //if (saveplots) gPad->SaveAs(Form("simfitpics/p/theta%.0f_dedx_p%.2f.png",theta,p));
        } else { 
          if (saveplots) gPad->SaveAs(Form("simbadfitpics/p/dedx_p%.2f_theta%.0f.png",p,theta));
          //if (saveplots) gPad->SaveAs(Form("simbadfitpics/p/theta%.0f_dedx_p%.2f.png",theta,p));
        }

      }



      // ------------------ pions -------------------------------

      pid = 3;

      unfittable = 0;

      // unfittable because the p and pi are too close
      if (fabs(p-0.75)<0.01 && (theta > 85 && theta < 90)) unfittable = 1;
      //if (fabs(p-0.70)<0.01 && (theta > 87)) unfittable = 1;
      if (fabs(p-0.80)<0.01 && (theta > 78 && theta < 85)) unfittable = 1;      
      if (fabs(p-0.85)<0.01 && (theta<63 || theta>75)) unfittable = 1;

      // unfittable due to stats
      //  if (fabs(p-0.55)<0.01 && (theta > 117 && theta < 119)) unfittable = 1;
      //  if (fabs(p-0.6)<0.01 && (theta > 105 && theta < 108)) unfittable = 1;


      if (fitpi && !unfittable && (p <= kcutmax ) && picounts>nneededtofit) {

	// printf("\n\n\n p %.2f GeV/c theta %2f degrees\n\n",p,theta);
        //  printf("Counts in peak regions: d: %f p %f K %f pi %f\n", dcounts, pcounts, kcounts, picounts);
          // find ktop and ptop, fit data between ktop and ptop with gauss

          g->SetParameter(1, 0.5*pitop);
          g->SetParameter(2,1);
          fitstat = ypro->Fit(g,"","",1,pitop);
          g->GetParameters(&fitpars[0]);

          if (!fitstat && (fitpars[1]<pitop)) {   // set start vals for langaus
          //   par[0]=Width (scale) parameter of Landau density
          //   par[1]=Most Probable (MP, location) parameter of Landau density
          //   par[2]=Total area (integral -inf to inf, normalization constant)
          //   par[3]=Width (sigma) of convoluted Gaussian function

              pisv[0] = 0.2; //0.1*fitpars[2]; //0.1*dneg->GetRMS(); //lpars[2]; //1.8; 
              pisv[1] = 0.9*fitpars[1];
              pisv[2] = 2.0*fitpars[0]; //0.1*picounts;
              pisv[3] = fitpars[2]; //0.4;
          } else {
              pisv[0] = 0.2; //0.1*fitpars[2]; //0.1*dneg->GetRMS(); //lpars[2]; //1.8; 
              pisv[1] = 0.5*(1.0+pitop);
              pisv[2] = 0.1*picounts;
              pisv[3] = 0.4;
	  }


        fr[0] = 1; 
        fr[1]=pitop; //3.0*dneg->GetMean();
	  
        good = 1;
 
    //   TF1 *langaufit(TH1F *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF)
   //   par[0]=Width (scale) parameter of Landau density
   //   par[1]=Most Probable (MP, location) parameter of Landau density
   //   par[2]=Total area (integral -inf to inf, normalization constant)
   //   par[3]=Width (sigma) of convoluted Gaussian function
 
        cout << "initial guesses: " << pisv[0] << " " << pisv[1] << " " << pisv[2] << " " << pisv[3] << endl;

       pllo[0]=0.05; pllo[1]=1.0;     pllo[2]=pisv[2]/5.; pllo[3]=0.05; //pisv[3]*0.66;
       plhi[0]=1.0; plhi[1]=pitop;  plhi[2]=pisv[2]*5.; plhi[3]=1.0; //pisv[3]*1.33;

       TF1* fitlg = onelangaufit(funcname,ypro,fr,pisv,pllo,plhi,fpars_lg,fpe,&chisqr,&ndf);

       fitlg->Draw("same");

       printf ("Chi sq %f  NDF %i   Chi sq/NDF %f\n",chisqr,ndf,chisqr/(float)ndf);

       // check peak is inside fit range 
       if (fitpars[1] < fr[0]) good = 0;
       if (fitpars[1] > fr[1]) good = 0;

       // check peak height
       fitpars[0] = fitlg->Eval(fitpars[1]);
       if (fitpars[0]<minpeakheight) good = 0; // not enough counts

          // check error:param ratio
       for (int j=0; j<4; j++) if (fpe[j] >= fpars_lg[j]) good = 0;  



       if (good && checkfits) {

         for (int j=0; j<4; j++) if (fpe[j] > 0.2*fpars_lg[j]) good = 0;
         for (int j=0; j<4; j++) if (fpe[j] > 0.2*fpars_lg[j]) printf("*** param %i error > 0.1 x param val ***\n",j);
  
  
         if (fpe[0] >= 2.0*meanabserrtol) good = 0; 
         if (fpe[0] >= 2.0*meanabserrtol) printf("*** Width error %.2f is too large ***\n",fpe[0]);
  
         if (fpe[1] >= meanabserrtol) good = 0;
         if (fpe[1] >= meanabserrtol) printf("*** Peak error %.2f is too large ***\n",fpe[1]);
       }

       if (good) {

          counts = picounts;
   
	  //           fitpars[0] = fpars_lg[2];     //   par[2]=Total area (integral -inf to inf, normalization constant)
           fitpars[1] = fpars_lg[1];  //lgpeak; //fpars_lg[0];
           fitpars[2] = 0.5*lgwidth; //fpars_lg[0];
           fitpars[3] = 0.5*fpars_lg[3];   // half gaussian sigma

           fiterrs[0] = 0; //fpe[2];         // total area
           fiterrs[1] = fpe[1];         // mpv error      //   par[1]= MPV of Landau density
           fiterrs[2] = 0.5*fpe[0];  // landau width  //   par[0]=Width (scale) param of Landau density
           fiterrs[3] = 0.5*fpe[3];    //   par[3]=Width (sigma) of convoluted Gaussian function

       }


        if (saveplots) ypro->GetYaxis()->SetRangeUser(0,1.0*fpars_lg[2]);
        if (saveplots) ypro->GetXaxis()->SetRangeUser(0,10);
        if (saveplots) gPad->Update();

        if (good) {

          t->Fill();

          if (saveplots) gPad->SaveAs(Form("simfitpics/pi/dedx_p%.2f_theta%.0f.png",p,theta));
          //if (saveplots) gPad->SaveAs(Form("simfitpics/pi/theta%.0f_dedx_p%.2f.png",theta,p));

        } else { 

	  //          fprintf(listbadpi,"theta %.0f p %.2f \n",theta,p);

          if (saveplots) gPad->SaveAs(Form("simbadfitpics/pi/dedx_p%.2f_theta%.0f.png",p,theta));
          //if (saveplots) gPad->SaveAs(Form("simbadfitpics/pi/theta%.0f_dedx_p%.2f.png",theta,p));
        }

      }


    } // ip loop (momentum)

  }  // ihisto loop (theta)


  // save the fit results to a new tree file

  TFile *outfile;

  if (sim) outfile = new TFile("fittedsimresults.root","RECREATE");
  if (!sim) outfile = new TFile("fitresults.root","RECREATE");
  outfile->cd();
  t->Write();

  outfile->Close();

  //  fclose(listbadpi);

}


