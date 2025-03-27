// $Id$
// $HeadURL$
//
//    File: DHelicityDataTriggerTime.h
//

#ifndef _DHelicityDataTriggerTime_
#define _DHelicityDataTriggerTime_

#include <DAQ/DDAQAddress.h>

class DHelicityDataTriggerTime:public DDAQAddress{
	
	/// Holds trigger time data for one event in
	/// a helicity decoder module.
	
	public:
		JOBJECT_PUBLIC(DHelicityDataTriggerTime);

		DHelicityDataTriggerTime():DDAQAddress(0, 0, 0, 0),time(0){}
		DHelicityDataTriggerTime(uint32_t rocid, uint32_t slot, uint32_t itrigger, uint64_t time):DDAQAddress(rocid, slot, 0, itrigger),time(time){}
		
		uint64_t time;           // from Trigger Time words
		
	
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(time, NAME_OF(time), "%ld");
		}

};

#endif // _DHelicityDataTriggerTime_

