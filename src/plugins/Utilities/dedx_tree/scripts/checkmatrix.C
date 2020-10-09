// this reads in a dedx_theta correction matrix 
// reads in the dedx_tree root trees
// uses the matrix to correct dedx and writes out binned histograms into a new root file

vector<vector<Float_t>>CDC_DEDX_CORRECTION;
Float_t cdc_min_theta, cdc_max_theta;
Float_t cdc_min_dedx, cdc_max_dedx;
Float_t cdc_theta_step, cdc_dedx_step; 
Int_t cdc_npoints_theta, cdc_npoints_dedx;


void Read_CDC_dEdx_CorrectionFile(void){

  //    FILE *dedxfile = fopen("/home/njarvis/phi/dsel/dedx_amp_corr_22July2020.txt","r");
    FILE *dedxfile = fopen("/home/njarvis/online/dedx/16hits/dedx_corrections_s5a_t_116_101.txt","r");
    fscanf(dedxfile,"%i values of theta\n",&cdc_npoints_theta);
    fscanf(dedxfile,"%f min theta\n",&cdc_min_theta);
    fscanf(dedxfile,"%f max theta\n",&cdc_max_theta);
    fscanf(dedxfile,"%f theta step\n",&cdc_theta_step);

    fscanf(dedxfile,"%i values of dedx\n",&cdc_npoints_dedx);

    fscanf(dedxfile,"%f min dedx\n",&cdc_min_dedx);
    fscanf(dedxfile,"%f max dedx\n",&cdc_max_dedx);
    fscanf(dedxfile,"%f dedx step\n",&cdc_dedx_step);
    fscanf(dedxfile,"\n");

    vector<Float_t> dedx_cf_alltheta;
    Float_t dedx_cf;

    // Store the scaling factors in vector<vector<double>>CDC_DEDX_CORRECTION;
  
    for (Int_t ii =0; ii<cdc_npoints_dedx; ii++) {
      for (Int_t jj=0; jj<cdc_npoints_theta; jj++) {
        fscanf(dedxfile,"%f\n",&dedx_cf);
        dedx_cf_alltheta.push_back(dedx_cf);
      }
      CDC_DEDX_CORRECTION.push_back(dedx_cf_alltheta);
      dedx_cf_alltheta.clear();
    }
    fclose(dedxfile);

}

Float_t Correct_CDC_dEdx(Float_t theta_deg, Float_t thisdedx){
            Int_t thetabin1, thetabin2, dedxbin1, dedxbin2;

            if (theta_deg <= cdc_min_theta) {
              thetabin1 = 0;
              thetabin2 = thetabin1;
            } else if (theta_deg >= cdc_max_theta) { 
              thetabin1 = cdc_npoints_theta - 1;
              thetabin2 = thetabin1;
            } else {
              thetabin1 = (Int_t)((theta_deg - cdc_min_theta)/cdc_theta_step);  
              thetabin2 = thetabin1 + 1;  
            }
  
            if (thisdedx <= cdc_min_dedx) {
              dedxbin1 = 0;
              dedxbin2 = dedxbin1;

            } else if (thisdedx >= cdc_max_dedx) { 
              dedxbin1 = cdc_npoints_dedx - 1;
              dedxbin2 = dedxbin1;
            } else {
              dedxbin1 = (Int_t)((thisdedx - cdc_min_dedx)/cdc_dedx_step);
              dedxbin2 = dedxbin1 + 1;
            }

           Float_t dedxcf;
  
            if ((thetabin1 == thetabin2) && (dedxbin1 == dedxbin2)) {
  
              dedxcf = CDC_DEDX_CORRECTION[dedxbin1][thetabin1];
  
  	    } else if (thetabin1 == thetabin2) {  // interp dedx only
  
              Float_t cf1 = CDC_DEDX_CORRECTION[dedxbin1][thetabin1];
              Float_t cf2 = CDC_DEDX_CORRECTION[dedxbin2][thetabin1];
  
              Float_t dedx1 = cdc_min_dedx + dedxbin1*cdc_dedx_step;
              Float_t dedx2 = dedx1 + cdc_dedx_step;
  
              dedxcf = cf1 + (thisdedx - dedx1)*(cf2 - cf1)/(dedx2-dedx1);

  	    } else if (dedxbin1 == dedxbin2) {  // interp theta only
  
              Float_t cf1 = CDC_DEDX_CORRECTION[dedxbin1][thetabin1];
              Float_t cf2 = CDC_DEDX_CORRECTION[dedxbin1][thetabin2];
  
              Float_t theta1 = cdc_min_theta + thetabin1*cdc_theta_step;
              Float_t theta2 = theta1 + cdc_theta_step;
  
              dedxcf = cf1 + (theta_deg - theta1)*(cf2 - cf1)/(theta2-theta1);

            } else {
  
              Float_t cf1 = CDC_DEDX_CORRECTION[dedxbin1][thetabin1];
              Float_t cf2 = CDC_DEDX_CORRECTION[dedxbin2][thetabin1];
  
              Float_t dedx1 = cdc_min_dedx + dedxbin1*cdc_dedx_step;
              Float_t dedx2 = dedx1 + cdc_dedx_step;
  
              Float_t cf3 = cf1 + (thisdedx - dedx1)*(cf2 - cf1)/(dedx2-dedx1);
  
              cf1 = CDC_DEDX_CORRECTION[dedxbin1][thetabin2];
              cf2 = CDC_DEDX_CORRECTION[dedxbin2][thetabin2];
  
              dedx1 = cdc_min_dedx + dedxbin1*cdc_dedx_step;
              dedx2 = dedx1 + cdc_dedx_step;
  
              Float_t cf4 = cf1 + (thisdedx - dedx1)*(cf2 - cf1)/(dedx2-dedx1);
  
              Float_t theta1 = cdc_min_theta + thetabin1*cdc_theta_step;
              Float_t theta2 = theta1 + cdc_theta_step;
  
              dedxcf = cf3 + (theta_deg - theta1)*(cf4 - cf3)/(theta2-theta1);
  
            }
  
            return dedxcf;
}



void checkmatrix(void){

Read_CDC_dEdx_CorrectionFile();

/*
  // plot display options
  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.5);
  gStyle->SetOptFit(1);
  gStyle->SetOptStat(0);
  gStyle->SetOptLogz(1);
*/

  // how many of each set of root files to read in.  All of them would be 150 & 187.
  int nchain1=150; //150;   //number of files to add to the tchain  (max 150)
  int nchain2=187; //187; //187;

  // number of hits required per track
  int minhits= 16; 

  // first part of filenames
  //  char filestub1[] = "/raid12/gluex/rawdata2/Run050932/dedx_tree_nt_tree_050932";
  //  char filestub2[] = "/raid12/gluex/rawdata2/Run050934/dedx_tree_nt_tree_050934";
  char filestub1[] = "/raid2/nsj/rawdata/Run050932/dedx_tree_4_18_1_nt_tree_050932";
  char filestub2[] = "/raid2/nsj/rawdata/Run050934/dedx_tree_4_18_1_nt_tree_50934";

  // set up the chain of the root files which will be processed in turn

  TChain chain("dedx_tree");

  for (int i=0; i<nchain1; i++) {
    chain.Add(Form("%s_%03i.root",filestub1,i));
  }

  for (int i=0; i<nchain2; i++) {
    chain.Add(Form("%s_%03i.root",filestub2,i));
  }


  // variables defined in the root trees

  ULong64_t eventnumber;   // event number unique to the run
  UInt_t ctracks, nhits;  // number of tracks in the event and hits on that track, respectively
  double x,y,z,r; // track vertex coordinates in cm 
  double phi,theta,p; //  track angles and momentum
  double dedx, dedx_int;     // dedx from pulse height and integral, respectively
  int charge;

  // set up the branch addresses to read the tree file

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
  if (n<1) return;

  cout << n << " entries\n";


  // set up the histograms to be filled later on
  const int nhistos=161;     // how many histos - one per degree in theta from 10 to 170 degrees

  const int dedxmax = 20;

  TH2D *dedxp_pos[nhistos];
  TH2D *dedxp_neg[nhistos];

  TH2D *dedxp_allpos = new TH2D("dedxp_allpos","dedxp_allpos",300,0,3,600,0,dedxmax);
  TH2D *dedxtheta_allpos = new TH2D("dedxtheta_allpos","dedxtheta_allpos",1800,0,180,600,0,dedxmax);
  TH2D *dedxp_allneg = new TH2D("dedxp_allneg","dedxp_allneg",300,0,3,600,0,dedxmax);
  TH2D *dedxtheta_allneg = new TH2D("dedxtheta_allneg","dedxtheta_allneg",1800,0,180,600,0,dedxmax);


  float thetamin=9.5;   // start with the first histogram centred on 10 degrees
  float dtheta= 1.0;     // each successive histogram centred 1 degree greater

  float thetamax[nhistos];   // max theta for each histo, used later 


  // create histograms of dedx vs p, one for each value of theta, and for each charge +/-

  for (int i=0; i<nhistos; i++) {

    dedxp_pos[i] = new TH2D(Form("dedxp_q+[%i]",i),Form("theta %.1f to %.1f degrees;p (GeV/c);dE/dx (keV/cm)",thetamin, thetamin+dtheta), 101,-0.025,5.025,200,0,25);

    dedxp_neg[i] = new TH2D(Form("dedxp_q-[%i]",i),Form("theta %.1f to %.1f degrees;p (GeV/c);dE/dx (keV/cm)",thetamin, thetamin+dtheta), 101,-0.025,5.025,200,0,25);

    thetamax[i] = thetamin + dtheta;

    thetamin += dtheta;

  }


  // loop through the entries (events) in the root file & fill the histograms

  for (int i=0; i<n; i++) {

    // read in the event data from the entry in the tree and skip the ones that I don't want.

    chain.GetEntry(i);    // read in this event

    if (ctracks<2) continue;
    if (nhits<minhits) continue;
    if (z<52) continue;
    if (z>78) continue;
    if (r>1) continue;

    // find which histo to fill

    int ihisto = 0;

    while (theta > thetamax[ihisto]) ihisto++;

    //    cout << "theta " << theta << " ihisto " << ihisto << " thetamax[ihisto] " << thetamax[ihisto] << endl;

    double new_dedx = dedx * (double)Correct_CDC_dEdx((float)theta, (float)dedx);
    // fill the histograms 

    if (ihisto < nhistos) {
      if (charge > 0) {
        dedxp_pos[ihisto]->Fill(p,new_dedx);

        dedxp_allpos->Fill(p,new_dedx);
        dedxtheta_allpos->Fill(theta,new_dedx);

      } else {
        dedxp_neg[ihisto]->Fill(p,new_dedx);

        dedxp_allneg->Fill(p,new_dedx);
        dedxtheta_allneg->Fill(theta,new_dedx);

      }
    }
  
  }


  /*

  // Draw the histograms on the screen - VERY slow

  for (int i=30; i<31; i++) {    //probably don't want 0 to nhistos unless nhistos is very small

    new TCanvas;
    dedxp_pos[i]->Draw("colz");

    new TCanvas;
    dedxp_neg[i]->Draw("colz");

  }
  */

  // create an output file & save the histograms into it

  TFile *f = new TFile(Form("matrix_applied_binned_histos_%ihits_%ifiles.root",minhits,nchain1+nchain2),"RECREATE");

  for (int i=0; i<nhistos; i++)  dedxp_pos[i]->Write();
  for (int i=0; i<nhistos; i++)  dedxp_neg[i]->Write();
  dedxp_allpos->Write();
  dedxp_allneg->Write();
  dedxtheta_allpos->Write();
  dedxtheta_allneg->Write();
  //  f->ls();   // list the file contents
  f->Close();


}
