// $Id$
//
//    File: DDIRCLEDRef_factory.cc
// Created: Wed Aug  7 09:30:17 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//


#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <limits>

#include <TMath.h>

using namespace std;

#include "DDIRCLEDRef_factory.h"

#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250Config.h>
#include <DAQ/DCODAROCInfo.h>

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>



//------------------
// Init
//------------------
void DDIRCLEDRef_factory::Init()
{
  /// Set basic conversion constants
  a_scale    = 0.2/5.2E5;
  t_scale    = 0.0625;   // 62.5 ps/count
  t_base     = 0.;       // ns
  t_base_tdc = 0.; // ns
}

//------------------
// BeginRun
//------------------
void DDIRCLEDRef_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DDIRCLEDRef_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  /// Generate DDIRCLEDRef object for each DTOFDigiHit object.
  /// This is where the first set of calibration constants
  /// is applied to convert from digitzed units into natural
  /// units.
  ///
  /// Note that this code does NOT get called for simulated
  /// data in HDDM format. The HDDM event source will copy
  /// the precalibrated values directly into the _data vector.

  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto calibration = app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);

  const DTTabUtilities* locTTabUtilities = NULL;
  event->GetSingle(locTTabUtilities);
 
  vector<const DCAEN1290TDCHit*> sipmtdchits;
  event->Get(sipmtdchits);
  vector<const Df250PulseData*> sipmadchits;
  event->Get(sipmadchits);

  tdc_time_offset = 0.0;
  adc_time_offset = 0.0;

  map<string,double> led_ref_time_offset;
  
  if (calibration->Get("DIRC/led_ref_time_offset", led_ref_time_offset)) 
	  jout << "Error loading DIRC SiPM reference timing table !" << jendl;
  
  if (led_ref_time_offset.find("ADC_time_offset") != led_ref_time_offset.end())
	  adc_time_offset = led_ref_time_offset["ADC_time_offset"];
  else
	  jout << "Error loading DIRC SiPM reference timing table !" << jendl;
  
  if (led_ref_time_offset.find("TDC_time_offset") != led_ref_time_offset.end())
	  tdc_time_offset = led_ref_time_offset["TDC_time_offset"];
  else
	  jout << "Error loading DIRC SiPM reference timing table !" << jendl;

  //cout << "Timing: " << adc_time_offset << "    " << tdc_time_offset  << endl;

            for(uint i=0; i<sipmadchits.size(); i++) {
                    const Df250PulseData* sipmadchit = (Df250PulseData*)sipmadchits[i];
                    if(sipmadchit->rocid == 77 && sipmadchit->slot == 16 && sipmadchit->channel == 15) {
                            double T = (double)((sipmadchit->course_time<<6) + sipmadchit->fine_time);
			    
			    // Apply calibration constants here
			    T =  t_scale * T + adc_time_offset;
			
			    DDIRCLEDRef *hit = new DDIRCLEDRef;
			    hit->amp = (double)sipmadchit->pulse_peak;
			    hit->integral = (double)sipmadchit->integral;
			
			    hit->t_TDC=numeric_limits<double>::quiet_NaN();
			    hit->t_fADC=T;
			    hit->t = hit->t_fADC;  // set initial time to the ADC time, in case there's no matching TDC hit
			
			    hit->has_fADC=true;
			    hit->has_TDC=false;

			    hit->AddAssociatedObject(sipmadchit);

			    Insert(hit);
		    }
            }

            for(uint i=0; i<sipmtdchits.size(); i++) {
                    const DCAEN1290TDCHit* sipmtdchit = (DCAEN1290TDCHit*)sipmtdchits[i];
                    if(sipmtdchit->rocid == 78 && sipmtdchit->slot == 8 && sipmtdchit->channel == 30) {

			    // Apply calibration constants here
			    double T = locTTabUtilities->Convert_DigiTimeToNs_CAEN1290TDC(sipmtdchit) + tdc_time_offset;

			    // Look for existing hits to see if there is a match
      			    // or create new one if there is no match
		            DDIRCLEDRef *hit = FindMatch(T);
      			    if(!hit){
				continue; // Do not use unmatched TDC hits
      			    } else if (hit->has_TDC) { // this tof ADC hit has already a matching TDC, make new tof ADC hit
	   	         	DDIRCLEDRef *newhit = new DDIRCLEDRef;
				newhit->amp = hit->amp;
				newhit->integral = hit->integral;
				newhit->t_fADC = hit->t_fADC;
				newhit->has_fADC = hit->has_fADC;
				newhit->t_TDC=numeric_limits<double>::quiet_NaN();
				newhit->t = hit->t_fADC;  // set initial time to the ADC time, in case there's no matching TDC hit	
				newhit->has_TDC=false;
				newhit->AddAssociatedObject(sipmtdchit);
				Insert(newhit);
				hit = newhit;
			    }
		      	    hit->has_TDC=true;
	                    hit->t_TDC=T;
			    hit->t = T;
     
/* 
      if (hit->dE>0.){

	// time walk correction
	// Note at this point the dE value is still in ADC units
	double tcorr = 0.;
	if (USE_AMP_4WALKCORR) {
	  // use amplitude instead of integral
	  tcorr = CalcWalkCorrAmplitude(hit);

	} else if (USE_NEW_4WALKCORR) {
	  // new functional form with 4 parameter but still using integral
	  tcorr = CalcWalkCorrNEW(hit);

	} else if (USE_NEWAMP_4WALKCORR) {
	  // new functional form with 2 functions and 4 parameter using amplitude
 	  tcorr = CalcWalkCorrNEWAMP(hit);

	} else {
	  // use integral
	  tcorr = CalcWalkCorrIntegral(hit);

	}
	
	T -= tcorr;
      }
      hit->t=T;
*/    
     			    hit->AddAssociatedObject(sipmtdchit);
    		    }
	    }
}

//------------------
// FindMatch
//------------------
DDIRCLEDRef* DDIRCLEDRef_factory::FindMatch(double T)
{
    DDIRCLEDRef* best_match = NULL;

    // Loop over existing hits (from fADC) and look for a match
    // in both the sector and the time.
    for(unsigned int i=0; i<mData.size(); i++){
        DDIRCLEDRef *hit = mData[i];

        if(!isfinite(hit->t_fADC)) continue; // only match to fADC hits, not bachelor TDC hits

        //double delta_T = fabs(hit->t - T);
        double delta_T = fabs(T - hit->t);

        // if there are multiple hits, pick the one that is closest in time
        if(best_match != NULL) {
            if(delta_T < fabs(best_match->t - T))
                best_match = hit;
        } else {
            best_match = hit;
        }

    }

    return best_match;
}

//------------------
// EndRun
//------------------
void DDIRCLEDRef_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DDIRCLEDRef_factory::Finish()
{
}


