{
  TTree* t = (TTree*)gDirectory->Get("t");

  int nreal;
  float realtheta[2000], realdedx[2000], realf[2000], realp[2000];
  int realpid[2000];

  int nfake;
  float faketheta[2000], fakededx[2000], fakef[2000], fakep[2000];
  int fakepid[2000];

  int nall;
  float alltheta[2000], alldedx[2000], allf[2000], allp[2000];
  int allpid[2000];



  int np90;
  float p90theta[2000], p90dedx[2000], p90f[2000], p90p[2000];

  int np80;
  float p80theta[2000], p80dedx[2000], p80f[2000], p80p[2000];

  int np70;
  float p70theta[2000], p70dedx[2000], p70f[2000], p70p[2000];

  int np60;
  float p60theta[2000], p60dedx[2000], p60f[2000], p60p[2000];

  int np50;
  float p50theta[2000], p50dedx[2000], p50f[2000], p50p[2000];

  int np45;
  float p45theta[2000], p45dedx[2000], p45f[2000], p45p[2000];


  int npi90;
  float pi90theta[2000], pi90dedx[2000], pi90f[2000], pi90p[2000];

  int npi80;
  float pi80theta[2000], pi80dedx[2000], pi80f[2000], pi80p[2000];

  int npi70;
  float pi70theta[2000], pi70dedx[2000], pi70f[2000], pi70p[2000];

  int npi60;
  float pi60theta[2000], pi60dedx[2000], pi60f[2000], pi60p[2000];

  int npi50;
  float pi50theta[2000], pi50dedx[2000], pi50f[2000], pi50p[2000];

  int npi45;
  float pi45theta[2000], pi45dedx[2000], pi45f[2000], pi45p[2000];





  float theta, dedx,f, p;
  int pid;

  t->SetBranchAddress("f",&f);
  t->SetBranchAddress("p",&p);
  t->SetBranchAddress("pid",&pid);
  t->SetBranchAddress("theta",&theta);
  t->SetBranchAddress("dedx",&dedx);

  double n = t->GetEntries();


  nall = (int)n;
  nreal = 0;
  nfake = 0;

  for (int i=0; i< n; i++) {
    t->GetEntry(i);

    alltheta[i] = theta;
    alldedx[i] = dedx;
    allpid[i] = pid;
    allf[i] = f;
    allp[i] = p;

    if (pid>0) {
      realtheta[nreal] = theta;
      realdedx[nreal] = dedx;
      realpid[nreal] = pid;
      realf[nreal] = f;
      realp[nreal] = p;
      nreal++;
    }

    if (pid<0) {
      faketheta[nfake] = theta;
      fakededx[nfake] = dedx;
      fakepid[nfake] = pid;
      fakef[nfake] = f;
      fakep[nfake] = p;
      nfake++;
    }

    if (pid<0) continue;

    if (pid == 1) {

      if (theta==90.0) {
        p90theta[np90] = theta;
        p90dedx[np90] = dedx;
        p90f[np90] = f;
        p90p[np90] = p;
        np90++;
      } else if (theta == 80.0) {
        p80theta[np80] = theta;
        p80dedx[np80] = dedx;
        p80f[np80] = f;
        p80p[np80] = p;
        np80++;
      } else if (theta == 70.0) {
        p70theta[np70] = theta;
        p70dedx[np70] = dedx;
        p70f[np70] = f;
        p70p[np70] = p;
        np70++;
      } else if (theta == 60.0) {
        p60theta[np60] = theta;
        p60dedx[np60] = dedx;
        p60f[np60] = f;
        p60p[np60] = p;
        np60++;
      } else if (theta == 50.0) {
        p50theta[np50] = theta;
        p50dedx[np50] = dedx;
        p50f[np50] = f;
        p50p[np50] = p;
        np50++;
      } else if (theta == 45.0) {
        p45theta[np45] = theta;
        p45dedx[np45] = dedx;
        p45f[np45] = f;
        p45p[np45] = p;
        np45++;
      }

    } else if (pid == 3) {
      if (theta==90.0) {
        pi90theta[npi90] = theta;
        pi90dedx[npi90] = dedx;
        pi90f[npi90] = f;
        pi90p[npi90] = p;
        npi90++;
      } else if (theta == 80.0) {
        pi80theta[npi80] = theta;
        pi80dedx[npi80] = dedx;
        pi80f[npi80] = f;
        pi80p[npi80] = p;
        npi80++;
      } else if (theta == 70.0) {
        pi70theta[npi70] = theta;
        pi70dedx[npi70] = dedx;
        pi70f[npi70] = f;
        pi70p[npi70] = p;
        npi70++;
      } else if (theta == 60.0) {
        pi60theta[npi60] = theta;
        pi60dedx[npi60] = dedx;
        pi60f[npi60] = f;
        pi60p[npi60] = p;
        npi60++;
      } else if (theta == 50.0) {
        pi50theta[npi50] = theta;
        pi50dedx[npi50] = dedx;
        pi50f[npi50] = f;
        pi50p[npi50] = p;
        npi50++;
      } else if (theta == 45.0) {
        pi45theta[npi45] = theta;
        pi45dedx[npi45] = dedx;
        pi45f[npi45] = f;
        pi45p[npi45] = p;
        npi45++;
      }
    }  // end if pid is 1 or 3
  }

  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.5);
  
  TGraph *greal = new TGraph(nreal,realtheta,realdedx);
  greal->SetMarkerColor(kMagenta);

  TGraph *gfake = new TGraph(nfake,faketheta,fakededx);
  gfake->SetMarkerColor(38);

  TMultiGraph *mg = new TMultiGraph();
  mg->Add(greal);
  mg->Add(gfake);
  mg->SetTitle("Real (pink) and interpolated (grey) correction factors; theta (degrees); dE/dx (keV/cm)");
  mg->Draw("AP");

  new TCanvas;
  TGraph2D *gg = new TGraph2D(nall,alltheta,alldedx,allf);
  gg->SetTitle("Correction factors; theta (degrees); dE/dx (keV/cm); correction factor"); 
 gg->Draw("APcolz");

 return;

  int col[] = {900,  880,  860,  433, 819, 801, 805, 14};

  TGraph *gp90 = new TGraph(np90,p90dedx,p90f);
  TGraph *gp80 = new TGraph(np80,p80dedx,p80f);
  TGraph *gp70 = new TGraph(np70,p70dedx,p70f);
  TGraph *gp60 = new TGraph(np60,p60dedx,p60f);
  TGraph *gp50 = new TGraph(np50,p50dedx,p50f);
  TGraph *gp45 = new TGraph(np45,p45dedx,p45f);

  TGraph *gpi90 = new TGraph(npi90,pi90dedx,pi90f);
  TGraph *gpi80 = new TGraph(npi80,pi80dedx,pi80f);
  TGraph *gpi70 = new TGraph(npi70,pi70dedx,pi70f);

  gStyle->SetMarkerSize(0.7);

  gp90->SetMarkerSize(0.7);
  gp80->SetMarkerSize(0.7);
  gp70->SetMarkerSize(0.7);
  gp60->SetMarkerSize(0.7);
  gp50->SetMarkerSize(0.7);
  gp45->SetMarkerSize(0.7);

  gpi90->SetMarkerSize(0.7);
  gpi80->SetMarkerSize(0.7);
  gpi70->SetMarkerSize(0.7);


  gp90->SetMarkerColor(col[0]);
  gp80->SetMarkerColor(col[1]);
  gp70->SetMarkerColor(col[2]);
  gp60->SetMarkerColor(col[3]);
  gp50->SetMarkerColor(col[4]);
  gp45->SetMarkerColor(col[5]);

  gpi90->SetMarkerColor(col[0]);
  gpi80->SetMarkerColor(col[1]);
  gpi70->SetMarkerColor(col[2]);



  TLegend *leg1 = new TLegend(0.92,0.3,0.99,0.9);
  leg1->AddEntry(gp90,"90","P");
  leg1->AddEntry(gp80,"80","P");
  leg1->AddEntry(gp70,"70","P");
  leg1->AddEntry(gp60,"60","P");
  leg1->AddEntry(gp50,"50","P");
  leg1->AddEntry(gp45,"45","P");
  leg1->SetBorderSize(0);

  TLegend *leg2 = new TLegend(0.92,0.3,0.99,0.9);
  leg2->AddEntry(gpi90,"90","P");
  leg2->AddEntry(gpi80,"80","P");
  leg2->AddEntry(gpi70,"70","P");
  leg2->SetBorderSize(0);





  new TCanvas;

  TMultiGraph *mgp = new TMultiGraph();
  mgp->Add(gp90);
  mgp->Add(gp80);
  mgp->Add(gp70);
  mgp->Add(gp60);
  mgp->Add(gp50);
  mgp->Add(gp45);
  mgp->SetTitle("protons;dE/dx;correction factor");
  mgp->Draw("AP");
  leg1->Draw();

  new TCanvas;

  TMultiGraph *mgpi = new TMultiGraph();
  mgpi->Add(gpi90);
  mgpi->Add(gpi80);
  mgpi->Add(gpi70);
  mgpi->SetTitle("pions;dE/dx;correction factor");

  mgpi->Draw("AP");
  leg2->Draw();


}
