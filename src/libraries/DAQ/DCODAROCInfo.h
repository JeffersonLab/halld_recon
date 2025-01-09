// $Id$
// $HeadURL$
//
//    File: DCODAROCInfo.h
// Created: Wed Oct 15 11:35:43 EDT 2014
// Creator: davidl (on Darwin harriet.local 13.3.0 i386)
//

#ifndef _DCODAROCInfo_
#define _DCODAROCInfo_

#include <JANA/JObject.h>

using namespace std;

class DCODAROCInfo:public JObject{
	public:
		JOBJECT_PUBLIC(DCODAROCInfo);
		
		uint32_t rocid;
		uint64_t timestamp;
		vector<uint32_t> misc;
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			summary.add(rocid, NAME_OF(rocid), "%d");
			summary.add(timestamp, NAME_OF(timestamp), "%ld");
			summary.add(misc.size(), "Nmisc", "%d");
		}
		
};

#endif // _DCODAROCInfo_

