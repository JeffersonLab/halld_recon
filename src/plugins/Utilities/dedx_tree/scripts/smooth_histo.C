{

  TTree* t = (TTree*)gDirectory->Get("t");

  int nentries = (int)t->GetEntries();
  float theta, dedx, f, p;
  int pid;
  t->SetBranchAddress("dedx",&dedx);
  t->SetBranchAddress("theta",&theta);
  t->SetBranchAddress("f",&f);
  t->SetBranchAddress("pid",&pid);
  t->SetBranchAddress("p",&p);

  const int THETA_MIN = 21;
  const int THETA_MAX = 136;
  int ntheta = 1 + THETA_MAX - THETA_MIN ;

  // theta 22 to 135, extended to 21 to 136
  // dedx 2.30382 to 15.0024, extended to 20
  // histo dedx 2.30740 to 20


   TCanvas *c = new TCanvas("c","Graph2D example",0,0,800,600);
   Double_t x, y, z;
   //   Int_t np = 200;

   const int npy = 1+ 100;

   TGraph2D *gtot = new TGraph2D();
   gtot->SetNpx(ntheta);
   gtot->SetNpy(npy);

   TGraph2D *gprotons = new TGraph2D();
   gprotons->SetNpx(ntheta);
   gprotons->SetNpy(npy);

   TGraph2D *gpions = new TGraph2D();
   gpions->SetNpx(ntheta);
   gpions->SetNpy(npy);

   TGraph2D *gdeuterons = new TGraph2D();
   gdeuterons->SetNpx(ntheta);
   gdeuterons->SetNpy(npy);

   TGraph2D *gcombo = new TGraph2D();
   gcombo->SetNpx(ntheta);
   gcombo->SetNpy(npy);

   TGraph2D *gfakes = new TGraph2D();
   gfakes->SetNpx(ntheta);
   gfakes->SetNpy(npy);

   int np=0;
   int nf=0;
   int nd=0;
   int npi=0;
   int nc=0;

   int nt=0;  // use for drawing more than one particle, just draw over one in a diff colour.
   // to get the axis range right

   float dcut = 0.46;

   for (Int_t N=0; N<nentries; N++) {
      
      t->GetEntry(N);
      x = theta;  //2*P*(r->Rndm(N))-P;
      y = dedx; //2*P*(r->Rndm(N))-P;
      z = f; //(sin(x)/x)*(sin(y)/y)+0.2;
      if (pid==-1) gfakes->SetPoint(nf,x,y,z);
      if (pid==0 && p < dcut) gdeuterons->SetPoint(nd,x,y,z);
      if (pid==1) gprotons->SetPoint(np,x,y,z);
      if (pid==3) gpions->SetPoint(npi,x,y,z);
      if (pid==4) gcombo->SetPoint(nc,x,y,z);

      if (fabs(pid)!=0 && pid!=4)  gtot->SetPoint(nt,x,y,z);
      if (fabs(pid) != 0 && pid !=4) nt++;

      // if (pid==0 && p<dcut)  gtot->SetPoint(nt,x,y,z);
      // if (pid==0 && p<dcut)  nt++;

      if (pid==-1) nf++;
      if (pid==0 && p<dcut) nd++;
      if (pid==1) np++;
      if (pid==3) npi++;
      if (pid==4) nc++;

   }
   gStyle->SetPalette(1);
   gtot->SetTitle("dE/dx correction factor;theta;dedx;f");
   gtot->SetMarkerColor(0);
   gtot->Draw("surf1");

   TH2D* histo = gtot->GetHistogram();

   new TCanvas;
   histo->Draw("surf1");
   gPad->SaveAs("v6_original.png");

   /*
    TH2D *histo3 = (TH2D*)histo->Clone(); 
   histo3->Smooth(1,"3a");
   
   new TCanvas;
   histo3->Draw("surf1");
   gPad->SaveAs("v6_smoothed3a.png");

   TH2D *histo5 = (TH2D*)histo->Clone(); 
   histo5->Smooth(1,"5a");

   new TCanvas;
   histo5->Draw("surf1");
   gPad->SaveAs("v6_smoothed5a.png");

   TH2D *histo5b = (TH2D*)histo->Clone(); 
   histo5b->Smooth(1,"5b");

   new TCanvas;
   histo5b->Draw("surf1");
   gPad->SaveAs("v6_smoothed5b.png");
   */

   //cout << npy << endl;

   char smoothopt[3];

   sprintf(smoothopt,"5a");

   histo->Smooth(1,smoothopt);
   histo->Smooth(1,smoothopt);

   sprintf(smoothopt,"5a_twice");

   char filename[100];
   sprintf(filename,"surface_histo_s%s_%i_%i.png",smoothopt,ntheta,npy);

   gPad->SaveAs(filename);

   sprintf(filename,"surface_histo_s%s_%i_%i.root",smoothopt,ntheta,npy);


   //




   //
   TFile* newfile = new TFile(filename,"RECREATE");
   histo->Write();
   newfile->Close(); 

   new TCanvas;
   histo->Draw("colz");
   gPad->SaveAs(Form("surface_histo_flat_s%s_%i_%i.png",smoothopt,ntheta,npy));


   // histo has npy bins in y and ntheta bins in x

   // bins are ordered underflow data overflow
   // ie 2 extra bins per histo

  double thetamin = histo->GetXaxis()->GetXmin();
  double thetamax = histo->GetXaxis()->GetXmax();

  double dedxmin = histo->GetYaxis()->GetXmin();
  double dedxmax = histo->GetYaxis()->GetXmax();

  double thetastep = (thetamax - thetamin) / (float)(ntheta - 1);
  double dedxstep = (dedxmax - dedxmin) / (float)(npy - 1);

  sprintf(filename,"dedx_corrections_s%s_%i_%i.txt",smoothopt,ntheta,npy);

   cout << "Writing text file " << filename << endl;

   FILE *outfile = fopen(filename,"w");
   fprintf(outfile,"%i values of theta\n",ntheta); //histo->GetNbinsX());
   fprintf(outfile,"%f min theta\n",thetamin);
   fprintf(outfile,"%f max theta\n",thetamax);
   fprintf(outfile,"%f theta step\n",thetastep);

   fprintf(outfile,"%i values of dedx\n",npy);
   fprintf(outfile,"%f min dedx\n",dedxmin);
   fprintf(outfile,"%f max dedx\n",dedxmax);
   fprintf(outfile,"%f dedx step\n",dedxstep);

   fprintf(outfile,"\n");


   int totalbins = (ntheta+2)*(npy+2);
   int ix = 0;

   for (int ibin = ntheta+2; ibin<totalbins-(ntheta+2); ibin++) {
     if (ix>0 && ix<=ntheta) fprintf(outfile,"%f\n",histo->GetBinContent(ibin));
     ix++;
     if (ix== ntheta+2) ix=0;
   }
   
   fclose(outfile);
   

}
