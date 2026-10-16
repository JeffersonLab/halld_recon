//
/*! **File**: DTPOLHit.h
 *+ Created: unknown
 *+ Creator: unknown
 *+ Purpose: Container class holdinig the TPOL hit data based on input from DTPOLSectorDigiHit
 * and DTPOLRingHit objects. The data is converted from ADC counts to energy and time counts
 * to ns based on calibration data from CCDB.
*/

/// \addtogroup TPOLDetector

/*! \file DTPOLHit.h
 * container class holding TPOL  raw data from silicon disc detector.
 */


#ifndef _DTPOLHit_
#define _DTPOLHit_

#include <JANA/JObject.h>

class DTPOLHit:public JObject{
 public:
  JOBJECT_PUBLIC(DTPOLHit);
  
  int sector;    // sector number 1-32
  double phi;
  int ring;      // ring number 1-24
  double theta;
  unsigned int nsamples; ///< number of samples in the readout
  unsigned int w_samp1; ///< ADC value of first sample in the window
  unsigned int w_min;   ///< smalles ADC value in the readout window
  unsigned int w_max;   ///< largest ADC value in the readout window
  unsigned int integral; ///< Interal provided by the FPGA alorithm
  double pulse_peak;     ///< Energy loss in keV
  double dE_proxy;       ///< ?
  double t_proxy;        ///< ?
  double dE;             ///< ?
  double t;              ///< ?
  
  /// \fn   void Summarize(JObjectSummary& summary)const
  /// used by hd_dump to print all TPOL Sector Hit object data for an event.
  void Summarize(JObjectSummary& summary)const{
    summary.add(sector, "sector", "%d");
    summary.add(phi, "phi", "%3.3f");
    summary.add(ring, "ring", "%d");
    summary.add(theta, "theta", "%3.3f");
    summary.add(nsamples, "nsamples", "%d");
    summary.add(w_samp1, "w_samp1", "%d");
    summary.add(w_min, "w_min", "%d");
    summary.add(w_max, "w_max", "%d");
    summary.add(integral, "integral", "%d");
    summary.add(pulse_peak, "pulse_peak", "%3.3f");
    summary.add(dE_proxy, "dE_proxy", "%3.3f");
    summary.add(t_proxy, "t_proxy", "%3.3f");
    summary.add(dE, "dE", "%3.3f");
    summary.add(t, "t", "%3.3f");
  }
};

#endif // _DTPOLHit_
