// $Id$
// $HeadURL$
//
//    File: Df125PulseTime.h
// Created: Mon Jul  8 12:01:57 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.0 i386)
//

#ifndef _Df125PulseTime_
#define _Df125PulseTime_

#include <DAQ/DDAQAddress.h>

class Df125PulseTime:public DDAQAddress{
	
	/// Holds pulse time for one identified
	/// pulse in one event in one channel of a single
	/// f125 Flash ADC module.
	
	public:
		JOBJECT_PUBLIC(Df125PulseTime);
		
		Df125PulseTime(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0, uint32_t pulse_number=0, uint32_t quality_factor=0, uint32_t time=0, uint32_t overflows=0, uint32_t peak_time=0, bool emulated=false):DDAQAddress(rocid, slot, channel, itrigger),pulse_number(pulse_number),quality_factor(quality_factor),time(time),overflows(overflows),peak_time(peak_time),emulated(emulated){}
		
		uint32_t pulse_number;         ///< from Pulse Time Data word
		uint32_t quality_factor;       ///< from Pulse Time Data word
		uint32_t time;                 ///< from Pulse Time Data word
		uint32_t overflows;            ///< (future expansion. "7" means "7 or more" samples overflowed
		uint32_t peak_time;            ///< from 2nd word for FDC data only (type 6 or 9)
		bool     emulated;             ///< true if made from Window Raw Data
		
		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(pulse_number, NAME_OF(pulse_number), "%d");
			summary.add(quality_factor, NAME_OF(quality_factor), "%d");
			summary.add(time, NAME_OF(time), "%d");
			summary.add(overflows, NAME_OF(overflows), "%d");
			summary.add(peak_time, NAME_OF(peak_time), "%d");
			summary.add(emulated, NAME_OF(emulated), "%d");
		}
};

#endif // _Df125PulseTime_

