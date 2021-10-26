// $Id$
//
/*! **File**: DTOFTDCDigiHit.h
 *+ Created: Wed Aug  7 09:31:00 EDT 2013
 *+ Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
 *+ Purpose: Container class object holding raw CAEN TDC data for TOF
*/
/// \addtogroup TOFDetector


#ifndef _DTOFTDCDigiHit_
#define _DTOFTDCDigiHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

/*! \file DTOFTDCDigiHit.h
 * Container class holding the raw TOF data from CAEN TDCs.
 */


// the follwing is for doxygen describing the class
/*! \class DTOFTDCDigiHit 
 * This is a container class holding raw data from TOF CAEN TDC
 * the data from the TDC modules are read and written to this container by the reader/translator code
 */
class DTOFTDCDigiHit:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DTOFTDCDigiHit);
  
  int plane;      ///< plane (1: vertical, 0: horizontal)
  int bar;        ///< bar number
  int end;        ///< left/right 0/1 or North/South 0/1
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

#endif // _DTOFTDCDigiHit_

