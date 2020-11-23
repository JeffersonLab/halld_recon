// required library change
// params a1,a2, b1,b2, c1, c2, c3 used for both + and - delta
// set up the fit so that a1, b1, c1 are the same for + and -
// a2 & b2 -ve = -1 * a2 & b2 +ve
// a3 and b3 must be 0 - only used for -ve delta. 

// I want to use the same function for both + & - 
// Set up the fit function to use one set of parameters (long side/first row/+)
// For delta<0 mutliply a2, b2 * -1




// void differentfit(int dofit=0, int savefit=0)

double long_drift_func[3][3];
double short_drift_func[3][3];
double magnet_correction[2][2];

vector<double> cdc_drift_table;
double Bz_avg;

// Set values for the region cut
//float deltaMin = -0.15, deltaMax = 0.15, tMin = 300, tMax = 1200;
float deltaMin = -0.175, deltaMax = 0.175, tMin = 300, tMax = 1200;

unsigned int Locate(vector<double>&xx, double x){
    int n=xx.size();
    if (x==xx[0]) return 0;
    else if (x==xx[n-1]) return n-2;

    int jl=-1;
    int ju=n;
    int ascnd=(xx[n-1]>=xx[0]);
    while(ju-jl>1){
        int jm=(ju+jl)>>1;
        if ( (x>=xx[jm])==ascnd)
            jl=jm;
        else
            ju=jm;
    } 
    return jl;
}

Double_t TimeToDistance( Double_t *x, Double_t *par){
   Double_t d=0.0;
   double delta = x[1]; // yAxis
   double t = x[0]; // xAxis

   // Cut out region in  fit.  *** these 2 lines create the half trapezium boundaries
   if (delta > deltaMax || delta < deltaMin) return 0.0;
   if (delta < (((deltaMax - deltaMin) / (tMax - tMin))*(t - tMin) + deltaMin)) return 0.0;
   //   if (t > 950) return 0.0;
   if (t > 850) return 0.0;

   // Variables to store values for time-to-distance functions for delta=0
   // and delta!=0
   double f_0=0.;
   double f_delta=0.;

   if (t > 0){

     // a3, b3 are not used.

         double a1=par[0];
         double a2=par[1];
         double a3=par[2];
         double b1=par[3];
         double b2=par[4];
         double b3=par[5];
         double c1=par[6];
         double c2=par[7];
         double c3=par[8];
 
         if (delta <=0) a2=-1.0*a2;
         if (delta <=0) b2=-1.0*b2;
         if (delta <=0) c2=-1.0*c2;

        // use "long side" functional form
         double my_t=0.001*t;
         double sqrt_t=sqrt(my_t);
         double t3=my_t*my_t*my_t;
         double delta_mag=fabs(delta);

         double a=a1+a2*delta_mag;
         double b=b1+b2*delta_mag;
         double c=c1+c2*delta_mag+c3*delta*delta;
         f_delta=a*sqrt_t+b*my_t+c*t3;
         f_0=a1*sqrt_t+b1*my_t+c1*t3;



      unsigned int max_index=cdc_drift_table.size()-1;
      if (t>cdc_drift_table[max_index]){
         d=f_delta;
         return d;
      }

      // Drift time is within range of table -- interpolate...
      unsigned int index=0;
      index=Locate(cdc_drift_table,t);
      double dt=cdc_drift_table[index+1]-cdc_drift_table[index];
      double frac=(t-cdc_drift_table[index])/dt;
      double d_0=0.01*(double(index)+frac); 

      double P=0.;
      double tcut=250.0; // ns
      if (t<tcut) {
         P=(tcut-t)/tcut;
      }
      d=f_delta*(d_0/f_0*P+1.-P);
   }
   return d;
}


void ttodfit(int dofit=0, int savefit=0)
{
   // Script for fitting the time to distance relation from data
   // if savefit=1, saves params into newccdb/ccdb_Format....




   TH1I *Bz_hist = (TH1I *) gDirectory->Get("/CDC_TimeToDistance/Bz");
   TF2 *f1,*f2; const Int_t npar = 18;
   bool isFieldOff = false;
   if (Bz_hist == 0) isFieldOff = true;
   f1 = new TF2("f1",TimeToDistance, 1, 1000, -0.3, 0.3, npar);
   f2 = new TF2("f2",TimeToDistance, 1, 1000, -0.3, 0.3, npar);

   f1->SetNpx(1000);
   f1->SetNpy(500);
   f2->SetNpx(1000);
   f2->SetNpy(500);

   //f1 = new TF2("f1",TimeToDistanceFieldOff, 0, 200, -0.18, 0.18, npar);
   //f2 = new TF2("f2",TimeToDistanceFieldOff, 0, 200, -0.18, 0.18, npar);

   TProfile *constants = (TProfile*) gDirectory->Get("/CDC_TimeToDistance/CDC_TD_Constants");
   long_drift_func[0][0] = constants->GetBinContent(101);
   long_drift_func[0][1] = constants->GetBinContent(102);
   long_drift_func[0][2] = constants->GetBinContent(103);
   long_drift_func[1][0] = constants->GetBinContent(104);
   long_drift_func[1][1] = constants->GetBinContent(105);
   long_drift_func[1][2] = constants->GetBinContent(106);
   long_drift_func[2][0] = constants->GetBinContent(107);
   long_drift_func[2][1] = constants->GetBinContent(108);
   long_drift_func[2][2] = constants->GetBinContent(109);
   magnet_correction[0][0] = constants->GetBinContent(110);
   magnet_correction[0][1] = constants->GetBinContent(111);
   short_drift_func[0][0] = constants->GetBinContent(112);
   short_drift_func[0][1] = constants->GetBinContent(113);
   short_drift_func[0][2] = constants->GetBinContent(114);
   short_drift_func[1][0] = constants->GetBinContent(115);
   short_drift_func[1][1] = constants->GetBinContent(116);
   short_drift_func[1][2] = constants->GetBinContent(117);
   short_drift_func[2][0] = constants->GetBinContent(118);
   short_drift_func[2][1] = constants->GetBinContent(119);
   short_drift_func[2][2] = constants->GetBinContent(120);
   magnet_correction[1][0] = constants->GetBinContent(121);
   magnet_correction[1][1] = constants->GetBinContent(122);

   cout << "magnet correction params " << magnet_correction[1][0] <<" " << magnet_correction[1][1] << endl; 


   for (unsigned int i=1; i<=78; i++){
      cdc_drift_table.push_back(constants->GetBinContent(i));
   }

   int run = (int) constants->GetBinContent(125);

   // So now you have the input to your function

   Double_t parameters[npar] =
   {long_drift_func[0][0], long_drift_func[0][1], long_drift_func[0][2],
      long_drift_func[1][0], long_drift_func[1][1], long_drift_func[1][2],
      long_drift_func[2][0], long_drift_func[2][1], long_drift_func[2][2],
      short_drift_func[0][0], short_drift_func[0][1], short_drift_func[0][2],
      short_drift_func[1][0], short_drift_func[1][1], short_drift_func[1][2],
      short_drift_func[2][0], short_drift_func[2][1], short_drift_func[2][2]};

   // fit params from run 51317
   /*
   parameters[0] =  0.981241;
   parameters[1] =  0.108007;
   parameters[2] =  0;
   parameters[3] =  -0.0546685;
   parameters[4] =  -0.692296;
   parameters[5] =  0;
   parameters[6] =  -0.0214396;
   parameters[7] =  0.385219;
   parameters[8] =  -0.248408;

   parameters[9] = 1.00239;
   parameters[10] =  0.458009;
   parameters[11] =  -3.58926;
   parameters[12] =  -0.083118;
   parameters[13] =  -0.330808;
   parameters[14] =  7.2702;
   
   // parameters[6] = 0;
   // parameters[7] = 0;
   // parameters[8] = 0;

   parameters[11]=0;
   parameters[14]=0;
   */

   f1->SetParameters(parameters);

   f2->SetParameters(parameters);

     cout << "Old fit parameters:\n";
     for (int i=0; i<9; i++) printf("%.4f ",parameters[i]);
     cout << endl;
     for (int i=9; i<18; i++) printf("%.4f ",parameters[i]);
     cout << endl;



   /*
      double fractionalRange=0.15;
      for(unsigned int i=0; i < npar; i++){
      f1->SetParLimits(i,parameters[i]-fractionalRange*parameters[i],parameters[i]+fractionalRange*parameters[i]);
      f2->SetParLimits(i,parameters[i]-fractionalRange*parameters[i],parameters[i]+fractionalRange*parameters[i]);
      }
      */

   TProfile2D *profile = (TProfile2D *) gDirectory->Get("/CDC_TimeToDistance/Predicted Drift Distance Vs Delta Vs t_drift, FOM 0.9+");

   //profile->Fit("f2");
   gStyle->SetOptStat(0);

   TCanvas *c1 = new TCanvas ("c1", "c1", 800, 600);
   Double_t contours[21] = 
   { 0.00, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 
      0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0};
   Int_t ncontours = 17;

   //   gPad->SetMaximum(1);
   profile->SetMinimum(0);
   profile->SetMaximum(1);
   profile->SetContour(ncontours, contours);

   profile->DrawCopy("colz");
   profile->DrawCopy("cont3 list same");

   f1->SetContour(ncontours, contours);

   //      f1->SetParameter(12,-0.085);//,0);

   f1->DrawCopy("cont3 list same");

   if (savefit) profile->SetTitle(Form("Run %i %s",run,profile->GetTitle()));

   if (savefit) c1->SaveAs(Form("Before/Before_Run%i.png", run));


   if (dofit) {
     TProfile2D *rebinned = (TProfile2D*) profile->Clone();
     rebinned->SetName("rebinned");
     rebinned->Rebin2D(4,4);

      TCanvas *c2 = new TCanvas ("c2", "c2", 800, 600);
      c2->cd();

      rebinned->SetContour(ncontours, contours);

      rebinned->DrawCopy("colz");
      rebinned->DrawCopy("cont3 list same");
  

      // fix unused params at 0

	f2->SetParameter(2,0);   //a3
	 f2->FixParameter(2,0);

         f2->SetParameter(5,0);   //b3
	 f2->FixParameter(5,0);
      
         f2->SetParameter(8,0);   //c3
	 f2->FixParameter(8,0);
      
	 for (int i=9; i<18; i++) {
	   f2->SetParameter(i,0);
           f2->FixParameter(i,0);
         }


      // drawing without 0 removes the profile
      // option S returns the parameter array
      TFitResultPtr fr = rebinned->Fit("f2", "S0");   

      f2->SetContour(ncontours, contours);

      f2->Draw("cont3 list same");
   // c2->Update();


   // new TCanvas;
   // profileRebin->Draw("colz");

   
      //      if (savefit) c2->SaveAs(Form("newfit_Run%i.png", run));

      if (savefit) rebinned->SetTitle(Form("Run %i %s",run,rebinned->GetTitle()));
      if (savefit) c2->SaveAs(Form("After/After_Run%i.png", run));

      if ((Int_t) fr == 0){ // Fit converged with no errors

        double newparams[18];
        for (int i=0; i<9; i++) newparams[i] = fr->Parameter(i);
        for (int i=0; i<9; i++) newparams[i+9] = fr->Parameter(i);
        newparams[10] = -1.0*newparams[1];
        newparams[13] = -1.0*newparams[4];
        newparams[16] = -1.0*newparams[7];


        cout << "New fit parameters:\n";
        for (int i=0; i<9; i++) printf("%.4f ",newparams[i]);
        cout << endl;
        for (int i=9; i<18; i++) printf("%.4f ",newparams[i]);
        cout << endl;
   
        if (savefit) {
            ofstream outputTextFile;
            outputTextFile.open(Form("ccdb/ccdb_Format_%i.txt",run)); 
            outputTextFile << newparams[0] << " " << newparams[1] << " " << newparams[2] << " " ;
            outputTextFile << newparams[3] << " " << newparams[4] << " " << newparams[5] << " " ;
            outputTextFile << newparams[6] << " " << newparams[7] << " " << newparams[8] << " " ;
            outputTextFile << magnet_correction[0][0] << " " << magnet_correction[0][1] << endl; 
            outputTextFile << newparams[9] << " " << newparams[10] << " " << newparams[11] << " " ;
            outputTextFile << newparams[12] << " " << newparams[13] << " " << newparams[14] << " " ;
            outputTextFile << newparams[15] << " " << newparams[16] << " " << newparams[17] << " " ;
            outputTextFile << magnet_correction[1][0] << " " << magnet_correction[1][1] << endl;
            outputTextFile.close();      
        }
   

  



   gStyle->SetOptFit();

   
   TCanvas *c3 = new TCanvas ("c3", "c3", 800, 600);
   f1->Draw("cont2 list");
   f1->SetLineColor(3);
   f2->Draw("cont2 list same");

   c3->Update();
   c3->SaveAs(Form("Combined/Combined_Run%i.png", run));


   // Get Residual vs Drift Time
   TH2I *resVsT = (TH2I*)gDirectory->Get("/CDC_TimeToDistance/Residual Vs. Drift Time");
   TCanvas *c4 = new TCanvas("c4", "c4", 800, 600);
   resVsT->Draw("colz");
   resVsT->SetTitle(Form("Run %i %s",run,resVsT->GetTitle()));
   c4->SaveAs(Form("ResVsT/ResVsT_Run%i.png", run));

   TCanvas *c5 = new TCanvas("c5", "c5", 800, 600);

   TH1D* p = (TH1D*)resVsT->ProjectionY();
   //p = (TH1D*)resVsT->ProjectionY("_py", resVsT->GetXaxis()->FindBin(100.), -1);
   p->Draw();
   p->Fit("gaus", "sqWR", "", -0.01, 0.01);

   TF1 *g = p->GetFunction("gaus");
   Double_t mean = g->GetParameter(1);
   Double_t sig = g->GetParameter(2);
  
   printf("res mean %.5f sigma %.5f\n",mean,sig);

   c5->SaveAs(Form("Proj/Proj_ResVsT_Run%i.png", run));



   FILE *fitop = fopen("res.txt","a");
   fprintf(fitop,"%d res mean %.5f sigma %.5f\n",run,mean,sig);
   fclose(fitop);

   //----
   // Get Residual vs Drift Time
   TH2I *resVsT9 = (TH2I*)gDirectory->Get("/CDC_TimeToDistance/Residual Vs. Drift Time, FOM 0.9+");
   TH1D* p9 = (TH1D*)resVsT9->ProjectionY();
   //p = (TH1D*)resVsT->ProjectionY("_py", resVsT->GetXaxis()->FindBin(100.), -1);
   p9->Fit("gaus", "sqWR", "", -0.01, 0.01);

   g = p9->GetFunction("gaus");
   mean = g->GetParameter(1);
   sig = g->GetParameter(2);
  
   printf("fom 0.9+ res mean %.5f sigma %.5f\n",mean,sig);

   FILE *fitop9 = fopen("res9.txt","a");
   fprintf(fitop9,"%d res mean %.5f sigma %.5f\n",run,mean,sig);
   fclose(fitop9);

   //----
   // Get Residual vs Drift Time
   TH2I *resVsT6 = (TH2I*)gDirectory->Get("/CDC_TimeToDistance/Residual Vs. Drift Time, FOM 0.6+");
   TH1D* p6 = (TH1D*)resVsT6->ProjectionY();
   //p = (TH1D*)resVsT->ProjectionY("_py", resVsT->GetXaxis()->FindBin(100.), -1);
   p6->Fit("gaus", "sqWR", "", -0.01, 0.01);

   g = p6->GetFunction("gaus");
   mean = g->GetParameter(1);
   sig = g->GetParameter(2);
  
   printf("fom 0.6+ res mean %.5f sigma %.5f\n",mean,sig);

   FILE *fitop6 = fopen("res6.txt","a");
   fprintf(fitop6,"%d res mean %.5f sigma %.5f\n",run,mean,sig);
   fclose(fitop6);


   //----

   TCanvas *c6 = new TCanvas("c6", "c6", 1600, 900);
   c6->Divide(3, 2, 0.001, 0.001);
   c6->cd(1);
   profile->SetContour(ncontours, contours);
   profile->Draw("colz");
   f1->SetContour(ncontours, contours);
   profile->Draw("cont2 list same");
   f1->Draw("cont3 list same");
   c6->Update();

   c6->cd(2);
   profile->Draw("colz");
   f2->SetContour(ncontours, contours);
   profile->Draw("cont2 list same");
   f2->Draw("cont3 list same");
   c6->Update();

   c6->cd(3);
   f1->Draw("cont3 list");
   f1->SetLineColor(3);
   f2->Draw("cont3 list same");
   c6->Update();
   
   c6->cd(4);
   resVsT->Draw("colz");
   c6->Update();

   c6->cd(5);
   p->Draw();
   c6->Update();

   c6->SaveAs(Form("Monitoring/Monitoring_Run%i.png", run));

   FILE *fadd = fopen("addtoccdb.sh","a");
   fprintf(fadd,"ccdb add CDC/drift_parameters -r %i-%i ccdb/ccdb_Format_%i.txt\n",run,run,run);
   fclose(fadd);

   } // if dofit 
      }  // if fr==0

   gApplication->Terminate();
   //return;


}

