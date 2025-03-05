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
  bool pattern_sync;  //From helicity signals (rtfm)
  bool t_settle;      //
  bool helicity;      //
  bool pair_sync;     //
  bool ihwp;          // from EPICS variable CCDB or other (0=IN and 1=OUT)
  bool beam_on;       // from EPICS variable CCDB of other (0=OFF and 1=ON)
  bool real_hel;      // actual helicity decoded in the factory (0 = -1,  1 = 1)
  bool valid;         // if the above is well defined (0 = invalid, 1 = valid)
  
   
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
  void Summarize(JObjectSummary& summary) const override{
    summary.add(pattern_sync, "pattern sync", "%d");
    summary.add(t_settle,     "t_settle",     "%d");
    summary.add(helicity,     "helicity",     "%d");
    summary.add(pair_sync,    "pair_sync",    "%d");
    summary.add(ihwp,         "ihwp",         "%d");
    summary.add(beam_on,      "beam_on",      "%d");
    summary.add(real_hel,     "real_hel",     "%d");
    summary.add(valid,        "valid",        "%d");
    
  } 
};

#endif // _DBeamHelicity_

