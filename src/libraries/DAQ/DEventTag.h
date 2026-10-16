// $Id$
//
//    File: DEventTag.h
// Created: Fri Dec  4 10:14:22 EST 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _DEventTag_
#define _DEventTag_


#include <TRIGGER/DL3Trigger.h>

class DEventTag:public JObject{
	public:
		JOBJECT_PUBLIC(DEventTag);

		DEventTag(uint64_t es=0L, DL3Trigger::L3_decision_t d=DL3Trigger::kNO_DECISION, uint64_t l3s=0, uint32_t l3a=0, double mva=3E-6)
			:event_status(es),L3_decision(d),L3_status(l3s),L3_algorithm(l3a),mva_encoded(mva){
				mva_response = ((double)(mva_encoded & 0x7FFFFFFF))/1000.0;
				if( mva_encoded & 0x80000000 ) mva_response = -mva_response;
			}
		
		uint64_t event_status;                   ///< JANA event status word when event was written
		DL3Trigger::L3_decision_t L3_decision;   ///< L3 decision when event was written
		uint64_t L3_status;                      ///< L3 status word when event was written
		uint32_t L3_algorithm;                   ///< L3 algorithm identifier when event was written
		double   mva_response;                   ///< L3 MVA response (if any)
		uint32_t mva_encoded;                    ///< same as above but encoded as unsigned int (for debugging)
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			summary.add(event_status, NAME_OF(event_status), "0x%x");
			summary.add(L3_decision, NAME_OF(L3_decision), "%d");
			summary.add(L3_status, NAME_OF(L3_status), "0x%016x");
			summary.add(L3_algorithm, NAME_OF(L3_algorithm), "0x%08x");
			summary.add(mva_response, "L3_mva_response", "%5.3f");
		}

};

#endif // _DEventTag_

