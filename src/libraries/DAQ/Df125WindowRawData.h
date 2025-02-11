// $Id$
// $HeadURL$
//
//    File: Df125WindowRawData.h
// Created: Thu Jun 19 20:48:04 EDT 2014
// Creator: davidl (on Darwin harriet.jlab.org 13.2.0 i386)
//

#ifndef _Df125WindowRawData_
#define _Df125WindowRawData_

#include <DAQ/DDAQAddress.h>

class Df125WindowRawData:public DDAQAddress{

	/// Holds window raw data samples for one event in
	/// one channel of a single f250 Flash ADC module.
	
	public:
		JOBJECT_PUBLIC(Df125WindowRawData);
	
		Df125WindowRawData(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0):DDAQAddress(rocid, slot, channel, itrigger),invalid_samples(false),overflow(false){}
	
		std::vector<uint16_t> samples;// from Window Raw Data words 2-N (each word contains 2 samples)
		bool invalid_samples;    // true if any sample's "not valid" bit set
		bool overflow;           // true if any sample's "overflow" bit set

		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(samples.size(), "Nsamples", "%d");
			summary.add(invalid_samples, NAME_OF(invalid_samples), "%d");
			summary.add(overflow, NAME_OF(overflow), "%d");
		}

};

#endif // _Df125WindowRawData_

