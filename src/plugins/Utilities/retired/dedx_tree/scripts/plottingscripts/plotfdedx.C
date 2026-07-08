{

  // mtm range is 0.3 to 3

  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.4);


  const int fixrange=0; //draw invisible graph to set the axis range
 
  int plotpid = 1;

  char treefile[50];

  sprintf(treefile, "dedx_correction_factors.root");


  TFile *f = new TFile(treefile,"READ");
  if (f) cout << "Opened " << treefile << endl;
  if (!f) exit;

  TTree *t = (TTree*)gDirectory->Get("t");


  char pidname[5][30] = {"deuteron","proton","K+","pi+","proton and pi+"};

  int pid;
  float p, theta, dedx, cf;
  double nentries = t->GetEntries();

  t->SetBranchAddress("pid",&pid);
  t->SetBranchAddress("p",&p);
  t->SetBranchAddress("f",&cf);
  t->SetBranchAddress("theta",&theta);
  t->SetBranchAddress("dedx",&dedx);

  const int maxnp = 713; //
  const int maxnpi = 357; // = 2*15; // make this npid//15;   // number of momentum values
  const int nt = 86;   // number of theta values



  int np = 0 ;
  int npi = 0;
  double pd[maxnp], pth[maxnp];
  double piond[maxnpi], pionth[maxnpi];

  double pmin;

  int ngraph;



  double pstart=0;
  double pstop=1.0;
  
  if (plotpid==1) pstart = 0.35;

  if (plotpid==3) pstart = 0.15;
  if (plotpid==3) pstop=0.46;    // the fits from 0.5 + are not good

  
  for (int i=0; i<(int)nentries; i++) {

      t->GetEntry(i);

       //      if (pid != plotpid) continue;

      pstart = 0.34;
      pstop = 0.86;

      if (pid==3) pstart = 0.14;
      if (pid==3) pstop=0.46;    // the fits from 0.5 + are not good

      if (p < pstart) continue;
      if (p > pstop) continue;

      //printf("theta %.0f p %.1f p_mean %.1f \n",theta,p,mean[1]);

      ngraph = 0;
      if (pid==3) ngraph = 1;  

      //cout << " p " << p << " graph # " << ngraph << endl;
      
      if (pid==1) {
        pd[np] = dedx;
        pth[np] = theta;
        np++;
      } else if (pid == 3) {
        piond[npi] = dedx;
        pionth[npi] = theta;
        npi++;
      }


  }

  
  int col[] = {1, 2, 609, 880, 860, 66, 834, 426, 800, 95 , 804, 892, 852, 872, 38};

  

  TMultiGraph *mg = new TMultiGraph();  

  TGraph *gghost;
  if (fixrange) {
    double ymin = 0, ymax = 15.5;
    if (plotpid==3) ymin = 2;
    if (plotpid==3) ymax = 4;
    double xghost[] = {45,130};
    double yghost[] = {ymin,ymax};
    gghost = new TGraph(2,xghost,yghost);
    gghost->SetMarkerColor(0);
    mg->Add(gghost);
  }

  
  TGraph* gp = new TGraph(np,pth,pd);  
  gp->SetMarkerColor(col[0]);
  mg->Add(gp);
  
  TGraph* gpi = new TGraph(npi,pionth,piond);  
  gpi->SetMarkerColor(col[1]);
  mg->Add(gpi);
  

  TLegend *leg = new TLegend(0.91,0.1, 0.999, 0.9);
  leg->SetBorderSize(0); 
  leg->SetMargin(0);
  TLegendEntry *le[55];
  
  le[0] = leg->AddEntry(gp,"protons","");
  le[0]->SetTextColor(col[0]);

  le[1] = leg->AddEntry(gpi,"pions","");
  le[1]->SetTextColor(col[1]);

  char title[300] = "";

  sprintf(pidname[plotpid],"");
  sprintf(title,"%s dE/dx values used for correction factors;theta (degrees);dE/dx (keV/cm)", pidname[plotpid]);

  mg->SetTitle(title);

//  mg->SetTitle(Form("mean of %s dE/dx band;theta (degrees);dE/dx (keV/cm)", pidname[plotpid]));

  //if (ntracks>0) mg->SetTitle(Form("%i track events, mean of %s dE/dx band;theta (degrees);dE/dx (keV/cm)",ntracks, pidname[plotpid]));

  mg->Draw("AP");

  //  if (plotpid==1) mg->GetYaxis()->SetRangeUser(0,15.5);
  //  if (plotpid==3) mg->GetYaxis()->SetRangeUser(2,3.6);

  leg->Draw();



}
