// $Id$
//
//    File: DEventProcessor_ccal_display.cc
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#include <map>
using namespace std;

#include "DEventProcessor_ccal_display.h"

#include <DANA/DApplication.h>
#include <CCAL/DCCALDigiHit.h>

#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulsePedestal.h>



// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new DEventProcessor_ccal_display());
}
} // "C"


//------------------
// init
//------------------
jerror_t DEventProcessor_ccal_display::init(void)
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
      
      if(ii < 6) column = 6 - ii; 
      else column = 5 - ii;

      if(jj < 6) row = jj - 6; 
      else row = jj - 5;
     
      char title[30];
      sprintf(title,"Peak_%d_%d", column, row);    
      
      sprintf(title,"Waveform_%d_%d", column, row);    
      ccal_wave[index]  = new TProfile(title,title,100,-0.5,99.5,-10.,4096);
      
    }
  }
  
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_ccal_display::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_ccal_display::evnt(JEventLoop *loop, uint64_t eventnumber)
{


	vector<const DCCALDigiHit*> ccal_digihits;

	int draw_cosmic = 0;


	cout << " Event number = " << eventnumber  <<  endl;
       

	loop->Get(ccal_digihits);

	cout << " Number of hits = " << ccal_digihits.size() <<  endl;

	japp->RootWriteLock();

        for(unsigned int ii = 0; ii < ccal_digihits.size(); ii++){
          const DCCALDigiHit *ccal_hit = ccal_digihits[ii];          
          
	  int row     =  ccal_hit->row;
	  int column  =  ccal_hit->column; 

	  int column_tmp, row_tmp;
	  
	  if(column > 0) column_tmp = 5 + column;
	  else column_tmp = 6 + column;
	  
	  if(row > 0) row_tmp = 5 + row;
	  else row_tmp = 6 + row;
	  
	  int index = row_tmp*12 + column_tmp;

	  
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
	  
	    for(int ii = 0; ii < 143; ii++){
	      ccal_wave[ii]->Reset();
	    }

	    japp->RootUnLock();		
	    return NOERROR;
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
	
	for(int ii = 0; ii < 143; ii++){
	  ccal_wave[ii]->Reset();
	}
	

	japp->RootUnLock();
	       
	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DEventProcessor_ccal_display::erun(void)
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_ccal_display::fini(void)
{
	return NOERROR;
}

