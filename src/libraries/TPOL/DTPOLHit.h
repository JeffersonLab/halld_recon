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
#include <JANA/JFactory.h>

class DTPOLHit:public jana::JObject{
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
  
  /// \fn   void toStrings(vector<pair<string,string> > &items)const
  /// used by hd_dump to print all TPOL Sector Hit object data for an event.
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "sector", "%d", sector);
    AddString(items, "phi", "%3.3f", phi);
    AddString(items, "ring", "%d", ring);
    AddString(items, "theta", "%3.3f", theta);
    AddString(items, "nsamples", "%d", nsamples);
    AddString(items, "w_samp1", "%d", w_samp1);
    AddString(items, "w_min", "%d", w_min);
    AddString(items, "w_max", "%d", w_max);
    AddString(items, "integral", "%d", integral);
    AddString(items, "pulse_peak", "%3.3f", pulse_peak);
    AddString(items, "dE_proxy", "%3.3f", dE_proxy);
    AddString(items, "t_proxy", "%3.3f", t_proxy);
    AddString(items, "dE", "%3.3f", dE);
    AddString(items, "t", "%3.3f", t);
  }
};

#endif // _DTPOLHit_
