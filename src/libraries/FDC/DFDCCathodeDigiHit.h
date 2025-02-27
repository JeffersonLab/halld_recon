// $Id$
//
//    File: DFDCCathodeDigiHit.h
// Created: Wed Aug  7 11:53:57 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DFDCCathodeDigiHit_
#define _DFDCCathodeDigiHit_

#include <JANA/JObject.h>

class DFDCCathodeDigiHit:public JObject{
	public:
		JOBJECT_PUBLIC(DFDCCathodeDigiHit);

		uint32_t package;
		uint32_t chamber;
		uint32_t view;
		uint32_t strip;
		uint32_t strip_type;		
		uint32_t pulse_integral;       ///< identified pulse integral as returned by FPGA algorithm
		uint32_t pulse_time;           ///< identified pulse time as returned by FPGA algorithm
		uint32_t pedestal;             ///< pedestal info used by FPGA (if any)
		uint32_t QF;                   ///< Quality Factor from FPGA algorithms
		uint32_t nsamples_integral;    ///< number of samples used in integral 
		uint32_t nsamples_pedestal;    ///< number of samples used in pedestal


		void Summarize(JObjectSummary& summary) const override {
			summary.add(package, NAME_OF(package), "%d");
			summary.add(chamber, NAME_OF(chamber), "%d");
			summary.add(view, NAME_OF(view), "%d");
			summary.add(strip, NAME_OF(strip), "%d");
			summary.add(strip_type, NAME_OF(strip_type), "%d");
			summary.add(pulse_integral, NAME_OF(pulse_integral), "%d");
			summary.add(pulse_time, NAME_OF(pulse_time), "%d");
			summary.add(pedestal, NAME_OF(pedestal), "%d");
			summary.add(QF, NAME_OF(QF), "%d");
			summary.add(nsamples_integral, NAME_OF(nsamples_integral), "%d");
			summary.add(nsamples_pedestal, NAME_OF(nsamples_pedestal), "%d");
		}
};

#endif // _DFDCCathodeDigiHit_

