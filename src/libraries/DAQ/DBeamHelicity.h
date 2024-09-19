// $Id$
//
//    File: DBeamHelicity.h
//

#ifndef _DBeamHelicity_
#define _DBeamHelicity_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DBeamHelicity:public JObject{
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
  void Summarize(JObjectSummary& summary) const override{
    summary.add(pattern_sync, "pattern sync", "%d");
    summary.add(t_settle, "t_settle",     "%d");
    summary.add(helicity, "helicity",     "%d");
    summary.add(pair_sync, "pair_sync",    "%d");
    summary.add(ihwp, "ihwp",         "%d");
    summary.add(beam_on, "beam_on",      "%d");
  } 
};

#endif // _DBeamHelicity_

