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
#include <JANA/JFactory.h>

/*! \file DCTOFTDCDigiHit.h
 * Container class holding the raw CTOF data from CAEN TDCs.
 */


// the follwing is for doxygen describing the class
/*! \class DCTOFTDCDigiHit 
 * This is a container class holding raw data from CTOF CAEN TDC
 * the data from the TDC modules are read and written to this container by the reader/translator code
 */
class DCTOFTDCDigiHit:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DCTOFTDCDigiHit);
  
  int plane;      ///< plane (0: vertical)
  int bar;        ///< paddle number 1,2,3,4
  int end;        ///< top/bottom 0/1
  uint32_t time;  ///< hit time
  
  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  ///\fn void toStrings(vector<pair<string,string> > &items)const
  /// standard method used by hd_dump to print this container class data.
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "bar", "%d", bar);
    AddString(items, "plane", "%d", plane);
    AddString(items, "end", "%d", end);
    AddString(items, "time", "%d", time);
  }
  
};

#endif // _DCTOFTDCDigiHit_

