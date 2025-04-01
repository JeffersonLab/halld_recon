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

   cout << npy << endl;

   char filename[100];
   sprintf(filename,"surface_histo_%i_%i.png",ntheta,npy);

   c1->SaveAs(filename);

   sprintf(filename,"surface_histo_%i_%i.root",ntheta,npy);

   TFile* newfile = new TFile(filename,"RECREATE");
   histo->Write();
   newfile->Close(); 

   // histo has npy bins in y and ntheta bins in x

   // bins are ordered underflow data overflow
   // ie 2 extra bins per histo

  double thetamin = histo->GetXaxis()->GetXmin();
  double thetamax = histo->GetXaxis()->GetXmax();

  double dedxmin = histo->GetYaxis()->GetXmin();
  double dedxmax = histo->GetYaxis()->GetXmax();

  double thetastep = (thetamax - thetamin) / (float)(ntheta - 1);
  double dedxstep = (dedxmax - dedxmin) / (float)(npy - 1);

   sprintf(filename,"dedx_corrections_%i_%i.txt",ntheta,npy);
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

   /*
   int ibin=0;

   cout << histo->GetNbinsX() << "x bins"  << endl;
   cout << histo->GetNbinsY() << "y bins"  << endl;

   cout << endl;

   cout << "first bin 0\n";
   cout << histo->GetXaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetYaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetBinContent(ibin) << "\n";

   ibin=ntheta-1;
   cout << "last bin  in bottom row " << ibin << "\n";
   cout << histo->GetXaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetYaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetBinContent(ibin) << "\n";



   ibin = ntheta;
   cout << "bin ntheta " << ibin << "\n";
   cout << histo->GetXaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetYaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetBinContent(ibin) << "\n";

   ibin = ntheta+1;
   cout << "bin ntheta+1: " << ibin << "\n";
   cout << histo->GetXaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetYaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetBinContent(ibin) << "\n";

   ibin = ntheta+2;
   cout << "bin ntheta+2: " << ibin << "\n";
   cout << histo->GetXaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetYaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetBinContent(ibin) << "\n";


   ibin = ntheta+3;
   cout << "bin ntheta+3: " << ibin << "\n";
   cout << histo->GetXaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetYaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetBinContent(ibin) << "\n";


   ibin = ntheta+4;
   cout << "bin ntheta+4: " << ibin << "\n";
   cout << histo->GetXaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetYaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetBinContent(ibin) << "\n";


   //   cout << histo->GetXaxis()->GetBinLowEdge(ibin);
   // cout << histo->GetYaxis()->GetBinLowEdge(ibin);
   // cout << histo->GetBinContent(ibin);


   ibin = (ntheta+0)*npy;
   cout << "last bin " << ibin << "\n";
   cout << histo->GetXaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetYaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetBinContent(ibin) << "\n";


   ibin = (ntheta+1)*npy;
   cout << "last overflow bin " << ibin << "\n";
   cout << histo->GetXaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetYaxis()->GetBinLowEdge(ibin) << "\t" << histo->GetBinContent(ibin) << "\n";


  
*/

}
