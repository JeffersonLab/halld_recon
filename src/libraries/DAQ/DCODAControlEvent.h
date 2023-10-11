// $Id$
// $HeadURL$
//
//    File: DCODAControlEvent.h
// Created: Sat Dec 16 07:55:18 EST 2017
// Creator: davidl (on Darwin harriet.local 13.3.0 i386)
//

#ifndef _DCODAControlEvent_
#define _DCODAControlEvent_

#include <JANA/JObject.h>

class DCODAControlEvent:public JObject{
	public:
		JOBJECT_PUBLIC(DCODAControlEvent);
		
		uint16_t event_type;
		uint32_t unix_time;
		std::vector<uint32_t> words;
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			summary.add(event_type, NAME_OF(event_type), "%04x");
			summary.add(unix_time, NAME_OF(unix_time), "%ld");
			summary.add(words.size(), "Nwords", "%d");
		}
		
};

#endif // _DCODAControlEvent_

