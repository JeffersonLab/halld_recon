// $Id:$
// $HeadURL:$
//
//    File: DDIRCPulseTime.h
// Created: Tue Aug 21 22:20:40 EDT 2018
// Creator: davidl
//

#ifndef _DDIRCPulseTime_
#define _DDIRCPulseTime_

#include <DAQ/DDAQAddress.h>

class DDIRCTDCHit:public DDAQAddress{
	
	/// Holds TDC Hit time for DIRC
	
	public:
		JOBJECT_PUBLIC(DDIRCTDCHit);
		
		DDIRCTDCHit(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0, uint32_t dev_id=0, uint32_t ievent_cnt=0, uint32_t channel_fpga=0, uint32_t edge=0, uint32_t time=0):
            DDAQAddress(rocid, slot, channel, itrigger),dev_id(dev_id),ievent_cnt(ievent_cnt),channel_fpga(channel_fpga),edge(edge),time(time){}
		
		uint32_t dev_id;       ///< device id
		uint32_t ievent_cnt;   ///< event_cnt (from device ID)
		uint32_t channel_fpga; ///< 0-191 from TDCHit word
		uint32_t edge;         ///< 0=leading edge  1=trailing edge
		uint32_t time;         ///< 16 bit relative to beginning of defined readout window
		
		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(dev_id, NAME_OF(dev_id), "%d");
			summary.add(ievent_cnt, NAME_OF(ievent_cnt), "%d");
			summary.add(channel_fpga, NAME_OF(channel_fpga), "%d");
			summary.add(edge, NAME_OF(edge), "%d");
			summary.add(time, NAME_OF(time), "%d");
		}
};

#endif // _DDIRCPulseTime_

