// $Id$
//
//    File: DEventProcessor_ccal_display.cc
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#include <map>
using namespace std;

#include "DEventProcessor_ccal_display.h"

#include <DANA/DEvent.h>
#include <CCAL/DCCALDigiHit.h>

#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulsePedestal.h>



// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new DEventProcessor_ccal_display());
}
} // "C"


//------------------
// Init
//------------------
void DEventProcessor_ccal_display::Init()
{
  	
  gStyle->SetOptStat(0);
    
  gStyle->SetTitleFontSize(0.07);
  
  gStyle->SetLineWidth(1.5);
  gStyle->SetTextSize(1.5);
  
  gStyle->SetTitleFont(130,"xy");
  gStyle->SetLabelFont(130,"xy");
   
  gStyle->SetLineWidth(1.5);
  gStyle->SetTextSize(1.5);
  

  gStyle->SetLabelSize(0.09,"xy");
  gStyle->SetTitleSize(0.09,"xy");
  

  gStyle->SetTitleOffset(1.,"x");
  gStyle->SetTitleOffset(0.6,"y");
  
  gStyle->SetTitleBorderSize(0);

  app1 =  new TApplication("app1",NULL,0);
  
  my_canvas  = new TCanvas("CCAL ", "CCAL ",1000, 700,  1500, 1000);
 
  
  my_canvas->Divide(12,12,0.001);
   

  for(Int_t ii = 0; ii < 12; ii++){
    for(Int_t jj = 0; jj < 12; jj++){
      
      int index = ii*12 + jj;
      
      int column, row;

      if(jj < 6) column = jj - 6; 
      else column = jj - 5;

      if(ii < 6) row = ii - 6; 
      else row = ii - 5;
     
      char title[30];
      sprintf(title,"Peak_%d_%d", column, row);    
      
      sprintf(title,"Waveform_%d_%d", column, row);    
      ccal_wave[index]  = new TProfile(title,title,100,-0.5,99.5,-10.,4096);
      
    }
  }
}

//------------------
// BeginRun
//------------------
void DEventProcessor_ccal_display::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DEventProcessor_ccal_display::Process(const std::shared_ptr<const JEvent>& event)
{


	vector<const DCCALDigiHit*> ccal_digihits;

	int draw_cosmic = 1;


	cout << " Event number = " << event->GetEventNumber()  <<  endl;
       

	event->Get(ccal_digihits);

	cout << " Number of hits = " << ccal_digihits.size() <<  endl;

	DEvent::GetLockService(event)->RootWriteLock();

        for(unsigned int ii = 0; ii < ccal_digihits.size(); ii++){
          const DCCALDigiHit *ccal_hit = ccal_digihits[ii];          
          
	  int row     =  ccal_hit->row;
	  int column  =  ccal_hit->column; 

	  
	  int index = row*12 + column;

	  
          const Df250WindowRawData *windorawdata;
	  const Df250PulseData     *pulsedata; 
	  
	  
          ccal_hit->GetSingle(pulsedata);	  
	  pulsedata->GetSingle(windorawdata);
	  
	  
	  if(pulsedata){
	    if(windorawdata){	    
	    
	      const vector<uint16_t> &samplesvector = windorawdata->samples;
	      
	      unsigned int nsamples = samplesvector.size();
	      
	      for(uint16_t samp = 0; samp < nsamples; samp++){
		
		ccal_wave[index]->Fill(float(samp),float(samplesvector[samp]));
		
	      }     
	    }
	  }             
	}


	if(draw_cosmic == 1){
	  if( (ccal_digihits.size() > 30) || (ccal_digihits.size() == 0)){
	  
	    for(int ii = 0; ii < 144; ii++){
	      ccal_wave[ii]->Reset();
	    }

	    DEvent::GetLockService(event)->RootUnLock();
	    return;
	  }
	}


	for(int ii = 143; ii >= 0; ii--){
	  
	  int index = 144 - ii;
	  
	  my_canvas->cd(index);
	  
	  gPad->SetBottomMargin(0.19);
	  gPad->SetLeftMargin(0.15);
	  ccal_wave[ii]->Draw("histo");	    
	}

	
	my_canvas->Update();
	
	cout << " --------  PRESS KEY  ----------- "  <<  endl;
	
	getchar();
        
	my_canvas->Clear();
	my_canvas->Divide(12,12,0.001);
	
	for(int ii = 0; ii < 144; ii++){
	  ccal_wave[ii]->Reset();
	}
	

	DEvent::GetLockService(event)->RootUnLock();
}

//------------------
// EndRun
//------------------
void DEventProcessor_ccal_display::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// Finish
//------------------
void DEventProcessor_ccal_display::Finish()
{
}

