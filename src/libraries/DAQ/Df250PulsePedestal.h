// $Id$
// $HeadURL$
//
//    File: Df250PulsePedestal.h
// Created: Mon Jul 28 09:44:35 EDT 2014
// Creator: davidl (on Darwin harriet.jlab.org 11.4.0 i386)
//

#ifndef _Df250PulsePedestal_
#define _Df250PulsePedestal_

#include <DAQ/DDAQAddress.h>

class Df250PulsePedestal:public DDAQAddress{
	
	/// Holds pulse time for one identified
	/// pulse in one event in one channel of a single
	/// f250 Flash ADC module.
	
	public:
		JOBJECT_PUBLIC(Df250PulsePedestal);
		
		Df250PulsePedestal(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0, uint32_t pulse_number=0, uint32_t pedestal=0, 
                uint32_t pulse_peak=0,bool emulated=false,uint32_t pedestal_emulated = 0xffff, uint32_t pulse_peak_emulated=0xffff):
            DDAQAddress(rocid, slot, channel, itrigger),pulse_number(pulse_number),pedestal(pedestal),pulse_peak(pulse_peak),
            emulated(emulated),pedestal_emulated(pedestal_emulated),pulse_peak_emulated(pulse_peak_emulated){}
		
		uint32_t pulse_number;   ///< from Pulse Pedestal Data word
		uint32_t pedestal;       ///< from Pulse Pedestal Data word
		uint32_t pulse_peak;     ///< from Pulse Pedestal Data word
		bool     emulated;       ///< true if made from Window Raw Data
        uint32_t pedestal_emulated;     ///< Calculated from raw data (when available)
        uint32_t pulse_peak_emulated;   ///< Calculated from raw data (when available)

		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(pulse_number, NAME_OF(pulse_number), "%d");
			summary.add(pedestal, NAME_OF(pedestal), "%d");
            summary.add(pedestal_emulated, NAME_OF(pedestal_emulated), "%d");
			summary.add(pulse_peak, NAME_OF(pulse_peak), "%d");
            summary.add(pulse_peak_emulated, NAME_OF(pulse_peak_emulated), "%d");
			summary.add(emulated, NAME_OF(emulated), "%d");
		}
};

#endif // _Df250PulsePedestal_

