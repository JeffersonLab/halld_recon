void fit_dedx(int run=1){

  // fit dedx in open file, run is the run number

  // if fit at 1.5GeV/c is successful, writes mean dedx to mip/runnnnnn.txt

  // CDC_dedx was not included in the plugins
  // have to use monitoring_hists


  TF1 *g = new TF1("gaus","gaus",0,12);

  double fitstat;
  double pars[3];

  TH1D *p;   // projection
  float pcut;
  int pbin;

  char opfilename[100];
  sprintf(opfilename,"dedx_peak_scan.txt");

  char meandedxfile[100];
  sprintf(meandedxfile,"dedx_mean_at_1_5GeV.txt");


  FILE *outfile = fopen(opfilename,"a");
  FILE *outfilejustnumbers = fopen("run_pi_mean_sig_p_mean_sig_both_mean_sig.txt","a");
  FILE *meanfile = fopen(meandedxfile,"a");

  const float newdedx = 2.01992;   // put the 1.5GeV/c cut at this dE/dx

  const int testing=0;

  TDirectory* tdir = gDirectory->GetDirectory("CDC_dedx");

  if (!tdir) return;

  tdir->cd();

  TH2D* h = (TH2D*)gDirectory->Get("dedx_p_pos");

  if (!h) return;


    //    rootfile->cd("Independent/Hist_DetectorPID/CDC");
    // h = (TH2D*)gDirectory->Get("dEdXVsP_q+");
  if (testing) new TCanvas;
  if (testing) h->Draw();


    // draw cut through histo at p=1.5 GeV/c

    pcut = 1.5;
    pbin = h->GetXaxis()->FindBin(pcut);

    p = h->ProjectionY("p1",pbin,pbin);

    if (p->GetEntries() < 15000) printf("Run %i entries %.0f (most have 30k+)\n",run,p->GetEntries());
    // will need to combine files for empty target runs

    if (testing) p->SetTitle(Form("CDC q+ dE/dx at %.2f GeV/c",pcut));
    if (testing) p->GetXaxis()->SetRangeUser(0,12);

    if (testing) new TCanvas;
    if (testing) p->DrawCopy("");

    fitstat = p->Fit(g,"Q0WE");  // fitstat=0 is good
  
    double resboth,uboth;

    if (fitstat == 0) {
      g->GetParameters(&pars[0]);

      resboth = 2.0*pars[2]/pars[1];
      uboth = pars[1];

      if (testing) g->DrawCopy("same");

      //printf("Run %i dE/dx at p=1.5 GeV/c mean %.2f  width %.2f  res %.2f\n",run,pars[1],2*pars[2],resboth[n]);
      //printf("Run %i dE/dx at p=1.5 GeV/c mean %.2f  width %.2f  res %.2f\n",run,pars[1],2*pars[2],resboth[n]);
      fprintf(meanfile,"%06i %.3f\n",run,pars[1]);

      FILE *fdedx = fopen(Form("mip/run_%i.txt",run),"w"); 
      fprintf(fdedx,"%.3f\n",pars[1]);
      fclose(fdedx);

   } else {
      printf("Run %i Fit failed\n",run);
    }

    if (uboth==0) return;   // skip the 0.5 GeV/c fits if the 1.5 GeV/c fit failed

    
    // draw cut through histo at p=0.5 GeV/c

    pcut = 0.5;
    pbin = h->GetXaxis()->FindBin(pcut);

    p = h->ProjectionY("p1",pbin,pbin);

    if (testing) p->SetTitle(Form("CDC q+ dE/dx at %.2f GeV/c",pcut));
    if (testing) p->GetXaxis()->SetRangeUser(0,12);

    if (testing) new TCanvas;
    if (testing) p->DrawCopy("");

    // fit pion line

    fitstat = p->Fit(g,"Q0WE","",0,2*uboth);  // fitstat=0 is good
  
    double respi,upi;

    if (fitstat == 0) {
      g->GetParameters(&pars[0]);
      respi = 2.0*pars[2]/pars[1];
      upi = pars[1];

      if (testing) g->DrawCopy("same");

      //   printf("Run %i dE/dx at p=0.5 GeV/c mean %.2f  width %.2f  res %.2f\n",run,pars[1],2*pars[2],respi[n]);

    } else {
      printf("Fit failed\n");
      respi=0;
      upi=0;
    }

    // fit p line

    fitstat = p->Fit(g,"Q0WE","",2*uboth,12);  // fitstat=0 is good
    double resp,up;  

    if (fitstat == 0) {
      g->GetParameters(&pars[0]);
      resp = 2.0*pars[2]/pars[1];
      up = pars[1];

      if (testing) g->DrawCopy("same");

      //   printf("Run %i dE/dx at p=0.5 GeV/c mean %.2f  width %.2f  res %.2f\n",run,pars[1],2*pars[2],resp[n]);

    } else {
      printf("Fit failed\n");
      resp=0;
      up=0;
    }

    fprintf(outfile,"%i  0.5GeV/c pi %.2f %.2f  p %.2f %.2f  1.5 GeV/c %.2f %.2f\n",run,upi,respi,up,resp,uboth,resboth);

    //#      fprintf(addfile,"ccdb add CDC/digi_scales -r %i-%i /home/njarvis/calibfall2019/dedx/ver17/ccdb/new_ascale_%i.txt\n",run,run,run);

 
    fclose(outfile);

    fprintf(outfilejustnumbers,"%i %.2f %.2f %.2f %.2f %.2f %.2f\n",run,upi,respi,up,resp,uboth,resboth);

    fclose(outfilejustnumbers);


  //  if (calcnewgains) fclose(addfile);

}
