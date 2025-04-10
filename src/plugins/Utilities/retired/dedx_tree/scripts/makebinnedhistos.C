
{

  // plot display options
  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.5);
  gStyle->SetOptFit(1);
  gStyle->SetOptStat(0);
  gStyle->SetOptLogz(1);

  // how many of each set of root files to read in.  All of them would be 150 & 187.
  int nchain1=1; //150;   //number of files to add to the tchain  (max 150)
  int nchain2=0; //187;

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

  TH2D *dedxp_pos[nhistos];
  TH2D *dedxp_neg[nhistos];

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


    // fill the histograms 

    if (ihisto < nhistos) {
      if (charge > 0) {
        dedxp_pos[ihisto]->Fill(p,dedx);
      } else {
        dedxp_neg[ihisto]->Fill(p,dedx);
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

  TFile *f = new TFile(Form("binned_histos_%ihits_%ifiles.root",minhits,nchain1+nchain2),"RECREATE");

  for (int i=0; i<nhistos; i++)  dedxp_pos[i]->Write();
  for (int i=0; i<nhistos; i++)  dedxp_neg[i]->Write();
  //  f->ls();   // list the file contents
  f->Close();


}
