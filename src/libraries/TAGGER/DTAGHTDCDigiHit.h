// $Id$
//
//    File: DTAGHTDCDigiHit.h
// Created: Tue Aug  6 13:02:22 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DTAGHTDCDigiHit_
#define _DTAGHTDCDigiHit_

#include <JANA/JObject.h>

class DTAGHTDCDigiHit: public JObject {
   public:
      JOBJECT_PUBLIC(DTAGHTDCDigiHit);
      
      int counter_id;  ///< counter id 1-274
	  uint32_t time;

	  void Summarize(JObjectSummary& summary) const override {
		 summary.add(counter_id, NAME_OF(counter_id), "%d");
		 summary.add(time, NAME_OF(time), "%d");
      }
      
};

#endif // _DTAGHTDCDigiHit_

