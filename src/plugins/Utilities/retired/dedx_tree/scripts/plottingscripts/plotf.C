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

  const int np = 15;   // number of momentum values
  const int nt = 86;   // number of theta values


  int n[np]={0};
  double d[np][nt], th[np][nt];
  double pmin;
  double pset[np];
  int ngraph;



  double pstart=0;
  double pstop=1.0;
  
  if (plotpid==1) pstart = 0.35;

  if (plotpid==3) pstart = 0.15;
  if (plotpid==3) pstop=0.46;    // the fits from 0.5 + are not good

  
  for (int i=0; i<(int)nentries; i++) {

      t->GetEntry(i);

      if (pid != plotpid) continue;
      //      if (p < pstart) continue;
      //  if (p > pstop) continue;

      //printf("theta %.0f p %.1f p_mean %.1f \n",theta,p,mean[1]);

      ngraph = -1;

      for (int j=0; j<np; j++) {

        pmin = pstart + j*0.05;
        if (fabs(p-pmin) < 0.01) ngraph = j; 
        if (ngraph >= 0) break;

      }
  
      //cout << " p " << p << " graph # " << ngraph << endl;
      
      int npoints = n[ngraph];

      d[ngraph][npoints] = cf; //dedx;
      th[ngraph][npoints] = theta;


      //cout << "ngraph " << n[ngraph] << " dedx " << d[ngraph][n[ngraph]] << " theta " << th[ngraph][n[ngraph]]<< endl;


      n[ngraph]++;

  }

  
  int col[] = {1, 2, 609, 880, 860, 66, 834, 426, 800, 95 , 804, 892, 852, 872, 38};

  TGraph *g[np];

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



  TLegend *leg = new TLegend(0.91,0.1, 0.999, 0.9);
  leg->SetBorderSize(0); 
  leg->SetMargin(0);
  TLegendEntry *le[55];
  
  le[0] = leg->AddEntry(g[0],"GeV/c","");
  le[0]->SetTextColor(1);


  for (int i=0; i<15; i++) {


    if (n[i]>0) { 

      g[i] = new TGraph(n[i],th[i],d[i]);  
      g[i]->SetMarkerColor(col[i]);
      mg->Add(g[i]);
      float thisp = pstart + i*0.05;
      le[i+1] = leg->AddEntry(g[i],Form("%.2f",thisp),"");
      le[i+1]->SetTextColor(col[i]);

    }
  
  }  

  char title[300] = "";

  sprintf(title,"%s dE/dx correction factors;theta (degrees);correction factor", pidname[plotpid]);

  mg->SetTitle(title);

//  mg->SetTitle(Form("mean of %s dE/dx band;theta (degrees);dE/dx (keV/cm)", pidname[plotpid]));

  //if (ntracks>0) mg->SetTitle(Form("%i track events, mean of %s dE/dx band;theta (degrees);dE/dx (keV/cm)",ntracks, pidname[plotpid]));

  mg->Draw("AP");

  //  if (plotpid==1) mg->GetYaxis()->SetRangeUser(0,15.5);
  //  if (plotpid==3) mg->GetYaxis()->SetRangeUser(2,3.6);

  leg->Draw();




/*
  gStyle->SetMarkerSize(0.2);
  gStyle->SetMarkerStyle(20);

  t->Draw("p_mean:theta>>h(150,0,3,1700,0,17)","!p_fitstat && p<0.31");

  h->Draw();
  */


}
