// $Id$
// $HeadURL$
//
//    File: DTSscalers.h
// Created: WTue Mar  8 23:02:17 EST 2016
// Creator: davidl (on Darwin harriet 13.4.0)
//

#ifndef _DTSscalers_
#define _DTSscalers_

#include <JANA/JObject.h>

class DTSscalers:public JObject{
	public:
		JOBJECT_PUBLIC(DTSscalers);
		
		uint32_t nsync_event;
		uint32_t int_count;
		uint32_t live_time;              // in clock counts (integrated)
		uint32_t busy_time;              // in clock counts (integrated)
		uint32_t inst_livetime;          // in percent x10 (instantaneous)
		uint32_t time;                   // unix time in sec
		uint32_t gtp_scalers[32];
		uint32_t fp_scalers[16];
		uint32_t gtp_rate[32];
		uint32_t fp_rate[16];


		void Summarize(JObjectSummary& summary) const override {
			summary.add(live_time, NAME_OF(live_time), "%d");
			summary.add(busy_time, NAME_OF(busy_time), "%d");
			summary.add(0.1*(float)inst_livetime, "inst_livetime", "%4.1f");
			summary.add(time, NAME_OF(time), "%d");
		}

};

#endif // _DTSscalers_

