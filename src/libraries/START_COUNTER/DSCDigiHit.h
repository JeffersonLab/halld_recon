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

class DSCDigiHit:public JObject{
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
  // the third argument to summary.add is printf style format
  void Summarize(JObjectSummary& summary)const{
    summary.add(sector, "sector", "%d");
    summary.add(pulse_integral, "pulse_integral", "%d");
    summary.add(pulse_peak, "pulse_peak", "%d");
    summary.add(pulse_time, "pulse_time", "%d");
    summary.add(pedestal, "pedestal", "%d");
    summary.add(QF, "QF", "%d");
    summary.add(nsamples_integral, "nsamples_integral", "%d");
    summary.add(nsamples_pedestal, "nsamples_pedestal", "%d");
  }
  
};

#endif // _DSCDigiHit_

