{

  const float dpmax = 0; //0.5; // exclude d with mtm > dpmax

  TTree* t = (TTree*)gDirectory->Get("t");

  int nentries = (int)t->GetEntries();
  float theta, dedx, f, p;
  int pid;
  t->SetBranchAddress("dedx",&dedx);
  t->SetBranchAddress("theta",&theta);
  t->SetBranchAddress("f",&f);
  t->SetBranchAddress("pid",&pid);
  t->SetBranchAddress("p",&p);


  // theta 21 to 137
  // dedx 2.30382 to 15.0024

   TCanvas *c = new TCanvas("c","Graph2D example",0,0,800,600);
   Double_t x, y, z;
   //   Int_t np = 200;
   TGraph2D *gtot = new TGraph2D();
   gtot->SetNpx(116);
   gtot->SetNpy(254);

   TGraph2D *gprotons = new TGraph2D();
   gprotons->SetNpx(116);
   gprotons->SetNpy(254);

   TGraph2D *gpions = new TGraph2D();
   gpions->SetNpx(116);
   gpions->SetNpy(254);

   TGraph2D *gdeuterons = new TGraph2D();
   gdeuterons->SetNpx(116);
   gdeuterons->SetNpy(254);

   TGraph2D *gcombo = new TGraph2D();
   gcombo->SetNpx(116);
   gcombo->SetNpy(254);

   TGraph2D *gfakes = new TGraph2D();
   gfakes->SetNpx(116);
   gfakes->SetNpy(254);

   int np=0;
   int nf=0;
   int nd=0;
   int npi=0;
   int nc=0;

   int nt=0;  // use for drawing more than one particle, just draw over one in a diff colour.
   // to get the axis range right

   for (Int_t N=0; N<nentries; N++) {
      
      t->GetEntry(N);

      if (dedx>10) continue;

      x = theta;  //2*P*(r->Rndm(N))-P;
      y = dedx; //2*P*(r->Rndm(N))-P;
      z = f; //(sin(x)/x)*(sin(y)/y)+0.2;
      if (pid==-1) gfakes->SetPoint(nf,x,y,z);
      if (pid==0 && p < dpmax) gdeuterons->SetPoint(nd,x,y,z);
      if (pid==1) gprotons->SetPoint(np,x,y,z);
      if (pid==3) gpions->SetPoint(npi,x,y,z);
      if (pid==4) gcombo->SetPoint(nc,x,y,z);

      if (fabs(pid)!=0 && pid!=4)  gtot->SetPoint(nt,x,y,z);
      if (fabs(pid) != 0 && pid !=4) nt++;

      if (pid==0 && p<dpmax)  gtot->SetPoint(nt,x,y,z);
      if (pid==0 && p<dpmax)  nt++;

      if (pid==-1) nf++;
      if (pid==0 && p<dpmax) nd++;
      if (pid==1) np++;
      if (pid==3) npi++;
      if (pid==4) nc++;

   }
   gStyle->SetPalette(1);
   gtot->SetTitle("dE/dx correction factor;theta;dedx;f");
   gtot->SetMarkerColor(0);
   gtot->Draw("surf1");

   new TCanvas;

   //   gtot->GetYaxis()->SetRangeUser(2,10);

   gtot->Draw("surf1");

   gprotons->SetMarkerStyle(20);
   gpions->SetMarkerStyle(20);
   gcombo->SetMarkerStyle(20);
   gdeuterons->SetMarkerStyle(20);
   gfakes->SetMarkerStyle(20);

   gprotons->SetMarkerSize(0.4);
   gprotons->SetMarkerColor(kRed);
   gprotons->Draw("psame");

   gpions->SetMarkerColor(kBlue);
   gpions->SetMarkerSize(0.4);
   gpions->Draw("psame");

   gcombo->SetMarkerColor(kMagenta);
   gcombo->SetMarkerSize(0.4);
   //   gcombo->Draw("psame");

   gdeuterons->SetMarkerColor(906);
   gdeuterons->SetMarkerSize(0.5);
    gdeuterons->Draw("psame");

   gfakes->SetMarkerColor(38);
   gfakes->SetMarkerSize(0.5);
   gfakes->Draw("psame");

   //   g

    //   return c;

   new TCanvas;
   gtot->Draw("p");
   gprotons->Draw("psame");
   gpions->Draw("psame");
   gcombo->Draw("psame");
  gdeuterons->Draw("psame");
   gfakes->Draw("psame");
   /*
   TH2D *h = dt->GetHistogram();

   new TCanvas;
   h->Draw("surf");
   */
}
