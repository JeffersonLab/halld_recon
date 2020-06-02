// $Id$
//
//    File: DDIRCTDCDigiHit.h
//

#ifndef _DDIRCTDCDigiHit_
#define _DDIRCTDCDigiHit_

#include <JANA/JObject.h>

class DDIRCTDCDigiHit : public JObject{
	public:
		JOBJECT_PUBLIC(DDIRCTDCDigiHit);
		
		int channel;
		uint32_t edge;         ///< 0=leading edge  1=trailing edge
		uint32_t time;         ///< 16 bit relative to beginning of defined readout window

		void Summarize(JObjectSummary& summary) const override {
			summary.add(channel, NAME_OF(channel), "%d");
			summary.add(time, NAME_OF(time), "%d");
			summary.add(edge, NAME_OF(edge), "%d");
		}
};

#endif // _DDIRCTDCDigiHit_

