
void extendscalefactors(){

  TFile *cffile = new TFile("dedx_correction_factors.root","READ");

  TTree *t = (TTree*)gDirectory->Get("t");
  int nentries = (int)t->GetEntries();

  int pid;
  // 0: d   1: p    2: K    3: pi    4: combined

  const int THETA_MIN = 45;
  const int THETA_MAX = 130;
  int ntheta = 1+ THETA_MAX - THETA_MIN;

  const float DEDX_MIN=2.0;
  const float DEDX_MAX=20.0;

  float p, theta;
  float dedx, err_dedx, cf;

  t->SetBranchAddress("theta",&theta);
  t->SetBranchAddress("dedx",&dedx);
  t->SetBranchAddress("err_dedx",&err_dedx);
  t->SetBranchAddress("f",&cf);
  t->SetBranchAddress("p",&p);
  t->SetBranchAddress("pid",&pid);


  TTree *newt = new TTree("t","extended range of dedx correction factors");

  newt->Branch("theta",&theta,"theta/F");
  newt->Branch("dedx",&dedx,"dedx/F");
  newt->Branch("err_dedx",&err_dedx,"err_dedx/F");
  newt->Branch("f",&cf,"f/F");
  // extras in case of problems
  newt->Branch("p",&p,"p/F");
  newt->Branch("pid",&pid,"pid/I");


  // copy existing entries *****for pi and p only ****

  for (int i=0; i<nentries; i++) {

    t->GetEntry(i);
    if (pid==1||pid==3) newt->Fill();
  }


  // the pions only run from 65 to 135
  // need to find the first and last pion cf for each p and copy it to every theta 45 - 64,  115-135
  // then fill in the dedx-min edge at 2. 


  // low dedx boundary
  // scan up the column of pion dedx vs theta to find the cf for the lowest value of dedx 
  // ?  if the lowest cf > 1.04 or has p > 2.6, leave a gap.
  // ?  The lowest cf band has a max at 1.04 w theta



  

  float lowestdedx[THETA_MAX+1];
  float cf_lowestdedx[THETA_MAX+1] = {0};

  for (int i=THETA_MIN; i<= THETA_MAX; i++) lowestdedx[i] = 100.0;

  int itheta;

  for (int i=0; i<nentries; i++) {
    t->GetEntry(i);

    if (pid != 3) continue;
    if (dedx == 0) continue;
    // if (cf>1.04) continue;  

    itheta = int(theta);
    //    cout << dedx << " " << theta << " " << lowestdedx[itheta] << endl;
    if (dedx < lowestdedx[itheta]) {
      lowestdedx[itheta] = dedx;
      cf_lowestdedx[itheta] = cf;
    }
  }


  int firsttheta=0;
  int lasttheta=0;


  for (int itheta=THETA_MIN; itheta<=THETA_MAX; itheta++)  {

    // cout << " min dedx at theta " << itheta << " degrees is " << lowestdedx[itheta] << endl;

    if (cf_lowestdedx[itheta] > 0) {

      if (firsttheta==0) firsttheta = itheta;
      lasttheta = itheta;

      theta = float(itheta);
      dedx = DEDX_MIN;
      cf = cf_lowestdedx[itheta];
      pid = -1;
      p = -1;
      newt->Fill();
      cout << "filling in cf " << cf << " for dedx " << dedx  << " theta " << theta << endl;
    }
  }    


  theta = (float)THETA_MIN;
  cf = cf_lowestdedx[firsttheta];
  newt->Fill();


  theta = (float)THETA_MAX;
  cf = cf_lowestdedx[lasttheta];
  newt->Fill();

  

  // add high dedx boundary

  // make array of cf and dedx at each theta, then fit and extrapolate

  float allf[THETA_MAX+1][20];
  float alld[THETA_MAX+1][20];
  int alln[THETA_MAX+1] = {0};

  float pionf[THETA_MAX+1][20];
  float piond[THETA_MAX+1][20];
  int pionn[THETA_MAX+1] = {0};

  
  for (int i=0; i<nentries; i++) {
    t->GetEntry(i);

    int it = (int)theta;

    if (pid==1) {
      alld[it][alln[it]] = dedx;
      allf[it][alln[it]] = cf;
      alln[it]++;
    } else if (pid==3) {
      piond[it][pionn[it]] = dedx;
      pionf[it][pionn[it]] = cf;
      pionn[it]++;
    }
  }
  
  TGraph *g;
  TF1 *f = new TF1("f","pol1",0,16);
  double fitstat;

  float lastgoodtheta, lastcfat20, lastcfat17, lastcfat15, lastcfat125,lastcfat4;   // vals from highest theta before data run out
  float lastgoodpitheta,firstcfat2,lastcfat2;


  int counter = 0;

  for (int it = THETA_MIN; it<=THETA_MAX; it++) {

    if (alln[it]>1) {

      g = new TGraph(alln[it],alld[it],allf[it]);
    
       //    if (counter==0) g->Draw("AP");

      fitstat = g->Fit("f","Q");

      if (!fitstat) {
    
        // evaluate the func for dedx=15, 20, and 12 if it is within the dip.

        pid = -1;
        p=0;
        theta = (float)it;
        lastgoodtheta = theta;

        dedx = 20.0;
        cf = f->Eval(dedx);
        lastcfat20 = cf;
        newt->Fill();
    
        dedx = 17.5;
        cf = f->Eval(dedx);
        lastcfat17 = cf;
        newt->Fill();
    
        dedx = 15.0;
        cf = f->Eval(dedx);
        lastcfat15 = cf;
        newt->Fill();

        dedx = 12.5;  //new
        cf = f->Eval(dedx);
        lastcfat125 = cf;
        newt->Fill();


        dedx = 4.0;  //new
        cf = f->Eval(dedx);
        lastcfat4 = cf;
        newt->Fill();


	//    if (theta<81) continue;
	//if (theta>101) continue;

        if (theta >70 && theta < 112) {
          dedx = 10.0;
          cf = f->Eval(dedx);
          newt->Fill();
        }


        if (theta >80 && theta < 100) {
          dedx = 9.0;
          cf = f->Eval(dedx);
          newt->Fill();
        }

      }  // if !fitstat

    } 

    /*
    if (pionn[it]>1) {

      cout << "pion n " << pionn[it] << endl;
      g = new TGraph(pionn[it],piond[it],pionf[it]);
    
       //    if (counter==0) g->Draw("AP");

      fitstat = g->Fit("f","0");

      if (!fitstat) {
    
        // evaluate the func for dedx=2 

        pid = -1;
        p=0;
        theta = (float)it;
        lastgoodpitheta = theta;

        dedx = 2.0;
        cf = f->Eval(dedx);
        if (firstcfat2==0) firstcfat2=cf;
        lastcfat2 = cf;
        newt->Fill();
    
      }

    } // if alln[it]>0 elseif pin[it]>0
    */
    counter++;
    if (counter==5) counter = 0;

  }


  if (lastgoodtheta < THETA_MAX) {

    theta = THETA_MAX;
    dedx = 20.0;
    cf = lastcfat20;
    newt->Fill();

    dedx = 17.5;
    cf = lastcfat15;
    newt->Fill();

    dedx =15.0;
    cf = lastcfat15;
    newt->Fill();

    dedx =12.5;
    cf = lastcfat125;
    newt->Fill();

    dedx =4.0;
    cf = lastcfat4;
    newt->Fill();


  }

  /*
  theta = THETA_MIN;
  dedx=2.0;
  cf = firstcfat2;
  newt->Fill();  

  theta = THETA_MAX;
  cf = lastcfat2;
  newt->Fill();  
  */

  //sides - first try without these.  

  // how many mtm values are there?  

  //  0.1 to 2.5 in steps of 0.05

  float f1[5][280]={0}, t1[5][280]={90}, d1[5][280] = {0};
  float f2[5][280]={0}, t2[5][280]={90}, d2[5][280] = {0};

  for (int j=0; j<5; j++) {
    for (int i=0; i<280; i++) {
      t1[j][i] = 90;
      t2[j][i] = 90;
    }
  }

  int ip;  //which array element to use for the p

  // find cf at first and last theta, for each mtm, protons & pions only

  for (int i=0; i<nentries; i++) {
    t->GetEntry(i);

    ip = (int)(p*100);    // so 0.35->35
    
    if (theta < t1[pid][ip]) {
        f1[pid][ip] = cf;
        d1[pid][ip] = dedx;
        t1[pid][ip] = theta;
    }

    if (theta > t2[pid][ip]) {
        f2[pid][ip] = cf;
        d2[pid][ip] = dedx;
        t2[pid][ip] = theta;
    } 
  }
    
  // replicate them

  for (int ip=0; ip<280; ip++) {

    for (int thispid = 0; thispid<5; thispid++) {

      if (thispid == 2) continue;   // no k+
      if (thispid == 0) continue;  // no d
      if (d1[thispid][ip] == 0) continue;
      if (t1[thispid][ip] > 70) continue; // don't think there are any; too big to be useful

      // copy first value to theta_min degrees
      pid = -1;
      p = 0;


      if (t1[thispid][ip] > THETA_MIN) {

	//	if (thispid==3 && t1[thispid][ip]<=65) {
	if (t1[thispid][ip]<=65) {

          theta = THETA_MIN;
          cf = f1[thispid][ip];
          dedx = d1[thispid][ip];
          newt->Fill();

          printf("for pid %i added cf %.1f from dedx %.1f   p %.1f and theta %.1f at theta %.0f deg\n",thispid, cf, dedx, 0.01*ip, t1[thispid][ip],theta); 
	  // cout << ip << " added " << dedx << " from " << t1[thispid][ip] << " at " << THETA_MIN << "deg   ";
        }

      }

      if (t2[thispid][ip] < THETA_MAX) {

        // if the last theta is < 100, duplicate cf from the first theta as they are mostly symmetrical
	//          cout << " last theta found is " << t2[thispid][ip] ;

	if ((t2[thispid][ip] >= 105 && thispid==3) || (t2[thispid][ip] >= 115 && thispid==1)) {
            theta = THETA_MAX;
            cf = f2[thispid][ip];
            dedx = d2[thispid][ip];

            newt->Fill();    

          printf("\nfor pid %i added cf %.1f from dedx %.1f   p %.1f and theta %.1f at theta %.0f deg\n",thispid, cf, dedx, 0.01*ip, t2[thispid][ip],theta); 
	  //            cout << " added " << dedx << " from " << t2[thispid][ip] << " degrees at " << THETA_MAX << "deg\n";

          }  else {

	    /*
            theta = THETA_MAX;
            cf = f1[thispid][ip];
            dedx = d1[thispid][ip];

            newt->Fill();    

          printf("for pid %i added cf %.1f from dedx %.1f   p %.1f and theta %.1f at theta %.0f deg\n",thispid, cf, dedx, 0.01*ip, t1[thispid][ip],theta); 
	    */

          }

	}  // end     if (t2[thispid][ip] < THETA_MAX) {

    }  // end for (int thispid = 0; thispid<5; thispid++) {

  } // end for (int ip=0; ip<280; ip++) {
  

  TFile *newfile = new TFile("extended_dedx_correction_factors.root","RECREATE");
  newt->Write();
  newfile->Close(); 

}
