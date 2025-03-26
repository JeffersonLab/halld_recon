
// draws de/dx cut functions on top of the histogram specified from the file specified

{
  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.5);
  gStyle->SetOptFit(1);
  gStyle->SetOptStat(0);
  gStyle->SetOptLogz(1);
  

  int ihisto = 30;    // draw histo # 30   (for 40 degrees, theta=ihisto+10)

  char histofile[300];

  sprintf(histofile, "/home/njarvis/online/dedx/binned_histos_20hits_150files.root");

  TFile *f = new TFile(histofile,"READ");

  if (!f) printf("Cannot find %s\n",histofile);
  if (!f) gApplication->Terminate();


  // functional forms for cut functions

  TF1 *fp = new TF1("fp","exp([0]*(x + [1])) + [2]", 0.0, 4.0);   // between protons and deuterons
  TF1 *fk = new TF1("fk","exp([0]*(x + [1])) + [2]", 0.0, 0.85);  // between protons and kaons
  TF1 *fpi = new TF1("fpi","exp([0]*(x + [1])) + [2]", 0.0, 0.425);  // below pions


  // plots are symmetric about 90 degrees (histo 80)

  cout << "Difference between selected angle and 90 degrees is " << fabs(ihisto-80) << endl;

    if (fabs(ihisto-80) < 10) {     // ihisto 71-89
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
        fp->SetParameters(-4.6, -0.98, 5.2);  // 60 degr
        fk->SetParameters(-4.6,-0.7,2.3); // good at 60 degr
        fpi->SetParameters(-6.5, -0.43, 2.75);

    } else if (fabs(ihisto-80) < 50) {  // ihisto 31-40, 120-129
        fp->SetParameters(-4.6, -0.98, 5.2);  // 50 degr
        fk->SetParameters(-4.6,-0.69,2.7); // good at 50 degr
        fpi->SetParameters(-6.5, -0.43, 2.75);

    } else { //if (fabs(ihisto-80) < 55) {    // ihisto < 31, > 129
        fp->SetParameters(-4.6, -0.98, 5.2);  // 45 degr same as 60
        fk->SetParameters(-5.0, -0.7, 2.9);
        fpi->SetParameters(-6.5, -0.43, 2.75);
    }

    
  TH2D* hpos = (TH2D*)gDirectory->Get(Form("dedxp_q+[%i]",ihisto));

  hpos->GetYaxis()->SetRangeUser(0,15);
  hpos->GetXaxis()->SetRangeUser(0,3);

  new TCanvas;
  hpos->Draw("colz");

  fp->Draw("same");
  fk->Draw("same");
  fpi->Draw("same");
  

}


 
