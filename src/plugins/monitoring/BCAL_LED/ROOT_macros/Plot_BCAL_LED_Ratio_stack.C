#include <unistd.h>
using namespace std;


void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

TFile *GetReferenceFile(Int_t run, TString variation = "default"){
    char command[1024];
    // first get the location of the file
    sprintf(command, "ccdb dump %s:%i:%s", "/BCAL/LED_monitoring/reference_file", run, variation.Data());
    FILE* inputPipe = gSystem->OpenPipe(command, "r");
    if(inputPipe == NULL)
        return NULL;
    //get the first (comment) line
    char buff[1024];
    if(fgets(buff, sizeof(buff), inputPipe) == NULL)
        return NULL;
    //get the actual data
    if(fgets(buff, sizeof(buff), inputPipe) == NULL)
        return NULL;
    stringstream ss;
    ss << buff;
    char url_base[1024], path_name[1024];
    ss >> url_base >> path_name;
    //Close the pipe
    gSystem->ClosePipe(inputPipe);

    // build URL of the location of the file
    stringstream ss_url;
    //cout << url_base << endl;
    //cout << table_name << endl;
	ss_url << url_base << "/" << path_name;
	
	// figure out the name of the file
	vector<string> tokens;
	split(path_name, '/', tokens);
	string filename = *(tokens.end()-1);
	
	// download the file if it doesn't already exist
	ifstream f(filename.c_str());
	if(!f.good()) {
		sprintf(command, "curl -O %s", ss_url.str().c_str());
		gSystem->Exec(command);
	} else {
		f.close();
	}


	// open it and return
	TFile *rootfile = new TFile(filename.c_str());
	
	// error check
	
	return rootfile;
}

void Plot_BCAL_LED_Ratio_stack(string infilename = "hd_root.root", int run = 20000)
{
  // #include <TRandom.h>

  //gROOT->Reset();
gStyle->SetPalette(1,0);
gStyle->SetOptStat(kFALSE);
// gStyle->SetOptStat(11111111);
gStyle->SetPadRightMargin(0.15);
gStyle->SetPadLeftMargin(0.15);
gStyle->SetPadBottomMargin(0.15);
//

   char string[256];
   //char path[256];
   Int_t j,k;
   Int_t n;
   Int_t errorbars = 0;
    
    TProfile *run1_low_up_1 = NULL;
    TProfile *run1_low_up_2 = NULL;
    TProfile *run1_low_up_3 = NULL;
    TProfile *run1_low_up_4 = NULL;
    TProfile *run1_low_down_1 = NULL;
    TProfile *run1_low_down_2 = NULL;
    TProfile *run1_low_down_3 = NULL;
    TProfile *run1_low_down_4 = NULL;

    TProfile *run1_high_up_1 = NULL;
    TProfile *run1_high_up_2 = NULL;
    TProfile *run1_high_up_3 = NULL;
    TProfile *run1_high_up_4 = NULL;
    TProfile *run1_high_down_1 = NULL;
    TProfile *run1_high_down_2 = NULL;
    TProfile *run1_high_down_3 = NULL;
    TProfile *run1_high_down_4 = NULL;

    TProfile *run2_low_up_1 = NULL;
    TProfile *run2_low_up_2 = NULL;
    TProfile *run2_low_up_3 = NULL;
    TProfile *run2_low_up_4 = NULL;
    TProfile *run2_low_down_1 = NULL;
    TProfile *run2_low_down_2 = NULL;
    TProfile *run2_low_down_3 = NULL;
    TProfile *run2_low_down_4 = NULL;

    TProfile *run2_high_up_1 = NULL;
    TProfile *run2_high_up_2 = NULL;
    TProfile *run2_high_up_3 = NULL;
    TProfile *run2_high_up_4 = NULL;
    TProfile *run2_high_down_1 = NULL;
    TProfile *run2_high_down_2 = NULL;
    TProfile *run2_high_down_3 = NULL;
    TProfile *run2_high_down_4 = NULL;

    TH1D *run1_low_up_1_hist = NULL;
    TH1D *run1_low_up_2_hist = NULL;
    TH1D *run1_low_up_3_hist = NULL;
    TH1D *run1_low_up_4_hist = NULL;
    TH1D *run1_low_down_1_hist = NULL;
    TH1D *run1_low_down_2_hist = NULL;
    TH1D *run1_low_down_3_hist = NULL;
    TH1D *run1_low_down_4_hist = NULL;

    TH1D *run1_high_up_1_hist = NULL;
    TH1D *run1_high_up_2_hist = NULL;
    TH1D *run1_high_up_3_hist = NULL;
    TH1D *run1_high_up_4_hist = NULL;
    TH1D *run1_high_down_1_hist = NULL;
    TH1D *run1_high_down_2_hist = NULL;
    TH1D *run1_high_down_3_hist = NULL;
    TH1D *run1_high_down_4_hist = NULL;

    TH1D *run2_low_up_1_hist = NULL;
    TH1D *run2_low_up_2_hist = NULL;
    TH1D *run2_low_up_3_hist = NULL;
    TH1D *run2_low_up_4_hist = NULL;
    TH1D *run2_low_down_1_hist = NULL;
    TH1D *run2_low_down_2_hist = NULL;
    TH1D *run2_low_down_3_hist = NULL;
    TH1D *run2_low_down_4_hist = NULL;

    TH1D *run2_high_up_1_hist = NULL;
    TH1D *run2_high_up_2_hist = NULL;
    TH1D *run2_high_up_3_hist = NULL;
    TH1D *run2_high_up_4_hist = NULL;
    TH1D *run2_high_down_1_hist = NULL;
    TH1D *run2_high_down_2_hist = NULL;
    TH1D *run2_high_down_3_hist = NULL;
    TH1D *run2_high_down_4_hist = NULL;

   
//   sprintf(string,"hd_root.root");
//   sprintf(string,"0%03i/bcalled0%03i.root",runnumber1,runnumber1);
   printf ("Histogram input filename=%s\n",infilename.c_str());
   TFile *run1in = new TFile(infilename.c_str(),"read");
      
	
	
		run1_low_up_1 = (TProfile*)run1in->Get("bcalLED/low_bias_up_sector_1_peak_vchannel");
		run1_low_up_2 = (TProfile*)run1in->Get("bcalLED/low_bias_up_sector_2_peak_vchannel");
		run1_low_up_3 = (TProfile*)run1in->Get("bcalLED/low_bias_up_sector_3_peak_vchannel"); 
		run1_low_up_4 = (TProfile*)run1in->Get("bcalLED/low_bias_up_sector_4_peak_vchannel");
		run1_low_down_1 = (TProfile*)run1in->Get("bcalLED/low_bias_down_sector_1_peak_vchannel");
		run1_low_down_2 = (TProfile*)run1in->Get("bcalLED/low_bias_down_sector_2_peak_vchannel");
		run1_low_down_3 = (TProfile*)run1in->Get("bcalLED/low_bias_down_sector_3_peak_vchannel");
		run1_low_down_4 = (TProfile*)run1in->Get("bcalLED/low_bias_down_sector_4_peak_vchannel");

		run1_high_up_1 = (TProfile*)run1in->Get("bcalLED/high_bias_up_sector_1_peak_vchannel");
		run1_high_up_2 = (TProfile*)run1in->Get("bcalLED/high_bias_up_sector_2_peak_vchannel");
		run1_high_up_3 = (TProfile*)run1in->Get("bcalLED/high_bias_up_sector_3_peak_vchannel");
		run1_high_up_4 = (TProfile*)run1in->Get("bcalLED/high_bias_up_sector_4_peak_vchannel");
		run1_high_down_1 = (TProfile*)run1in->Get("bcalLED/high_bias_down_sector_1_peak_vchannel");
		run1_high_down_2 = (TProfile*)run1in->Get("bcalLED/high_bias_down_sector_2_peak_vchannel");
		run1_high_down_3 = (TProfile*)run1in->Get("bcalLED/high_bias_down_sector_3_peak_vchannel");
		run1_high_down_4 = (TProfile*)run1in->Get("bcalLED/high_bias_down_sector_4_peak_vchannel");
		
		//sprintf(string,"BCAL_LED_Reference_Spring2016.root");
		//printf ("Histogram input filename=%s\n",string);
		//TFile *run2in = new TFile(string,"read");
		TFile *run2in = GetReferenceFile(run);
		
		run2_low_up_1 = (TProfile*)run2in->Get("bcalLED/low_bias_up_sector_1_peak_vchannel");
		run2_low_up_2 = (TProfile*)run2in->Get("bcalLED/low_bias_up_sector_2_peak_vchannel");
		run2_low_up_3 = (TProfile*)run2in->Get("bcalLED/low_bias_up_sector_3_peak_vchannel"); 
		run2_low_up_4 = (TProfile*)run2in->Get("bcalLED/low_bias_up_sector_4_peak_vchannel");
		run2_low_down_1 = (TProfile*)run2in->Get("bcalLED/low_bias_down_sector_1_peak_vchannel");
		run2_low_down_2 = (TProfile*)run2in->Get("bcalLED/low_bias_down_sector_2_peak_vchannel");
		run2_low_down_3 = (TProfile*)run2in->Get("bcalLED/low_bias_down_sector_3_peak_vchannel");
		run2_low_down_4 = (TProfile*)run2in->Get("bcalLED/low_bias_down_sector_4_peak_vchannel");

		run2_high_up_1 = (TProfile*)run2in->Get("bcalLED/high_bias_up_sector_1_peak_vchannel");
		run2_high_up_2 = (TProfile*)run2in->Get("bcalLED/high_bias_up_sector_2_peak_vchannel");
		run2_high_up_3 = (TProfile*)run2in->Get("bcalLED/high_bias_up_sector_3_peak_vchannel");
		run2_high_up_4 = (TProfile*)run2in->Get("bcalLED/high_bias_up_sector_4_peak_vchannel");
		run2_high_down_1 = (TProfile*)run2in->Get("bcalLED/high_bias_down_sector_1_peak_vchannel");
		run2_high_down_2 = (TProfile*)run2in->Get("bcalLED/high_bias_down_sector_2_peak_vchannel");
		run2_high_down_3 = (TProfile*)run2in->Get("bcalLED/high_bias_down_sector_3_peak_vchannel");
		run2_high_down_4 = (TProfile*)run2in->Get("bcalLED/high_bias_down_sector_4_peak_vchannel");
		
		cout << "input profiles, projecting into histograms" << "\n";
		
		 run1_low_up_1_hist = run1_low_up_1->ProjectionX("run1_low_up_1_projectionx");
		 run1_low_up_2_hist = run1_low_up_2->ProjectionX("run1_low_up_2_projectionx");
		 run1_low_up_3_hist = run1_low_up_3->ProjectionX("run1_low_up_3_projectionx");
		 run1_low_up_4_hist = run1_low_up_4->ProjectionX("run1_low_up_4_projectionx");
		 run1_low_down_1_hist = run1_low_down_1->ProjectionX("run1_low_down_1_projectionx");
		 run1_low_down_2_hist = run1_low_down_2->ProjectionX("run1_low_down_2_projectionx");
		 run1_low_down_3_hist = run1_low_down_3->ProjectionX("run1_low_down_3_projectionx");
		 run1_low_down_4_hist = run1_low_down_4->ProjectionX("run1_low_down_4_projectionx");

		 run1_high_up_1_hist = run1_high_up_1->ProjectionX("run1_high_up_1_projectionx");
		 run1_high_up_2_hist = run1_high_up_2->ProjectionX("run1_high_up_2_projectionx");
		 run1_high_up_3_hist = run1_high_up_3->ProjectionX("run1_high_up_3_projectionx");
		 run1_high_up_4_hist = run1_high_up_4->ProjectionX("run1_high_up_4_projectionx");
		 run1_high_down_1_hist = run1_high_down_1->ProjectionX("run1_high_down_1_projectionx");
		 run1_high_down_2_hist = run1_high_down_2->ProjectionX("run1_high_down_2_projectionx");
		 run1_high_down_3_hist = run1_high_down_3->ProjectionX("run1_high_down_3_projectionx");
		 run1_high_down_4_hist = run1_high_down_4->ProjectionX("run1_high_down_4_projectionx");
		
		 run2_low_up_1_hist = run2_low_up_1->ProjectionX("run2_low_up_1_projectionx");
		 run2_low_up_2_hist = run2_low_up_2->ProjectionX("run2_low_up_2_projectionx");
		 run2_low_up_3_hist = run2_low_up_3->ProjectionX("run2_low_up_3_projectionx");
		 run2_low_up_4_hist = run2_low_up_4->ProjectionX("run2_low_up_4_projectionx");
		 run2_low_down_1_hist = run2_low_down_1->ProjectionX("run2_low_down_1_projectionx");
		 run2_low_down_2_hist = run2_low_down_2->ProjectionX("run2_low_down_2_projectionx");
		 run2_low_down_3_hist = run2_low_down_3->ProjectionX("run2_low_down_3_projectionx");
		 run2_low_down_4_hist = run2_low_down_4->ProjectionX("run2_low_down_4_projectionx");

		 run2_high_up_1_hist = run2_high_up_1->ProjectionX("run2_high_up_1_projectionx");
		 run2_high_up_2_hist = run2_high_up_2->ProjectionX("run2_high_up_2_projectionx");
		 run2_high_up_3_hist = run2_high_up_3->ProjectionX("run2_high_up_3_projectionx");
		 run2_high_up_4_hist = run2_high_up_4->ProjectionX("run2_high_up_4_projectionx");
		 run2_high_down_1_hist = run2_high_down_1->ProjectionX("run2_high_down_1_projectionx");
		 run2_high_down_2_hist = run2_high_down_2->ProjectionX("run2_high_down_2_projectionx");
		 run2_high_down_3_hist = run2_high_down_3->ProjectionX("run2_high_down_3_projectionx");
		 run2_high_down_4_hist = run2_high_down_4->ProjectionX("run2_high_down_4_projectionx");

		
		cout << "profiles projected into histograms" << "\n";

		run1_low_up_1_hist->SetTitle("6v Upstream sector 1");
		run1_low_up_2_hist->SetTitle("6v Upstream sector 2");
		run1_low_up_3_hist->SetTitle("6v Upstream sector 3"); 
		run1_low_up_4_hist->SetTitle("6v Upstream sector 4");
		run1_low_down_1_hist->SetTitle("6v Downstream sector 1");
		run1_low_down_2_hist->SetTitle("6v Downstream sector 2");
		run1_low_down_3_hist->SetTitle("6v Downstream sector 3");
		run1_low_down_4_hist->SetTitle("6v Downstream sector 4");

		run1_high_up_1_hist->SetTitle("6.25v Upstream sector 1");
		run1_high_up_2_hist->SetTitle("6.25v Upstream sector 2");
		run1_high_up_3_hist->SetTitle("6.25v Upstream sector 3");
		run1_high_up_4_hist->SetTitle("6.25v Upstream sector 4");
		run1_high_down_1_hist->SetTitle("6.25v Downstream sector 1");
		run1_high_down_2_hist->SetTitle("6.25v Downstream sector 2");
		run1_high_down_3_hist->SetTitle("6.25v Downstream sector 3");
		run1_high_down_4_hist->SetTitle("6.25v Downstream sector 4");
		
		cout << "Divideing histograms" << "\n";
		
		run1_low_up_1_hist->Divide(run2_low_up_1_hist);
		run1_low_up_2_hist->Divide(run2_low_up_2_hist);
		run1_low_up_3_hist->Divide(run2_low_up_3_hist); 
		run1_low_up_4_hist->Divide(run2_low_up_4_hist);
		run1_low_down_1_hist->Divide(run2_low_down_1_hist);
		run1_low_down_2_hist->Divide(run2_low_down_2_hist);
		run1_low_down_3_hist->Divide(run2_low_down_3_hist);
		run1_low_down_4_hist->Divide(run2_low_down_4_hist);

		run1_high_up_1_hist->Divide(run2_high_up_1_hist);
		run1_high_up_2_hist->Divide(run2_high_up_2_hist);
		run1_high_up_3_hist->Divide(run2_high_up_3_hist);
		run1_high_up_4_hist->Divide(run2_high_up_4_hist);
		run1_high_down_1_hist->Divide(run2_high_down_1_hist);
		run1_high_down_2_hist->Divide(run2_high_down_2_hist);
		run1_high_down_3_hist->Divide(run2_high_down_3_hist);
		run1_high_down_4_hist->Divide(run2_high_down_4_hist);
		
		cout << "Histograms divided" << "\n";
		run1_low_up_1_hist->SetMinimum(0.8);
		run1_low_up_1_hist->SetMaximum(1.2);
		run1_low_up_2_hist->SetMinimum(0.8);
		run1_low_up_2_hist->SetMaximum(1.2);
		run1_low_up_3_hist->SetMinimum(0.8);
		run1_low_up_3_hist->SetMaximum(1.2);
		run1_low_up_4_hist->SetMinimum(0.8);
		run1_low_up_4_hist->SetMaximum(1.2);

		run1_low_down_1_hist->SetMinimum(0.8);
		run1_low_down_1_hist->SetMaximum(1.2);
		run1_low_down_2_hist->SetMinimum(0.8);
		run1_low_down_2_hist->SetMaximum(1.2);
		run1_low_down_3_hist->SetMinimum(0.8);
		run1_low_down_3_hist->SetMaximum(1.2);
		run1_low_down_4_hist->SetMinimum(0.8);
		run1_low_down_4_hist->SetMaximum(1.2);

		run1_high_up_1_hist->SetMinimum(0.8);
		run1_high_up_1_hist->SetMaximum(1.2);
		run1_high_up_2_hist->SetMinimum(0.8);
		run1_high_up_2_hist->SetMaximum(1.2);
		run1_high_up_3_hist->SetMinimum(0.8);
		run1_high_up_3_hist->SetMaximum(1.2);
		run1_high_up_4_hist->SetMinimum(0.8);
		run1_high_up_4_hist->SetMaximum(1.2);

		run1_high_down_1_hist->SetMinimum(0.8);
		run1_high_down_1_hist->SetMaximum(1.2);
		run1_high_down_2_hist->SetMinimum(0.8);
		run1_high_down_2_hist->SetMaximum(1.2);
		run1_high_down_3_hist->SetMinimum(0.8);
		run1_high_down_3_hist->SetMaximum(1.2);
		run1_high_down_4_hist->SetMinimum(0.8);
		run1_high_down_4_hist->SetMaximum(1.2);


   TCanvas *c1 = new TCanvas("c1","c1 pedsubpeak ",200,10,1200,1200);
   c1->Divide(2,2);
   
////////////////////////////////////////////////
   TVirtualPad *c1_1 = c1->cd(1);
   c1_1->SetBorderMode(0);
   c1_1->SetFillColor(0);

   c1_1->SetGridx();
   c1_1->SetGridy();
   c1_1->SetBorderMode(0);
   c1_1->SetFillColor(0);
   
    if (errorbars ==0) {run1_low_up_1_hist->Draw("hist p");
      run1_low_up_2_hist->Draw("same hist p");
      run1_low_up_3_hist->Draw("same hist p");
      run1_low_up_4_hist->Draw("same hist p");
    }
    else {run1_low_up_1_hist->Draw("E1 X0");
      run1_low_up_2_hist->Draw("same E1 X0");
      run1_low_up_3_hist->Draw("same E1 X0");
      run1_low_up_4_hist->Draw("same E1 X0");
    }

    run1_low_up_1_hist->SetMarkerColor(1);    
    run1_low_up_1_hist->SetMarkerStyle(2);
    run1_low_up_2_hist->SetMarkerColor(2);    
    run1_low_up_1_hist->SetMarkerStyle(2);
    run1_low_up_3_hist->SetMarkerColor(3);    
    run1_low_up_1_hist->SetMarkerStyle(2);
    run1_low_up_4_hist->SetMarkerColor(4);    
    run1_low_up_1_hist->SetMarkerStyle(2);

   
    sprintf(string,"Run ratio to reference");
//    sprintf(string,"Run %05d ratio to reference",runnumber1);
   TLatex *t1 = new TLatex(0.15,0.92,string);
   t1->SetNDC();
   t1->SetTextSize(0.03);
   t1->Draw(); 

///////////////////////////////////////////////////////////////////////////
   TVirtualPad *c1_2 = c1->cd(2);
   c1_2->SetBorderMode(0);
   c1_2->SetFillColor(0);

   c1_2->SetGridx();
   c1_2->SetGridy();
   c1_2->SetBorderMode(0);
   c1_2->SetFillColor(0);
   
    if (errorbars ==0) {run1_low_down_1_hist->Draw("hist p");
      run1_low_down_2_hist->Draw("same hist p");
      run1_low_down_3_hist->Draw("same hist p");
      run1_low_down_4_hist->Draw("same hist p");
    }
    else {run1_low_down_1_hist->Draw("E1 X0");
      run1_low_down_2_hist->Draw("same E1 X0");
      run1_low_down_3_hist->Draw("same E1 X0");
      run1_low_down_4_hist->Draw("same E1 X0");
    }

    run1_low_down_1_hist->SetMarkerColor(1);    
    run1_low_down_1_hist->SetMarkerStyle(2);
    run1_low_down_2_hist->SetMarkerColor(2);    
    run1_low_down_1_hist->SetMarkerStyle(2);
    run1_low_down_3_hist->SetMarkerColor(3);    
    run1_low_down_1_hist->SetMarkerStyle(2);
    run1_low_down_4_hist->SetMarkerColor(4);    
    run1_low_down_1_hist->SetMarkerStyle(2);

///////////////////////////////////////////////////////////////////////////
   TVirtualPad *c1_3 = c1->cd(3);
   c1_3->SetBorderMode(0);
   c1_3->SetFillColor(0);


   c1_3->SetGridx();
   c1_3->SetGridy();
   c1_3->SetBorderMode(0);
   c1_3->SetFillColor(0);
   
    if (errorbars ==0) {run1_high_up_1_hist->Draw("hist p");
      run1_high_up_2_hist->Draw("same hist p");
      run1_high_up_3_hist->Draw("same hist p");
      run1_high_up_4_hist->Draw("same hist p");
    }
    else {run1_high_up_1_hist->Draw("E1 X0");
      run1_high_up_2_hist->Draw("same E1 X0");
      run1_high_up_3_hist->Draw("same E1 X0");
      run1_high_up_4_hist->Draw("same E1 X0");
    }

    run1_high_up_1_hist->SetMarkerColor(1);    
    run1_high_up_1_hist->SetMarkerStyle(2);
    run1_high_up_2_hist->SetMarkerColor(2);    
    run1_high_up_1_hist->SetMarkerStyle(2);
    run1_high_up_3_hist->SetMarkerColor(3);    
    run1_high_up_1_hist->SetMarkerStyle(2);
    run1_high_up_4_hist->SetMarkerColor(4);    
    run1_high_up_1_hist->SetMarkerStyle(2);


    ///////////////////////////////////////////////////////////////////////////
   TVirtualPad *c1_4 = c1->cd(4);
   c1_4->SetBorderMode(0);
   c1_4->SetFillColor(0);

   c1_4->SetGridx();
   c1_4->SetGridy();
   c1_4->SetBorderMode(0);
   c1_4->SetFillColor(0);
   
    if (errorbars ==0) {run1_high_down_1_hist->Draw("hist p");
      run1_high_down_2_hist->Draw("same hist p");
      run1_high_down_3_hist->Draw("same hist p");
      run1_high_down_4_hist->Draw("same hist p");
    }
    else {run1_high_down_1_hist->Draw("E1 X0");
      run1_high_down_2_hist->Draw("same E1 X0");
      run1_high_down_3_hist->Draw("same E1 X0");
      run1_high_down_4_hist->Draw("same E1 X0");
    }

    run1_high_down_1_hist->SetMarkerColor(1);    
    run1_high_down_1_hist->SetMarkerStyle(2);
    run1_high_down_2_hist->SetMarkerColor(2);    
    run1_high_down_1_hist->SetMarkerStyle(2);
    run1_high_down_3_hist->SetMarkerColor(3);    
    run1_high_down_1_hist->SetMarkerStyle(2);
    run1_high_down_4_hist->SetMarkerColor(4);    
    run1_high_down_1_hist->SetMarkerStyle(2);

 ///////////////////////////////////////////////////////////////////////////////////////////
    
    
    
    
//     sprintf(string,"bcalled_monitoring.pdf");
//     c1->SaveAs(string);
    
    
//     sprintf(string,"bcalled_%05d_monitoring.pdf",runnumber1);
//     c1->SaveAs(string);
     
    
//gApplication->Terminate();
}

