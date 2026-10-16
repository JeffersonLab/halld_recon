
void makescalefactors(){

  // sim file has 12 entries for p, 16 for pi+
  // read these into an array
  // then read in the real file, make new tree
  // dedx, theta, correction-factor,pid
  // include pid for tracking down weird points

  // also check to make sure that there aren't enough d in sim to fit


  TFile *f = new TFile("fittedsimresults.root","READ");
  TTree *simt = (TTree*)gDirectory->Get("t");
  int simentries = (int)simt->GetEntries();

  const int simarraysize=1000;   //992 proton entries.  only using 90 degrees for sim though.

  int pid;
  // 0: d   1: p    2: K    3: pi    4: combined

  float p, theta;
  double mean, err, ht;

  simt->SetBranchAddress("pid",&pid);
  simt->SetBranchAddress("p",&p);
  simt->SetBranchAddress("ht",&ht);
  simt->SetBranchAddress("theta",&theta);
  simt->SetBranchAddress("mean",&mean);
  simt->SetBranchAddress("err_m",&err);

  float sim_proton_theta[simarraysize];
  float sim_pion_theta[simarraysize];
  
  float sim_proton_mtm[simarraysize];
  float sim_pion_mtm[simarraysize];

  double sim_proton_dedx[simarraysize];
  double sim_pion_dedx[simarraysize];

  int np=0;
  int npi=0;

  bool usethis;

  // load in sim data for limited range of theta

  for (int i=0; i<simentries; i++) {

    simt->GetEntry(i);

    // round p to 2 sig figs to make code easier
    p = 0.01*int(p*100);

    if (pid==1 && (int)theta>44 && (int)theta<131 && ht>25) {
        sim_proton_mtm[np] = p;
        sim_proton_dedx[np] = mean;
        sim_proton_theta[np] = theta;
        np++;

    } else if (pid==3 && (int)theta==90 && ht>100) {

        sim_pion_mtm[npi] = p;
        sim_pion_dedx[npi] = mean;
        sim_pion_theta[npi] = theta;
        npi++;    

    }
  } 
  
  f->Close();

  

  f = new TFile("fitresults.root","READ");
  TTree *t = (TTree*)gDirectory->Get("t");
  int nentries = (int)t->GetEntries();

  t->SetBranchAddress("ht",&ht);
  t->SetBranchAddress("pid",&pid);
  t->SetBranchAddress("p",&p);
  t->SetBranchAddress("theta",&theta);
  t->SetBranchAddress("mean",&mean);
  t->SetBranchAddress("err_m",&err);



  TTree *newt = new TTree("t","dedx correction factors");

  float dedx, err_dedx, cf;

  newt->Branch("theta",&theta,"theta/F");
  newt->Branch("dedx",&dedx,"dedx/F");
  newt->Branch("err_dedx",&err_dedx,"err_dedx/F");
  newt->Branch("f",&cf,"f/F");
  // extras in case of problems
  newt->Branch("p",&p,"p/F");
  newt->Branch("pid",&pid,"pid/I");

  double firstdedx;  // use this to save proton dedx at 45 or 67 degrees
  double lastdedx;
  double lookfortheta;
  bool found;

  int itheta;
  float simp;

  for (int i=0; i<nentries; i++) {

    t->GetEntry(i);

    // round p to 2 sig figs to make code easier
    p = 0.01*int(p*100);
    itheta = (int) theta;

    found = 0;
    usethis = 1;

    // printf("looking for pid %i and mtm %f\n",pid,p);

    //    if (pid == 1 && itheta == 66 && p ==0.8) continue; // remove outlier

    if (pid ==1 && itheta >= 45 && itheta <= 130 && p > 0.349) {

      if (ht < 60) usethis = 0;

      if (p==0.35 && (itheta < 67)) usethis = 0;
      if (p==0.35 && (itheta > 83)) usethis = 0;

      if (usethis) {

        found = 0;
        firstdedx = 0;

        for (int j = 0; j < np; j++) {   // find dedx for the first theta for the current p

          if (p != sim_proton_mtm[j]) continue;

          lastdedx = sim_proton_dedx[j];  // they are ordered by theta and then by p

          if (p == 0.35 && (int)sim_proton_theta[j]==67) {
              firstdedx = sim_proton_dedx[j];
          } else if ((int)sim_proton_theta[j]==45) {
              firstdedx = sim_proton_dedx[j];
          } 

        }

        printf("set firstdedx to %f for p %f \n",firstdedx,p);

        for (int j = 0; j < np; j++) {

          if (p != sim_proton_mtm[j]) continue;

          if (itheta == (int)sim_proton_theta[j]) {
            found = 1;
            dedx = mean;
            err_dedx = err;
            cf = sim_proton_dedx[j]/dedx;
          }
          if (found) printf("Found a direct match for  p %.2f and theta %.0f\n",p,theta);
          if (found) break;
        }

        if (!found)  {
          found = 1;   
          dedx = mean;
          err_dedx = err;
          cf = lastdedx/dedx; //always present
          printf("Using lastdedx %f  for  p %.2f and theta %.0f, firstdedx is %f\n",lastdedx,p,theta,firstdedx);
        }

        if (!found) printf("Could not find a sim dedx for p %.2f and theta %.0f\n",p,theta);

      }

    } else if (pid == 3 && itheta >= 60 && itheta <=110 && p >= 0.15 && p <= 0.45) {

      if (p==0.15 && (itheta < 70)) usethis = 0;

      if (ht < 100) usethis = 0;

      if (usethis) {

        found = 0;

        lookfortheta = 90;

        for (int j = 0; j < npi; j++) {
          if ((p == sim_pion_mtm[j]) && (sim_pion_theta[j] ==lookfortheta)) {
            found = 1;
            dedx = mean;
            err_dedx = err;
            cf = sim_pion_dedx[j]/dedx;
          }
          if (found) break;
        }

        if (!found) printf("Did not find mtm to match %.2f and theta %.0f for pid %i\n",p,theta,pid);    
      }

    }

    if (found) newt->Fill();  
       
    // read in from t
    // search arrays for matching p and pid
    // calc correction factor f
    // fill new tree
    // write out to new file
  }
   

  TFile *newfile = new TFile("dedx_correction_factors.root","RECREATE");
  newt->Write();
  newfile->Close(); 
  
}
