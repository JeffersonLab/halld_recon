// $Id$
//
//    File: DCTOFHit_factory.cc
// Created: Fri Mar 18 09:00:12 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//
/// This code combines the raw digitisations from the CAEN TDC and the FADC 
/// for the CPP paddles into DCTOFHit objects.

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include "DCTOFHit_factory.h"
#include "DCTOFDigiHit.h"
#include "DCTOFTDCDigiHit.h"
#include <TTAB/DTTabUtilities.h>
#include <DANA/DEvent.h>


//------------------
// Init
//------------------
void DCTOFHit_factory::Init()
{
  auto app = GetApplication();
  DELTA_T_ADC_TDC_MAX = 20.0; // ns
  app->SetDefaultParameter("CTOF:DELTA_T_ADC_TDC_MAX", DELTA_T_ADC_TDC_MAX, 
			      "Maximum difference in ns between a (calibrated) fADC time and CAEN TDC time for them to be matched in a single hit");

  /// Set basic conversion constants
  t_scale    = 0.0625;   // 62.5 ps/count
  t_base_adc = 0.;       // ns
  t_base_tdc = 0.; // ns

  INSTALLED = false;
    
}

//------------------
// BeginRun
//------------------
void DCTOFHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	map<string,string> installed;
	DEvent::GetCalib(event, "/CTOF/install_status", installed);
	if(atoi(installed["status"].data()) == 0)
		INSTALLED = false;
	else
		INSTALLED = true;
		
	if(!INSTALLED) return;

  // load base time offset
  map<string,double> base_time_offset;
  if ((GetCalib(event,"/CTOF/adc_base_time_offset",base_time_offset))==false){
    t_base_adc = base_time_offset["t0"];
  }
  if ((GetCalib(event,"/CTOF/tdc_base_time_offset",base_time_offset))==false){
    t_base_tdc = base_time_offset["t0"];
  }

  // Channel-by-channel timing offsets
  GetCalib(event, "/CTOF/adc_timing_offsets", adc_time_offsets);  
  GetCalib(event, "/CTOF/tdc_timing_offsets", tdc_time_offsets);
  GetCalib(event, "/CTOF/adc2E", adc2E);
}

//------------------
// Process
//------------------
void DCTOFHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{   
  if(!INSTALLED) return;

  const DTTabUtilities* locTTabUtilities = NULL;
  event->GetSingle(locTTabUtilities);
  
  // Get FADC hits
  vector<const DCTOFDigiHit*> digihits;
  event->Get(digihits);
   //Get the TDC hits
  vector<const DCTOFTDCDigiHit*> tdcdigihits;
  event->Get(tdcdigihits);
  
  for(unsigned int i=0;i<digihits.size(); i++){
    const DCTOFDigiHit *digihit = digihits[i];

    for (unsigned int j=0;j<tdcdigihits.size();j++){
      const DCTOFTDCDigiHit *tdcdigihit = tdcdigihits[j];
 
      if (tdcdigihit->bar!=digihit->bar || tdcdigihit->end!=digihit->end) 
	continue;

      // pulse pedestal
      double pedestal=0.;
      //double pedestal4Amp=0.;
      uint32_t nsamples_integral = digihit->nsamples_integral;
      uint32_t nsamples_pedestal = digihit->nsamples_pedestal;

      if  (digihit->pedestal>0
	   && locTTabUtilities->CheckFADC250_PedestalOK(digihit->QF) ) {
	pedestal = digihit->pedestal
	  * double(nsamples_integral)/double(nsamples_pedestal);
	//pedestal4Amp = digihit->pedestal/double(nsamples_pedestal);
      }

      // pulse integral
      double A = (double)digihit->pulse_integral;
      double dA = A - pedestal;
      if (dA<0.) continue;

      // Time
      int ind=digihit->bar-1 + 4.*digihit->end;
      double T = locTTabUtilities->Convert_DigiTimeToNs_CAEN1290TDC(tdcdigihit);
      T += t_base_tdc + tdc_time_offsets[ind];
      double Tadc = t_scale*double(digihit->pulse_time) + adc_time_offsets[ind] + t_base_adc;
      double delta_T = fabs(T - Tadc);

      if(delta_T > DELTA_T_ADC_TDC_MAX) continue;
      
      DCTOFHit *hit = new DCTOFHit;
      hit->bar   = digihit->bar;
      hit->end   = digihit->end;
      hit->t     = T;
      hit->t_adc = Tadc;
      hit->dE    = adc2E[ind]*dA;

      hit->AddAssociatedObject(digihit);
      hit->AddAssociatedObject(tdcdigihit);

      Insert(hit);
    }
  }
}

//------------------
// EndRun
//------------------
void DCTOFHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DCTOFHit_factory::Finish()
{
}

