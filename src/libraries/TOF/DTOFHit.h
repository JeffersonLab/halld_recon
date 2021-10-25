// $Id: DTOFHit.h Tue Jan 18 16:15:26 EST 2011
//
/*! File:    DTOFHit.h
 *+  Created: Tue Jan 18 16:15:26 EST 2011
 *+ Creator: B. Zihlmann
 *+ Purpose: Container class object holding TOF Hit data converted to GeV (ADC) and ns (ADC and TDC)

*/

/// \addtogroup TOFDetector

/*! \file DTOFHit.h container class for collected data for a TOF hit
 * this includes ADC and TDC data in unites of ADC counts, energy in GeV
 * and time in ns.

 */

#ifndef _DTOFHit_
#define _DTOFHit_

#include "JANA/JObject.h"
#include "JANA/JFactory.h"

/*! \class DTOFHit
 * container class for TOF Hits formed from raw ADC and TDC data provided by the objects
 * DTOFDigitHit and DTOFTDCDigiHit. Hits from ADC and TDC data are matched if the timing is
 * adequate. The data is converted from TDC counts to times in units of ns. The ADC information
 * converted to energy [GeV], at this point with no correction for hit location (attenuation not known yet).
 */

class DTOFHit:public jana::JObject{
  
 public:
  JOBJECT_PUBLIC(DTOFHit);
  
  int plane;      ///< plane (0: vertical, 1: horizontal)
  int bar;        ///< bar number: 1,2,3, ..., 44 (46 for TOF2)
  int end;        ///< TOP/BOTTOM 0/1 (vertical paddles) or North/South 0/1 (horizontal paddles)
  float dE;       ///< Energy deposition to be filled later
  float Amp;      ///< Signal max Amplitude, ADC counts
  float t_fADC;   ///< time from adc converted to ns
  float t_TDC;   ///< time from tdc converted to ns
  float t; ///< TDC time corrected for signal walk
  bool has_fADC; ///< boolean to indicate this hit contains ADC information
  bool has_TDC; ///< boolean to indicate this hit contains TDC information
  
  /// \fn void toStrings(vector<pair<string,string> > &items)const
  /// print function used by hd_dump to list all TOF hits based on this objects for an event 
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "bar", "%d", bar);
    AddString(items, "plane", "%d", plane);
    AddString(items, "end", "%d", end);
    AddString(items, "dE", "%12.4f", dE);
    AddString(items, "Amp", "%12.4f", Amp);
    AddString(items, "t", "%12.4f", t);
    AddString(items, "t_TDC","%12.4f",t_TDC);
    AddString(items, "t_fADC","%12.4f",t_fADC);
  }
};

#endif // _DTOFHit_

