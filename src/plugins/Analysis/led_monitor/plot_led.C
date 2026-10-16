#include <TFile.h>
#include <TH1F.h>
#include <TCanvas.h>

void plot_led(const char* list_of_runs = "list_of_runs", int par_col = -100, int par_row = -100){

  
 
  printf("Open file with the list of runs  %s \n\n",list_of_runs);
  
  FILE *file1  = fopen(list_of_runs,"r");

  Int_t ii = 0;
  Int_t nmod = 0;
  
  Int_t run;
  Int_t col = 0;
  Int_t row = 0;
  float par0 = -100;
  float par1 = -100;
  float par2 = -100;
  

  double run_peak[100][40][40];
  double run_epeak[100][40][40];

  memset(run_peak,0.,sizeof(run_peak));
  memset(run_epeak,0.,sizeof(run_epeak));
  
  while (!feof(file1)){
    fscanf(file1,"%d \n",&run);
    cout << " Process run  " << run << endl;

    char run_name[256];
    sprintf(run_name,"run_%d.txt",run);

    nmod = 0;
    FILE *file2  = fopen(run_name,"r");
    if(file2){      
      while (!feof(file2)){
	fscanf(file2,"%d  %d  %f  %f  %f \n",&col, &row, &par0, &par1, &par2);
	run_peak[ii][col][row]  = par1;
	run_epeak[ii][col][row] = par2;
	nmod++;
      }
      fclose(file2);
    } else{
      cout << " File doesn't exist  " << run_name << endl;
    }

    cout << " Total number of ECAL modules =  " << nmod << endl;
      
    ii++;    
  }
  
  cout << " Number of runs to process: = " << ii << endl;

  fclose(file1);


  double x[100];
  double y[100];
  double ex[100];
  double ey[100];

  TGraphErrors *gr[40][40];
  
  for(int kk = 0; kk < 40; kk++){
    for(int ll = 0; ll < 40; ll++){
      
      memset(x,0,sizeof(x));
      memset(y,0,sizeof(y));
      memset(ex,0,sizeof(ex));
      memset(ey,0,sizeof(ey));
      
      for(int plot = 0; plot < ii; plot++){
	x[plot]  = plot + 1;
	y[plot]  = run_peak[plot][kk][ll];
	ex[plot] = 0;
	ey[plot] = run_epeak[plot][kk][ll];
       	ey[plot] = 0.;
      }

      gr[kk][ll] = new TGraphErrors(ii, x, y, ex, ey);
      gr[kk][ll]->SetMaximum(1.05*y[0]);
      gr[kk][ll]->SetMinimum(0.95*y[0]);
    }
  }
  
  // Plot
  
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
    
    gr[col_plot][row_plot]->SetMarkerStyle(20);
    gr[col_plot][row_plot]->SetMarkerSize(1);
    gr[col_plot][row_plot]->Draw("AP");

    char title[20];
    sprintf(title,"Col = %d  Row = %d",par_col,par_row);
    gr[col_plot][row_plot]->SetTitle(title);

    
  } else {
    
    // Plot quadrants

    TCanvas *c1 = new TCanvas("Top North","Top North", 400, 10, 1000, 900);
    TCanvas *c2 = new TCanvas("Bottom North","Bottom North", 400, 10, 1000, 900);
    TCanvas *c3 = new TCanvas("Top South","Top South", 400, 10, 1000, 900);
    TCanvas *c4 = new TCanvas("Bottom South","Bottom South", 400, 10, 1000, 900);
    
    c1->Divide(20,20,0.0001,0.0001);
    
    for(int col = 20; col < 40; col++){
      for(int row = 20; row < 40; row++){
	
	int index = col - 20 + 20*(39 - row) + 1;
	
	c1->cd(index);
	
	gPad->SetBottomMargin(0.);
	gPad->SetLeftMargin(0.);
	
	gr[col][row]->SetMarkerStyle(20);
	gr[col][row]->SetMarkerSize(1);
	gr[col][row]->SetMarkerColor(4);
	gr[col][row]->Draw("AP"); 
	
      }
    }
    c1->Update();
    
    c2->Divide(20,20,0.0001,0.0001);
    
    for(int  col = 20; col < 40; col++){
      for(int  row = 0; row < 20; row++){
	
	int index = col - 20 + 20*(19 - row) + 1;
	
      //      if(col == 34 && row == 0) continue;
	
	c2->cd(index);
	
	gPad->SetBottomMargin(0.);
	gPad->SetLeftMargin(0.);
	
	gr[col][row]->SetMarkerStyle(20);
	gr[col][row]->SetMarkerSize(1);
	gr[col][row]->SetMarkerColor(6);
	gr[col][row]->Draw("AP"); 
	
      }
    }
    
    c2->Update();
    
    c3->Divide(20,20,0.0001,0.0001);
    
    for(int  col = 0; col < 20; col++){
      for(int  row = 20; row < 40; row++){
	
	int index = col + 20*(39 - row) + 1;
      
	c3->cd(index);
	
	gPad->SetBottomMargin(0.);
	gPad->SetLeftMargin(0.);
	
	gr[col][row]->SetMarkerStyle(20);
	gr[col][row]->SetMarkerSize(1);
	gr[col][row]->SetMarkerColor(1);
	gr[col][row]->Draw("AP"); 
	
      }
    }
    
    c3->Update();
    
    c4->Divide(20,20,0.0001,0.0001);
    
    for(int  col = 0; col < 20; col++){
      for(int  row = 0; row < 20; row++){
	
	int index = col + 20*(19 - row) + 1;
	
	
	c4->cd(index);
	
	gPad->SetBottomMargin(0.);
	gPad->SetLeftMargin(0.);
	
	gr[col][row]->SetMarkerStyle(20);
	gr[col][row]->SetMarkerSize(1);
	gr[col][row]->SetMarkerColor(8);
	gr[col][row]->Draw("AP"); 
	
      }
    }
    
    c4->Update();
  }
  
}
