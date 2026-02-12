// $Id$
//
/*! **File**: DSCHit.h
 *+ Created: Wed Feb  7 10:46:20 EST 2007
 *+ Creator: davidl (on Linux megrez.jlab.org 2.6.9-42.0.2.ELsmp x86_64)
 *+ Purpose: Container class for Start Counter holding data from matched ADC and TDC hits
 * with energies converted to GeV and times converted to ns
*/

/// \addtogroup SCDetector

/*! \file DSCHit.h
 * Container class holding the Start Counter hit data from ADC and TDC converted
 * to Energy (GeV) and time ns.
 */


#ifndef _DSCHit_
#define _DSCHit_

#include <JANA/JObject.h>

class DSCHit:public JObject{
 public:
  JOBJECT_PUBLIC(DSCHit);
  
  int sector;	    ///< sector number 1-30
  float dE;       ///< Energy loss in GeV
  float t;        ///< best time (walk-corrected tdc)
  float t_TDC;   ///< time from TDC in ns, no walk correction, but global offset applied
  float t_fADC; ///< time from fADC in ns, global offset applied
  float pulse_height; ///< signal amplitude in ADC pulses (used in time-walk corrections)
  bool has_fADC;  ///< boolbean 
  bool has_TDC;   ///< booblean

  /// \vn   void toStrings(vector<pair<string,string> > &items)const
  /// method used by hd_dump to print this Start Counter hit data.  
  void Summarize(JObjectSummary& summary) const {
    summary.add(sector, "sector", "%d");
    summary.add(dE, "dE", "%3.3f");
    summary.add(t, "t", "%3.3f");
    summary.add(t_TDC, "t_TDC","%3.3f");
    summary.add(t_fADC, "t_fADC", "%3.3f");
    summary.add((int)has_fADC, "has_fADC", "%d");
    summary.add((int)has_TDC, "has_TDC", "%d");
  }
};

#endif // _DSCHit_

