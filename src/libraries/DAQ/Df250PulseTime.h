// $Id$
// $HeadURL$
//
//    File: Df250PulseTime.h
// Created: Tue Aug  7 15:25:03 EDT 2012
// Creator: davidl (on Darwin harriet.jlab.org 11.4.0 i386)
//

#ifndef _Df250PulseTime_
#define _Df250PulseTime_

#include <DAQ/DDAQAddress.h>

class Df250PulseTime:public DDAQAddress{
	
	/// Holds pulse time for one identified
	/// pulse in one event in one channel of a single
	/// f250 Flash ADC module.
	
	public:
		JOBJECT_PUBLIC(Df250PulseTime);
		
		Df250PulseTime(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0, uint32_t pulse_number=0, uint32_t quality_factor=0, 
                uint32_t time=0, bool emulated=false, uint32_t quality_factor_emulated = 0xffff, uint32_t time_emulated = 0xffff):
            DDAQAddress(rocid, slot, channel, itrigger),pulse_number(pulse_number),quality_factor(quality_factor),time(time),
            emulated(emulated), quality_factor_emulated(quality_factor_emulated), time_emulated(time_emulated){}
		
		uint32_t pulse_number;         ///< from Pulse Time Data word
		uint32_t quality_factor;       ///< from Pulse Time Data word
		uint32_t time;                 ///< from Pulse Time Data word
		bool     emulated;             ///< true if made from Window Raw Data
        uint32_t quality_factor_emulated;   ///< Calculated from raw data if available
        uint32_t time_emulated;             ///< Calculated from raw data if available
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(pulse_number, NAME_OF(pulse_number), "%d");
			summary.add(quality_factor, NAME_OF(quality_factor), "%d");
            summary.add(quality_factor_emulated, NAME_OF(quality_factor_emulated), "%d");
			summary.add(time, NAME_OF(time), "%d");
            summary.add(time_emulated, NAME_OF(time_emulated), "%d");
			summary.add(emulated, NAME_OF(emulated), "%d");
		}
};

#endif // _Df250PulseTime_

