// $Id$
//
//    File: JEventProcessor_led_monitor.cc
// Created: Mon Jun  2 03:58:40 PM EDT 2025
// Creator: somov (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_led_monitor.h"
using namespace std;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_led_monitor());
}
} // "C"


//------------------
// JEventProcessor_led_monitor (Constructor)
//------------------
JEventProcessor_led_monitor::JEventProcessor_led_monitor()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_led_monitor (Destructor)
//------------------
JEventProcessor_led_monitor::~JEventProcessor_led_monitor()
{
}

//------------------
// Init
//------------------
void JEventProcessor_led_monitor::Init()
{
    // This is called once at program startup. 

    auto app = GetApplication();
    // lockService should be initialized here like this
    lockService = app->GetService<JLockService>();
    
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

    htime          =  new TH1F("time","time", 100, -0.5, 99.5);
  
    htrig_bit      =  new TH1F("trig_bit","trig_bit", 32, 0.5, 32.5);
    htrig_bit_fp   =  new TH1F("trig_bit_fp","trig_bit_fp", 32, 0.5, 32.5);
  
    for(int ii = 0; ii < 40; ii++){
      for(int jj = 0; jj < 40; jj++){
	char name[100];
	sprintf(name,"%i_%i",ii,jj);
	ecal_wave_ind[ii][jj] = new TH1F(name,name,1000,0,2000);
	
      }
    }
    

    
}

//------------------
// BeginRun
//------------------
void JEventProcessor_led_monitor::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_led_monitor::Process(const std::shared_ptr<const JEvent> &event)
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
    // lockService->RootFillLock(this);
    //  ... fill historgrams or trees ...
    // lockService->RootFillUnLock(this);

    vector<const DECALDigiHit *> ecal_digi_hits; 
    vector<const DL1Trigger *> l1trig;
  
    event->Get(l1trig);
    event->Get(ecal_digi_hits);
    
    unsigned int trig_bit[33], trig_bit_fp[33];
    
    int adc_time_min  =  35;
    int adc_time_max  =  50;
    
    if( ecal_digi_hits.size() < 1000)
      return;
    
    lockService->RootFillLock(this);
    
    if( l1trig.size() > 0){           
      for(unsigned int bit = 0; bit < 32; bit++){
	trig_bit[bit + 1] = (l1trig[0]->trig_mask & (1 << bit)) ? 1 : 0;       
	if(trig_bit[bit + 1] == 1) htrig_bit->Fill(Float_t(bit+1)); 
      }
      for(unsigned int bit = 0; bit < 32; bit++){
	trig_bit_fp[bit + 1] = (l1trig[0]->fp_trig_mask & (1 << bit)) ? 1 : 0;         
	if(trig_bit_fp[bit + 1] == 1) htrig_bit_fp->Fill(Float_t(bit+1)); 
      }       
    }
    
    if(trig_bit_fp[5] != 1){
      lockService->RootFillUnLock(this);
      return;
    }
    
    
     //    cout << " Number of digi hits = " << ecal_digi_hits.size() << endl;
    
    for(unsigned int ii = 0; ii < ecal_digi_hits.size(); ii++){
      const DECALDigiHit *ecal_hit = ecal_digi_hits[ii];
      
      int row     =  ecal_hit->row;
      int column  =  ecal_hit->column; 
      
      int pulse_peak    =   ecal_hit->pulse_peak;
      int pulse_int     =   ecal_hit->pulse_integral; 
      int pedestal      =   ecal_hit->pedestal;
      int nsamples_pedestal  =  ecal_hit->nsamples_pedestal ;
      int nsamples_integral  = ecal_hit->nsamples_integral;
      
      double peak_ps = (double) pulse_peak - (double)pedestal / nsamples_pedestal;
      double int_ps  = (double) pulse_int - (double) pedestal * (double) nsamples_integral / nsamples_pedestal;
      
      uint32_t  adc_time = (ecal_hit->pulse_time  & 0x7FC0) >> 6;
      
      if((column == 1) && (row == 28))
	htime->Fill(float(adc_time));
      
      
#if 1
      const Df250WindowRawData *windorawdata;
      const Df250PulseData     *pulsedata; 
      
      ecal_hit->GetSingle(pulsedata);         
      pulsedata->GetSingle(windorawdata);
      
      if(pulsedata){
	
	unsigned int max_amp = 0;
	
	if(windorawdata){       
	  
	  const vector<uint16_t> &samplesvector = windorawdata->samples;          
	  unsigned int nsamples = samplesvector.size();
	  
	  for(uint16_t samp = 0; samp < nsamples; samp++){
	    
	    unsigned int adc_amp = samplesvector[samp];
	    
	    if (adc_amp > max_amp) max_amp = adc_amp;	   
	    
	  }  // Loop over samples
	  
	  peak_ps = (double)max_amp - 100.;
	  
	  ecal_wave_ind[column][row]->Fill(peak_ps);	 	  
	}
	
      }
#endif
      
      if( (adc_time > adc_time_min) && (adc_time < adc_time_max))      
	ecal_wave_ind[column][row]->Fill(peak_ps);	 
      
    }  // Loop over hits
    
    lockService->RootFillUnLock(this);
    
    
    
}

//------------------
// EndRun
//------------------
void JEventProcessor_led_monitor::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_led_monitor::Finish()
{
    // Called before program exit after event processing is finished.
}

