// $Id$
//
/*! **File**: DCTOFTDCDigiHit.h
 *+ Created: Thu 05 May 2022 10:15:51 AM EDT
 *+ Creator: Beni (on Darwin harriet.jlab.org 11.4.2 i386)
 *+ Purpose: Container class object holding raw CAEN TDC data for CTOF
*/
/// \addtogroup CTOFDetector


#ifndef _DCTOFTDCDigiHit_
#define _DCTOFTDCDigiHit_

#include <JANA/JObject.h>

/*! \file DCTOFTDCDigiHit.h
 * Container class holding the raw CTOF data from CAEN TDCs.
 */


// the follwing is for doxygen describing the class
/*! \class DCTOFTDCDigiHit 
 * This is a container class holding raw data from CTOF CAEN TDC
 * the data from the TDC modules are read and written to this container by the reader/translator code
 */
class DCTOFTDCDigiHit:public JObject{
 public:
  JOBJECT_PUBLIC(DCTOFTDCDigiHit);
  
  int plane;      ///< plane (0: vertical)
  int bar;        ///< paddle number 1,2,3,4
  int end;        ///< top/bottom 0/1
  uint32_t time;  ///< hit time
  
  // This method is used primarily for pretty printing
  /// standard method used by hd_dump to print this container class data.
  void Summarize(JObjectSummary& summary) const override{
    summary.add(bar, "bar", "%d");
    summary.add(plane, "plane", "%d");
    summary.add(end, "end", "%d");
    summary.add(time, "time", "%d");
  }
  
};

#endif // _DCTOFTDCDigiHit_

