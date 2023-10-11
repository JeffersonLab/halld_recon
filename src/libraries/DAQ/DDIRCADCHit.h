// $Id:$
// $HeadURL:$
//
//    File: DDIRCADCHit.h
// Created: Wed Aug 22 15:02:37 EDT 2018
// Creator: davidl
//

#ifndef _DDIRCADCHit_
#define _DDIRCADCHit_

#include <DAQ/DDAQAddress.h>

class DDIRCADCHit:public DDAQAddress{
	
	/// Holds pulse integral data for one identified
	/// pulse in one event in one channel of a single
	/// f250 Flash ADC module.
	
	public:
		JOBJECT_PUBLIC(DDIRCADCHit);

		DDIRCADCHit(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0, uint32_t dev_id=0, uint32_t ievent_cnt=0, 
		    uint32_t adc_hold1=0, uint32_t adc_hold2=0, uint32_t adc_max_bits=0, uint32_t maroc_id=0,uint32_t adc=0):
		DDAQAddress(rocid, slot, channel, itrigger),dev_id(dev_id),ievent_cnt(ievent_cnt),
		  adc_hold1(adc_hold1),adc_hold2(adc_hold2),adc_max_bits(adc_max_bits),maroc_id(maroc_id),adc(adc){}
		
		uint32_t dev_id;         ///< device id
		uint32_t ievent_cnt;     ///< event_cnt (from device ID)
		uint32_t adc_hold1;      ///< from Pulse Integral Data word
		uint32_t adc_hold2;      ///< from Pulse Integral Data word
		uint32_t adc_max_bits;   ///< from Pulse Integral Data word
		uint32_t maroc_id;       ///< from Pulse Integral Data word (future)
		uint32_t adc;            ///< number of samples used in integral 


		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(dev_id, NAME_OF(dev_id), "%d");
			summary.add(ievent_cnt, NAME_OF(ievent_cnt), "%d");
			summary.add(adc_hold1, NAME_OF(adc_hold1), "%d");
			summary.add(adc_hold2, NAME_OF(adc_hold2), "%d");
			summary.add(adc_max_bits, NAME_OF(adc_max_bits), "%d");
			summary.add(maroc_id, NAME_OF(maroc_id), "%d");
			summary.add(adc, NAME_OF(adc), "%d");
		}
};

#endif // _DDIRCADCHit_

