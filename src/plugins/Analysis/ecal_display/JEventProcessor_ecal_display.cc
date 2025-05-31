// $Id$
//
//    File: JEventProcessor_ecal_display.cc
// Created: Thu May 29 09:06:37 AM EDT 2025
// Creator: somov (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_ecal_display.h"
using namespace std;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_ecal_display());
}
} // "C"


//------------------
// JEventProcessor_ecal_display (Constructor)
//------------------
JEventProcessor_ecal_display::JEventProcessor_ecal_display()
{
     SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_ecal_display (Destructor)
//------------------
JEventProcessor_ecal_display::~JEventProcessor_ecal_display()
{
}

//------------------
// Init
//------------------
void JEventProcessor_ecal_display::Init()
{
  // This is called once at program startup. 
  
    auto app = GetApplication();
    // lockService should be initialized here like this
    lockService = app->GetService<JLockService>();

    
    par_region        =   1;
    par_print_thr     =   200;
    par_max_amp       =   0;
    
    JParameterManager *parms = app->GetJParameterManager();
    parms->SetDefaultParameter("DET:REG", par_region);
    parms->SetDefaultParameter("MAX:AMP", par_max_amp);
    
    parms->SetDefaultParameter("PRINT:THR", par_print_thr);
    
    gStyle->SetOptStat(0);
    
    gStyle->SetTitleFontSize(0.05);
    
    gStyle->SetTitleFont(130,"xy");
    gStyle->SetLabelFont(130,"xy");
    
    gStyle->SetLineWidth(1.5);
    gStyle->SetTextSize(1.2);
    
    
    gStyle->SetLabelSize(0.04,"xy");
    gStyle->SetTitleSize(0.03,"xy");
    
    
    gStyle->SetTitleOffset(0.5,"x");
    gStyle->SetTitleOffset(0.2,"y");
    
    gStyle->SetTitleBorderSize(0);
    
       
    app1 =  new TApplication("app1",NULL,0);
            
    for(int ii = 0; ii < 40; ii++){
      for(int jj = 0; jj < 40; jj++){
	char name[100];
	sprintf(name,"%i_%i",ii,jj);
	ecal_wave_ind[ii][jj] = new TProfile(name,name,100,-0.5,99.5,-10.,14096);
	
      }
    }
            
    my_canvas  = new TCanvas("ECAL ", "ECAL ",1000, 800);
  
    my_canvas->Divide(20,20,0001,0.0001);

    
}

//------------------
// BeginRun
//------------------
void JEventProcessor_ecal_display::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_ecal_display::Process(const std::shared_ptr<const JEvent> &event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.
    // Here's an example:
    //
    // vector<const MyDataClass*> mydataclasses;
    // event->Get(mydataclasses);
    //
    // If you have lockService initialized in Init() then you can acquire locks like this

    vector<const DECALDigiHit *> ecal_digi_hits; 
  
    TLatex latexMax;
    float yMax = 0.6;
    float xMax = 0.0;

    event->Get(ecal_digi_hits);
  
    cout << " Number of hits = " << ecal_digi_hits.size() << endl;
  
    if( ecal_digi_hits.size() < 1000){
      return;
    }
    
    lockService->RootFillLock(this); 
    
    for(unsigned int ii = 0; ii < ecal_digi_hits.size(); ii++){
      const DECALDigiHit *ecal_hit = ecal_digi_hits[ii];
      
      int row     =  ecal_hit->row;
      int column  =  ecal_hit->column; 
      
      const Df250WindowRawData *windorawdata;
      const Df250PulseData     *pulsedata; 
      
      ecal_hit->GetSingle(pulsedata);         
      pulsedata->GetSingle(windorawdata);
      
      if(pulsedata){
	       	
	if(windorawdata){       
		
	  const vector<uint16_t> &samplesvector = windorawdata->samples;
	  
	  unsigned int nsamples = samplesvector.size();
	  	  
	  for(uint16_t samp = 0; samp < nsamples; samp++){
	    
	    unsigned int adc_amp = samplesvector[samp];

	    //	    cout << " I am here " <<  adc_amp << endl;
	    
	    ecal_wave_ind[column][row]->Fill(float(samp),float(adc_amp));	    	    
	    
	  }  // Loop over samples	  	 	  
	  
	}	
      }
      
    }  // Loop over digi hits
    
    my_canvas->Divide(20,20,0.0001,0.0001);

    // 1 - North Top
    // 2 - North Bottom
    // 3 - South Top
    // 4 - South Bottom
    
    int plot_waveforms = par_region;
    
    if(plot_waveforms == 1)
      {      
	for(int  col = 20; col < 40; col++){
	  for(int  row = 20; row < 40; row++){
	    
	    int index = col - 20 + 20*(39 - row) + 1;
	    
	    my_canvas->cd(index);
	    
	    gPad->SetBottomMargin(0.);
	    gPad->SetLeftMargin(0.);
	    
	    TString textMax = Form(" %d",(int)ecal_wave_ind[col][row]->GetMaximum());
	    latexMax.SetTextSize(0.0);
	    latexMax.SetTextColor(1);
	    
	    if(ecal_wave_ind[col][row]->GetMaximum() < par_print_thr)
	      {
		ecal_wave_ind[col][row]->SetLineColor(2);
		int max =  ecal_wave_ind[col][row]->GetMaximum() - 100.;
		cout << "Column: " << col-19 <<   "    Row: " << row-19 << "   Amp  (mV): "<< max/2. << endl << endl;
		latexMax.SetTextSize(0.3);	      
	      }  //color coding

	    if(par_max_amp > 0)
	      ecal_wave_ind[col][row]->SetMaximum(par_max_amp);

	    ecal_wave_ind[col][row]->Draw("histo");

	    if(par_print_thr > 0)
	      latexMax.DrawLatexNDC(xMax,yMax, textMax);
	    
	  }
	}
      }   // NORTH TOP
    
    
    else if(plot_waveforms == 2)
      {	
	for(int  col = 20; col < 40; col++){
	  for(int  row = 0; row < 20; row++){
	    
	    int index = col - 20 + 20*(19 - row) + 1;
	    
	    my_canvas->cd(index);
	    
	    gPad->SetBottomMargin(0.);
	    gPad->SetLeftMargin(0.);
	    
	    TString textMax = Form(" %d",(int)ecal_wave_ind[col][row]->GetMaximum());
	    latexMax.SetTextSize(0.0);
	    latexMax.SetTextColor(1);
	    
	    if(ecal_wave_ind[col][row]->GetMaximum() < par_print_thr)
	      {
		ecal_wave_ind[col][row]->SetLineColor(2);
		int max =  ecal_wave_ind[col][row]->GetMaximum() - 100.;
		cout << "Column: " << col-19 <<   "    Row: " << row-20 << "   Amp (mV): "<< max/2. << endl << endl;
		latexMax.SetTextSize(0.3);
	      }  //color coding

	    if(par_max_amp > 0)
	      ecal_wave_ind[col][row]->SetMaximum(par_max_amp);

	    ecal_wave_ind[col][row]->Draw("histo");
	    
	    if(par_print_thr > 0)
	      latexMax.DrawLatexNDC(xMax,yMax, textMax);
	    
	  }
	}
      }   // NORTH BOTTOM
    
    
    else if(plot_waveforms == 3)
      {
	
	for(int  col = 0; col < 20; col++){
	  for(int  row = 20; row < 40; row++){
	    
	    int index = col + 20*(39 - row) + 1;
	    
	    my_canvas->cd(index);
	    
	    gPad->SetBottomMargin(0.);
	    gPad->SetLeftMargin(0.);
	    
	    TString textMax = Form(" %d",(int)ecal_wave_ind[col][row]->GetMaximum());
	    latexMax.SetTextSize(0.0);
	    latexMax.SetTextColor(1);
	    
	    if(ecal_wave_ind[col][row]->GetMaximum() < par_print_thr)
	      {
		ecal_wave_ind[col][row]->SetLineColor(2);
		int max =  ecal_wave_ind[col][row]->GetMaximum() - 100.;
		cout << "Column: " << col-20 <<   "    Row: " << row-19 << "   Amp  (mV): "<< max/2. << endl << endl;
		latexMax.SetTextSize(0.3);
	      }  //color coding

	    if(par_max_amp > 0)
	      ecal_wave_ind[col][row]->SetMaximum(par_max_amp);
	    
	    ecal_wave_ind[col][row]->Draw("histo");
	    
	    if(par_print_thr > 0)
	      latexMax.DrawLatexNDC(xMax,yMax, textMax);
	    	    
	  }
	}
      } // SOUTH TOP
    
    else if(plot_waveforms == 4)
      {		
	for(int  col = 0; col < 20; col++){
	  for(int  row = 0; row < 20; row++){
	    
	    int index = col + 20*(19 - row) + 1;
	    
	    my_canvas->cd(index);
	    
	    gPad->SetBottomMargin(0.);
	    gPad->SetLeftMargin(0.);

	    TString textMax = Form(" %d",(int)ecal_wave_ind[col][row]->GetMaximum());
	    latexMax.SetTextSize(0.0);
	    latexMax.SetTextColor(1);
	    
	    if(ecal_wave_ind[col][row]->GetMaximum() < par_print_thr)
	      {
		ecal_wave_ind[col][row]->SetLineColor(2);
		
		int max =  ecal_wave_ind[col][row]->GetMaximum() - 100.;
		cout << "Column: " << col-20 <<   "    Row: " << row-20 << "   Amp: "<< max/2. << endl << endl;
		latexMax.SetTextSize(0.3);				
	      } //color coding

	    if(par_max_amp > 0)
	      ecal_wave_ind[col][row]->SetMaximum(par_max_amp);
	    
	    ecal_wave_ind[col][row]->Draw("histo");
	    
	    if(par_print_thr > 0)
	      latexMax.DrawLatexNDC(xMax,yMax, textMax);
	    
	  }
	}
      } // SOUTH BOTTOM
    
    
	
    my_canvas->Update();
    
    getchar();
    
    my_canvas->Clear();
    
    //Reseting individual histograms    
    for(int  col = 0; col < 40; col++){
      for(int  row = 0; row < 40; row++){
	ecal_wave_ind[col][row]->Reset("ICESM");
      }
    }
    
 
    lockService->RootFillUnLock(this);   
    
    return;
  
    
}

//------------------
// EndRun
//------------------
void JEventProcessor_ecal_display::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_ecal_display::Finish()
{
    // Called before program exit after event processing is finished.
}

