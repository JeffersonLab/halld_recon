{

    TH1F *cdc_time = (TH1F*)gDirectory->Get("/CDC_drift/cdc_time");
    if (!cdc_time) return;

    gStyle->SetOptStat(0);
    
    long long nentries = cdc_time->GetEntries();
    
    int imin = cdc_time->FindFirstBinAbove(); // first histogram bin with counts
    double xmin = cdc_time->GetBinLowEdge(imin);

    // find max content bin
    // search on from peak to find counts=0, end of fit range

    int maxcontent=0;
    int ipeak = 0;   // peak bin
    
    for (int i=imin; i<cdc_time->GetNbinsX(); i++) {
      if (cdc_time->GetBinContent(i) > maxcontent) {
        maxcontent = cdc_time->GetBinContent(i);
        ipeak = i;
      }
    }

    double xpeak = cdc_time->GetXaxis()->GetBinLowEdge(ipeak);
    
    int imax = ipeak;      // find last bin with counts 

    while (cdc_time->GetBinContent(imax) > 0 && imax<cdc_time->GetNbinsX() ) imax++;

    double xmax = cdc_time->GetBinLowEdge(imax);
    
    //starting point for background height
    double bg = 0;
    int bgrange = 4;
    for (int i=imin; i<imin+bgrange; i++) bg += cdc_time->GetBinContent(i);
    bg = bg/(double)bgrange;
    
    double fitparams[10]; 
    double startpar[10];

    //FIt function from G.Avolio et al NIM A523 (2004) 309-322
    
    TF1 *f = new TF1("f","[9] + [0] * (1 + [1]*exp(([3]-x)/[2]) + [7]*exp(([3]-x)/[8]) ) / ( (1+exp(([3]-x)/[5])) * (1+exp((x-[4])/[6])) )",xmin,xmax);

    f->SetLineWidth(1);
    f->SetLineColor(6);
    
    // set start values and limits here for all fit params except 0,3,4

    startpar[1] = 15; //amplitude of first exp contrib to peak
    startpar[7] = 3; //amplitude of second exp contrib to peak
    
    f->SetParLimits(1,0,startpar[1]*2);  //prev *10
    f->SetParLimits(7,0,startpar[7]*2);  //prev *10

    startpar[5] = 5*0.8; //slope up of t0 edge
    startpar[6] = 25*0.8; //slope down of tmax edge

    f->SetParLimits(5,0,startpar[5]*2.5);   //prev *2
    f->SetParLimits(6,0,startpar[6]*2.5);   //prev *2

    startpar[2] = 20*0.8; //first exp fall-off
    startpar[8] = 200*0.8; //second exp fall-off

    f->SetParLimits(2,0,startpar[2]*3);
    f->SetParLimits(8,startpar[2]*3,startpar[8]*3);

    for (int j=1;j<3;j++) f->SetParameter(j,startpar[j]);
    for (int j=5;j<9;j++) f->SetParameter(j,startpar[j]);

    // start values & limits for fit params 0,3,4 depend on nentries

    startpar[0] = 0.0005*nentries;  //overall scaling factor
    startpar[9] = bg; //noise background

    f->SetParLimits(0,0,startpar[0]*100);
    f->SetParameter(0,startpar[0]);

    f->SetParLimits(9,0,bg*2);
    f->SetParameter(9,startpar[9]);

    startpar[3] = xpeak;  //t0
    startpar[4] = xmax; //xpeak + 500*0.8; //tmax  //prev 550

    f->SetParLimits(3,startpar[3]-(50*0.8),startpar[3]);
    f->SetParLimits(4,startpar[3]+(500*0.8),xmax);  // min 0.5us

    f->SetParameter(3,startpar[3]);
    f->SetParameter(4,startpar[3] + (700*0.8));


    int fitstatus = cdc_time->Fit("f","RLL");

    if (fitstatus == 0 || fitstatus == 2) {   //fitstatus 0=good, 2=error matrix not posdef, fit params are correlated

      f->GetParameters(fitparams);

      double t0 = fitparams[3];
      double tmax = fitparams[4];
      double tdiff = tmax - t0;
    
      cdc_time->GetXaxis()->SetRangeUser(t0-200, tmax+300);
      
      //printf("fitstatus:%1i nentries:%5lli [0] %2.0f  [1] %2.0f  [2] %3.0f  [3] %4.0f  [4] %4.0f  [5] %4.1f  [6] %3.0f  [7] %3.1f  [8] %4.0f [9] %4.0f [tmax] %3.0f\n",fitstatus,nentries,fitparams[0],fitparams[1],fitparams[2],fitparams[3],fitparams[4],fitparams[5],fitparams[6],fitparams[7],fitparams[8],fitparams[9],tdiff);      

      cdc_time->SetTitle(Form("t_{0} %.1fns   t_{max} %.1fns   t_{diff} %.1fns",t0, tmax, tdiff));
    }
          
}
