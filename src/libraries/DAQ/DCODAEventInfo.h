// $Id$
// $HeadURL$
//
//    File: DCODAEventInfo.h
// Created: Wed Oct 15 11:35:43 EDT 2014
// Creator: davidl (on Darwin harriet.local 13.3.0 i386)
//

#ifndef _DCODAEventInfo_
#define _DCODAEventInfo_

#include <JANA/JObject.h>

class DCODAEventInfo:public JObject{
	public:
		JOBJECT_PUBLIC(DCODAEventInfo);
		
		uint32_t run_number;
		uint32_t run_type;
		uint64_t event_number;
		uint16_t event_type;
		uint64_t avg_timestamp;
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			summary.add(run_number, NAME_OF(run_number), "%d");
			summary.add(run_type, NAME_OF(run_type), "%d");
			summary.add(event_number, NAME_OF(event_number), "%ld");
			summary.add(event_type, NAME_OF(event_type), "%d");
			summary.add(avg_timestamp, NAME_OF(avg_timestamp), "%ld");
		}
		
};

#endif // _DCODAEventInfo_

