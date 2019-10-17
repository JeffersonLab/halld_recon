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
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			DDAQAddress::toStrings(items);
			AddString(items, "dev_id",       "%d", dev_id);
			AddString(items, "ievent_cnt",   "%d", ievent_cnt);
			AddString(items, "channel_fpga", "%d", channel_fpga);
			AddString(items, "edge",         "%d", edge);
			AddString(items, "time",         "%d", time);
		}
};

#endif // _DDIRCPulseTime_

