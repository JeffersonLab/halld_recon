#include <TFitResultPtr.h>

void fit(int run, int par_col = -100, int par_row = -100){

  gStyle->SetOptStat(1);
  gStyle->SetOptFit(1);

  char file_read[200];
  char file_write[200];
  
  sprintf(file_read,"run_%d.root",run);
  sprintf(file_write,"run_%d.txt",run);
  

  TFile *file      =  new TFile(file_read,"r");
   
  TH1F *histograms[40][40];


  // Fit and plot selected histogram
  if( (par_col != -100) && (par_row != -100)){ 
    
    TCanvas *c1 = new TCanvas("Top North","Top North", 400, 10, 800, 600);
    
    c1->Divide(1,1);
    c1->cd(1);
    
    int col_plot;
    int row_plot;
    
    if(par_col < 0) col_plot = 20 + par_col;
    if(par_col > 0) col_plot = 19 + par_col;
    
    if(par_row < 0) row_plot = 20 + par_row;
    if(par_row > 0) row_plot = 19 + par_row;
    
    for(int i = 0; i < 40;i++) 
      for(int ii = 0; ii < 40;ii++)
	histograms[i][ii] = (TH1F*)file->Get(Form("%d_%d",i,ii));
    
    char title[20];
    sprintf(title,"Col = %d  Row = %d",par_col,par_row);
    histograms[col_plot][row_plot]->SetTitle(title);
    
    
    if(histograms[col_plot][row_plot] == NULL){
      cout << " Histogram does not exist   " <<  "  Col = " <<   col_plot <<  "  Row =   " << row_plot << endl;
      return;
    }
    
    if(histograms[col_plot][row_plot]->GetEntries() == 0){
      cout << " Histogram is empty   " <<  "  Col = " <<   col_plot <<  "  Row =   " << row_plot << endl;
      return;
    }
    
    TFitResultPtr r = histograms[col_plot][row_plot]->Fit("gaus","SL","",30,4096);
    Int_t fitStatus  =  r;
    TF1 *fit =  histograms[col_plot][row_plot]->GetFunction("gaus");

            
  } else {

    FILE  *file_out  =  fopen(file_write,"w");
    
    // Fit, and store fit results of all histograms in the file     
    for(int i = 0; i < 40;i++)
      {
	for(int ii = 0; ii < 40;ii++)
	  {
	    
	    double fit_res1 = 0.;	
	    double fit_res2 = 0.;
	    double fit_res3 = 0.;
	    
	    histograms[i][ii] = (TH1F*)file->Get(Form("%d_%d",i,ii));
	    
	    if(histograms[i][ii] == NULL){
	      cout << " Histogram does not exist   " <<  "  Col = " <<   i <<  "  Row =   " << ii << endl;
	      fprintf(file_out,"%d %d %10.3f   %10.3f   %10.3f \n",i,ii,fit_res1,fit_res2,fit_res3);
	      continue;
	    }
	    
	    if(histograms[i][ii]->GetEntries() == 0){
	      cout << " Histogram is empty   " <<  "  Col = " <<   i <<  "  Row =   " << ii << endl;
	      fprintf(file_out,"%d %d %10.3f   %10.3f   %10.3f \n",i,ii,fit_res1,fit_res2,fit_res3);
	      continue;
	    }
	    
	    TFitResultPtr r = histograms[i][ii]->Fit("gaus","QSL","",30,4096);
	    Int_t fitStatus  =  r;
	    TF1 *fit =  histograms[i][ii]->GetFunction("gaus");
	    
	    if((fit == NULL) || (fitStatus != 0)){
	      cout << " Fit failed  " <<  fitStatus << "  Col = " << i <<  "  Row =  " << ii << endl;
	      fprintf(file_out,"%d %d %10.3f   %10.3f   %10.3f \n",i,ii,fit_res1,fit_res2,fit_res3);
	      continue;
	    }
	    
	    fit_res1 = fit->GetParameter(0);
	    fit_res2 = fit->GetParameter(1);
	    fit_res3 = fit->GetParameter(2);
	    
	    fprintf(file_out,"%d %d %10.3f   %10.3f   %10.3f \n",i,ii,fit_res1,fit_res2,fit_res3);
	    
	    
	  }
      }
    
    fclose(file_out);  // Write results
  }
  
}
