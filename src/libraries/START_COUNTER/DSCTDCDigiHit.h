// $Id$
//
/*! **File**: DSCTDCDigiHit.h
 *+ Created: Tue Aug  6 13:02:22 EDT 2013
 *+ Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
 *+ Purpose: Container class holding raw data for the Start Counter from F1TDC
*/
/// \addtogroup SCDetector

/*! \file DSCTDCDigiHit.h
 * Containter class definition for the TDC raw data hits of the start counter detector 
 * from F1TDC data
 */



#ifndef _DSCTDCDigiHit_
#define _DSCTDCDigiHit_

#include <JANA/JObject.h>

class DSCTDCDigiHit:public JObject{
 public:
  JOBJECT_PUBLIC(DSCTDCDigiHit);
  
  // Add data members here. For example:
  int sector;		///< sector number 1-30
  uint32_t time; ///< time 
  
  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  /// \fn   void Summarize(JObjectSummary& summary)const
  /// print method used by hd_dump to list all Start Counter raw TDC hits
  void Summarize(JObjectSummary& summary)const{
    summary.add(sector, "sector", "%d");
    summary.add(time, "time", "%d");
  }
  
};

#endif // _DSCTDCDigiHit_

