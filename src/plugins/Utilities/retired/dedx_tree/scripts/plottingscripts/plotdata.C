{

  // plots peak positions from root tree in fitresults.root or fittedsimresults.root

  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.4);

  const int fixrange=0; //draw invisible graph to set the axis range
 
  int plotpid=1;      // 1=protons, 3=pions
 
  const int sim=1;

  char treefile[50];
  sprintf(treefile, "fitresults.root");
  if (sim) sprintf(treefile, "fittedsimresults.root");

  TFile *f = new TFile(treefile,"READ");
  if (f) cout << "Opened " << treefile << endl;
  if (!f) exit;

  TTree *t = (TTree*)gDirectory->Get("t");


  char pidname[5][30] = {"deuteron","proton","K+","pi+","proton and pi+"};

  int pid;
  float p, theta;
  double nentries = t->GetEntries();

  t->SetBranchAddress("pid",&pid);
  t->SetBranchAddress("p",&p);
  t->SetBranchAddress("theta",&theta);


  //  t->SetBranchAddress("n",nentries);

  // use arrays so that all pids can be included more easily

  // 0: d   1: p    2: K    3: pi

  double ht, mean, sigma;
  double err_c, err_m, err_s;
  
  t->SetBranchAddress("ht",&ht);
  t->SetBranchAddress("mean",&mean);
  t->SetBranchAddress("sigma",&sigma);
  t->SetBranchAddress("err_h",&err_c);
  t->SetBranchAddress("err_m",&err_m);
  t->SetBranchAddress("err_s",&err_s);

  const int np = 55;   // number of momentum values
  const int nt = 161;   // number of theta values


  int n[np]={0};
  double d[np][nt], th[np][nt];
  double pmin;
  double pset[np];
  int ngraph;



  double pstart=0;
  double pstop=5.0;
  
  if (plotpid==1) pstart = 0.3;

  //  if (plotpid==3) pstart = 0.15;
  if (plotpid==3) pstop=0.46;    // the fits from 0.5 + are not good

  double minht;

  if (plotpid==1) minht=30;  //30
  if (plotpid==1 && sim==1) minht = 25;
 
   if (plotpid==3) minht=100;

  for (int i=0; i<(int)nentries; i++) {

      t->GetEntry(i);

      if (pid != plotpid) continue;
      if (p < pstart) continue;
      if (p > pstop) continue;
      if (ht <= minht) continue;

      //printf("theta %.0f p %.1f p_mean %.1f \n",theta,p,mean[1]);

      ngraph = -1;

      for (int j=0; j<np; j++) {

        pmin = pstart + j*0.05;
        if (fabs(p-pmin) < 0.01) ngraph = j; 
        if (ngraph >= 0) break;

      }
  
      
      int npoints = n[ngraph];

      d[ngraph][npoints] = mean;
      th[ngraph][npoints] = theta;

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
  char simlabel[8];
  char peaktype[8];

  if (sim) sprintf(simlabel,"bggen ");

  if (plotpid==1) sprintf(peaktype,"mean");
  if (plotpid==3) sprintf(peaktype,"mode");

  sprintf(title,"%s of %s%s dE/dx;theta (degrees);dE/dx (keV/cm)", peaktype, simlabel, pidname[plotpid]);

  mg->SetTitle(title);


  mg->Draw("AP");

  //  if (plotpid==1) mg->GetYaxis()->SetRangeUser(0,15.5);
  //  if (plotpid==3) mg->GetYaxis()->SetRangeUser(2,3.6);

  leg->Draw();

}
