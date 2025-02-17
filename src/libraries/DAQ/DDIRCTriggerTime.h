// $Id:$
// $HeadURL:$
//
//    File: DDIRCTriggerTime.h
// Created: Tue Aug 21 22:20:40 EDT 2018
// Creator: davidl 
//

#ifndef _DDIRCTriggerTime_
#define _DDIRCTriggerTime_

#include <DAQ/DDAQAddress.h>

class DDIRCTriggerTime:public DDAQAddress{
	
	/// Holds trigger time data for one event from DIRC
	
	public:
		JOBJECT_PUBLIC(DDIRCTriggerTime);

		DDIRCTriggerTime():DDAQAddress(0, 0, 0, 0),time(0){}
		DDIRCTriggerTime(uint32_t rocid, uint32_t slot, uint32_t itrigger, uint64_t time):DDAQAddress(rocid, slot, 0, itrigger),time(time){}
		
		uint64_t time;           // from Trigger Time words
		
		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(time, NAME_OF(time), "%ld");
		}

};

#endif // _DDIRCTriggerTime_

