//
/*! **File**: DTPOLRingDigiHit.h
 *+ Created: unknown
 *+ Creator: unknown
 *+ Purpose: Container class holdinig the TPOL raw data from fADC250 digitizing the signals
 * from the silicon disc detector segmented radially along r. The detector has 24 rings but,
 * as of now (2021/10/26), is not instrumented with any readout system.
*/

/// \addtogroup TPOLDetector

/*! \file DTPOLRingDigiHit.h
 * Container class hodling raw data from TPOL detector silicon of the ring segmentation.
 * The physical readout of this part of the detector is currently not implemented.
 */


#ifndef _DTPOLRingDigiHit_
#define _DTPOLRingDigiHit_

#include <JANA/JObject.h>

class DTPOLRingDigiHit:public JObject{
 public:
  JOBJECT_PUBLIC(DTPOLRingDigiHit);
  
  
  int      ring;                 ///< ring number 1-24
  uint32_t pulse_integral;       ///< identified pulse integral as returned by FPGA algorithm
  uint32_t pulse_time;           ///< identified pulse time as returned by FPGA algorithm
  uint32_t pedestal;             ///< pedestal info used by FPGA (if any)
  uint32_t QF;                   ///< Quality Factor from FPGA algorithms
  uint32_t nsamples_integral;    ///< number of samples used in integral 
  uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
  
  // This method is used primarily for pretty printing
  // the second argument to summary.add is printf style format
  /// \fn   void Summarize(JObjectSummary& summary) const
  /// used by hd_dump to print all TPOL Sector Hit object data for an event.
  void Summarize(JObjectSummary& summary) const{
    summary.add(ring, "ring", "%d");
    summary.add(pulse_integral, "pulse_integral", "%d");
    summary.add(pulse_time, "pulse_time", "%d");
    summary.add(pedestal, "pedestal", "%d");
    summary.add(QF, "QF", "%d");
    summary.add(nsamples_integral, "nsamples_integral", "%d");
    summary.add(nsamples_pedestal, "nsamples_pedestal", "%d");
  }
  
};

#endif // _DTPOLRingDigiHit_
