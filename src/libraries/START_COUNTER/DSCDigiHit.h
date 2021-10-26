// $Id$
//
/*! **File**: DSCDigiHit.h
 *+ Created: Tue Aug  6 12:53:36 EDT 2013
 *+ Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
 *+ Purpose: Container class holding Start Counter raw data from fADC250
*/
/// \defgroup SCDetector StartCounter Detector Objects

/*! \file DSCDigiHit.h
 * Container class definition for raw data from fADC250(s) reading the start counter
 * the raw data includes ADC and amplituded and integral values as well as pedestal and 
 * timing 
 */


#ifndef _DSCDigiHit_
#define _DSCDigiHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DSCDigiHit:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DSCDigiHit);
  
  int sector;		///< sector number 1-30
  uint32_t pulse_integral; ///< identified pulse integral as returned by FPGA algorithm
  uint32_t pulse_time;     ///< identified pulse time as returned by FPGA algorithm
  uint32_t pedestal;       ///< pedestal info used by FPGA (if any)
  uint32_t QF;             ///< Quality Factor from FPGA algorithms
  uint32_t nsamples_integral;    ///< number of samples used in integral 
  uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
  uint32_t pulse_peak;           ///<  maximum sample in pulse
  
  uint32_t datasource;           ///<  0=window raw data, 1=old(pre-Fall16) firmware, 2=Df250PulseData
  
  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "sector", "%d", sector);
    AddString(items, "pulse_integral", "%d", pulse_integral);
    AddString(items, "pulse_peak", "%d", pulse_peak);
    AddString(items, "pulse_time", "%d", pulse_time);	
    AddString(items, "pedestal", "%d", pedestal);
    AddString(items, "QF", "%d", QF);
    AddString(items, "nsamples_integral", "%d", nsamples_integral);
    AddString(items, "nsamples_pedestal", "%d", nsamples_pedestal);
  }
  
};

#endif // _DSCDigiHit_

