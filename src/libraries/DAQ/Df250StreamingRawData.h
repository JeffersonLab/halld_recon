// $Id$
// $HeadURL$
//
//    File: Df250StreamingRawData.h
// Created: Tue Aug  7 15:25:15 EDT 2012
// Creator: davidl (on Darwin harriet.jlab.org 11.4.0 i386)
//

#ifndef _Df250StreamingRawData_
#define _Df250StreamingRawData_

#include <DAQ/DDAQAddress.h>

class Df250StreamingRawData:public DDAQAddress{
	
	/// Holds streaming raw data for one event in one
	/// channel of a single f250 Flash ADC module.
	/// This should represent a dump of the entire
	/// memory for the channel (~17 milliseconds worth
	/// of data) It will only be used for debugging.
	
	public:
		JOBJECT_PUBLIC(Df250StreamingRawData);
		
		std::vector<uint16_t> samples;   // from Streaming Raw Data words 2-N (each word contains 2 samples)
		bool invalid_samples;       // true if any sample's "not valid" bit set
		bool overflow;              // true if any sample's "overflow" bit set


		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(samples.size(), "Nsamples", "%d");
			summary.add(invalid_samples, NAME_OF(invalid_samples), "%d");
			summary.add(overflow, NAME_OF(overflow), "%d");
		}

};

#endif // _Df250StreamingRawData_

