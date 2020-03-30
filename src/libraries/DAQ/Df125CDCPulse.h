// $Id$
// $HeadURL$
//
//    File: Df125CDCPulse.h
// Created: Fri Nov  13 16:16:00 EDT 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 x86_64)
//

#ifndef _Df125CDCPulse_
#define _Df125CDCPulse_

#include <DAQ/DDAQAddress.h>

class Df125CDCPulse:public DDAQAddress{
	
	/// Holds pulse integral data for one identified
	/// pulse in one event in one channel of a single
	/// f125 Flash ADC module.
	
	public:
		JOBJECT_PUBLIC(Df125CDCPulse);

		Df125CDCPulse(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0
		                , uint32_t NPK=0
						, uint32_t le_time=0
						, uint32_t time_quality_bit=0
						, uint32_t overflow_count=0
						, uint32_t pedestal=0
						, uint32_t integral=0
						, uint32_t first_max_amp=0
						, uint32_t word1=0
						, uint32_t word2=0
						, uint32_t nsamples_pedestal=1
						, uint32_t nsamples_integral=1
						, bool emulated=false 
                        , uint32_t le_time_emulated = 0xffff
                        , uint32_t time_quality_bit_emulated = 0xffff
                        , uint32_t overflow_count_emulated = 0xffff
                        , uint32_t pedestal_emulated = 0xffff
                        , uint32_t integral_emulated = 0xffff
                        , uint32_t first_max_amp_emulated = 0xffff )
						  :DDAQAddress(rocid, slot, channel, itrigger)
						  , NPK(NPK)
						  , le_time(le_time)
						  , time_quality_bit(time_quality_bit)
						  , overflow_count(overflow_count)
						  , pedestal(pedestal)
						  , integral(integral)
						  , first_max_amp(first_max_amp)
						  , word1(word1)
						  , word2(word2)
						  , nsamples_pedestal(nsamples_pedestal)
						  , nsamples_integral(nsamples_integral)
						  , emulated(emulated)
                          , le_time_emulated(le_time_emulated)
                          , time_quality_bit_emulated(time_quality_bit_emulated)
                          , overflow_count_emulated(overflow_count_emulated)
                          , pedestal_emulated(pedestal_emulated)
                          , integral_emulated(integral_emulated)
                          , first_max_amp_emulated(first_max_amp_emulated){}

		uint32_t NPK;                       ///< from first word
		uint32_t le_time;                   ///< from first word
		uint32_t time_quality_bit;          ///< from first word
		uint32_t overflow_count;            ///< from first word
		uint32_t pedestal;                  ///< from second word
		uint32_t integral;                  ///< from second word
		uint32_t first_max_amp;             ///< from second word
		uint32_t word1;                     ///< first word
		uint32_t word2;                     ///< second word
		uint32_t nsamples_pedestal;         ///< number of samples used in integral 
		uint32_t nsamples_integral;         ///< number of samples used in pedestal
		bool     emulated;                  ///< true if emulated values are copied to the main input 
        uint32_t le_time_emulated;          ///< emulated from raw data when available
        uint32_t time_quality_bit_emulated; ///< emulated from raw data when available
        uint32_t overflow_count_emulated;   ///< emulated from raw data when available
        uint32_t pedestal_emulated;         ///< emulated from raw data when available
        uint32_t integral_emulated;         ///< emulated from raw data when available
        uint32_t first_max_amp_emulated;    ///< emulated from raw data when available


		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(le_time, NAME_OF(le_time), "%d");
            summary.add(le_time_emulated, "le_time_em", "%d");
			summary.add(integral, NAME_OF(integral), "%d");
            summary.add(integral_emulated, "integral_em", "%d");
			summary.add(pedestal, NAME_OF(pedestal), "%d");
			summary.add(NPK, NAME_OF(NPK), "%d");
			summary.add(time_quality_bit, NAME_OF(time_quality_bit), "%d");
			summary.add(overflow_count, NAME_OF(overflow_count), "%d");
			summary.add(first_max_amp, NAME_OF(first_max_amp), "%d");
			summary.add(nsamples_integral, NAME_OF(nsamples_integral), "%d");
			summary.add(nsamples_pedestal, NAME_OF(nsamples_pedestal), "%d");
			summary.add(emulated, NAME_OF(emulated), "%d");
		}
};

#endif // _Df125CDCPulse_

