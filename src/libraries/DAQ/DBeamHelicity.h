// $Id$
//
//    File: DBeamHelicity.h
//

#ifndef _DBeamHelicity_
#define _DBeamHelicity_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DBeamHelicity:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DBeamHelicity);
  
  // Need to add notes on meanings (Mark?)
  bool pattern_sync; 
  bool t_settle;
  bool helicity;
  bool pair_sync;
  bool ihwp;    // from EPICS variable (0=IN and 1=OUT)
  bool beam_on; // from EPICS variable (0=OFF and 1=ON)
  
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "pattern sync", "%d", pattern_sync);
    AddString(items, "t_settle",     "%d", t_settle);
    AddString(items, "helicity",     "%d", helicity);
    AddString(items, "pair_sync",    "%d", pair_sync);
    AddString(items, "ihwp",         "%d", ihwp);
    AddString(items, "beam_on",      "%d", beam_on);
  } 
};

#endif // _DBeamHelicity_

