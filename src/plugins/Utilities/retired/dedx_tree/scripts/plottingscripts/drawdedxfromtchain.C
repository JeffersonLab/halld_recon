{

  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.5);
  gStyle->SetOptFit(1);
  gStyle->SetOptStat(0);
  gStyle->SetOptLogz(1);



  int minhits= 16;

  const float dedxmax = 20;
  //  char filename[500];
  //  sprintf(filename, "/raid12/gluex/rawdata2/Run050932/dedx_tree_050932_000_015.root");
  //  sprintf(filename, "/raid12/gluex/rawdata2/Run050766/dedx_moremass_tree_050766_000_010.root");

  //  char filestub1[] = "/raid12/gluex/rawdata2/Run050932/dedx_tree2_pi_d_tree_050932";
  //  char filestub1[] = "/raid12/gluex/rawdata2/Run050932/dedx_tree_nt_fixv4_tree_050932";
  //  char filestub1[] = "/raid4/nsj/rawdata/Run050932/dedx_tree_nt_v7_s5a_tree_050932";
  //  char filestub1[] = "/raid4/nsj/rawdata/Run050932/dedx_tree_418_nt_v3_tree_050932";

  const bool saveplots=1;
  /*
  char filestub1[] = "/raid2/nsj/rawdata/Run050932/dedx_tree_4_18_1_v7_s5a2_int_tree_050932";
  char title[] = "corrected data (v7, s5a2, recon 4.18.1, truncated)";
  char filename_suffix[] = "corrected_4_18_1_v7_s5a2";
*/

  /*  
  char filestub1[] = "/raid2/nsj/rawdata/Run050932/dedx_tree_4_18_1_tree_050932";
  char title[] = "original data (recon 4.18.1, truncated)";
  char filename_suffix[] = "4_18_1";
  */
  /*
  char filestub1[] = "/raid4/nsj/sim/bggen/mcr_4120_tree_g4_50932_bggen_50932";
  char title[] = "MC data (sim 4.12.0, truncated)";
  char filename_suffix[] = "sim_4_12_0";
  */

  int nchain1=4;    //number of files to add to the tchain 
  char filestub1[] = "/raid2/nsj/rawdata/Run050932/dedx_tree_4_20_newmatrix_notrunc_050932";
  char title[] = "corrected data, without hit truncation";
  char filename_suffix[] = "4_20_050932_corr_notrunc";


  //  char filestub1[] = "/raid2/nsj/rawdata/Run050932/dedx_tree_4_20_newmatrix_050932";
  // char title[] = "corrected data, with hit truncation";
  // char filename_suffix[] = "4_20_050932_corr";

  //  char filestub1[] = "/raid2/nsj/rawdata/Run050932/dedx_tree_4_20_nocorr_050932";
  // char title[] = "original data, with hit truncation";
  // char filename_suffix[] = "4_20_050932_nocorr";

  //  char filestub1[] = "/raid2/nsj/rawdata/Run050932/dedx_tree_4_20_nocorr_notrunc_050932";
  //  char title[] = "original data, without hit truncation";
  // char filename_suffix[] = "4_20_050932_nocorr_notrunc";



  TChain chain("dedx_tree");

  for (int i=0; i<nchain1; i++) {
        chain.Add(Form("%s_%03i.root",filestub1,i));
        if (i<5) printf("Opened %s_%03i.root\n",filestub1,i);
    // chain.Add(Form("%s_%06i.root",filestub1,1000000+i));
    //if (i<5) printf("Opened %s_%06i.root\n",filestub1,1000000+i);


    if (i==5) printf("%i more files to go\n",nchain1-4);
  }


  //  TFile *file = new TFile(filename,"r");

  ULong64_t eventnumber;
  UInt_t ctracks, nhits;
  double x,y,z,r,phi,theta,p,dedx,dedx_int;
  int charge;

  //TTree *tree = (TTree*)file->Get("dedx_tree");

  chain.SetBranchAddress("eventnumber",&eventnumber);
  chain.SetBranchAddress("ctracks",&ctracks);
  chain.SetBranchAddress("ndedxhits",&nhits);
  chain.SetBranchAddress("x",&x);
  chain.SetBranchAddress("y",&y);
  chain.SetBranchAddress("z",&z);
  chain.SetBranchAddress("r",&r);
  chain.SetBranchAddress("phi",&phi);
  chain.SetBranchAddress("theta",&theta);
  chain.SetBranchAddress("charge",&charge);
  chain.SetBranchAddress("p",&p);
  chain.SetBranchAddress("dedx",&dedx);
  chain.SetBranchAddress("dedx_int",&dedx_int);

  int n = (int)chain.GetEntries(); 

  if (n<1) cout << "empty tree\n";
  if (n<1) gApplication->Terminate();

  cout << n << " entries\n";


  TH2D *dedxp = new TH2D("dedxp",Form("%s;p (GeV/c);dE/dx (keV/cm)",title),300,0,3,600,0,dedxmax);
  TH2D *dedxtheta = new TH2D("dedxtheta",Form("%s;theta (degrees);dE/dx (keV/cm)",title),180,0,180,600,0,dedxmax);

  TH2D *intdedxp = new TH2D("intdedxp",Form("%s;p (GeV/c);dE/dx from integral (keV/cm)",title),300,0,3,600,0,dedxmax);
  TH2D *intdedxtheta = new TH2D("intdedxtheta",Form("%s;theta (degrees);dE/dx from integral (keV/cm)",title),180,0,180,600,0,dedxmax);


  for (int i=0; i<n; i++) {

    chain.GetEntry(i);

    if (ctracks<2) continue;
    if (nhits<minhits) continue;
       if (charge<0) continue;
    if (z<52) continue;
    if (z>78) continue;
    if (r>1) continue;

    // find which histo to fill


    dedxp->Fill(p,dedx);
    dedxtheta->Fill(theta,dedx);
    intdedxp->Fill(p,dedx_int);
    intdedxtheta->Fill(theta,dedx_int);
  
  }

     if (saveplots) {

       /*
     TFile *f = new TFile(Form("dedx_%s.root",filename_suffix),"RECREATE");

     f->cd();

     dedxp->Write();
     dedxtheta->Write();
     intdedxp->Write();
     intdedxtheta->Write();
       */
     new TCanvas;
     gStyle->SetOptStat(0);
     dedxp->Draw("colz");
     gPad->SetLogz();
     //    dedxp->Draw("colz");
     gPad->SaveAs(Form("dedxp_%s.png",filename_suffix));
     new TCanvas;
     dedxtheta->Draw("colz");
     gPad->SaveAs(Form("dedxtheta_%s.png",filename_suffix));
     /*     new TCanvas;
     intdedxp->Draw("colz");
     gPad->SaveAs(Form("intdedxp_%s.png",filename_suffix));
     new TCanvas;
     intdedxtheta->Draw("colz");
     gPad->SaveAs(Form("intdedxtheta_%s.png",filename_suffix)); */
     }
}
