// $Id$
//
//    File: DTRDDigiHit.h
//

#ifndef _DTRDDigiHit_
#define _DTRDDigiHit_

#include <JANA/JObject.h>

class DTRDDigiHit : public JObject {
	public:
		JOBJECT_PUBLIC(DTRDDigiHit);
		
		uint32_t plane;
		uint32_t strip;
		uint32_t pulse_peak;           ///< identified pulse peak as returned by FPGA algorithm
		uint32_t pulse_time;           ///< identified pulse time as returned by FPGA algorithm
		uint32_t pedestal;             ///< pedestal info used by FPGA (if any)
		uint32_t QF;                   ///< Quality Factor from FPGA algorithms
		uint32_t nsamples_integral;    ///< number of samples used in integral 
		uint32_t nsamples_pedestal;    ///< number of samples used in pedestal


		void Summarize(JObjectSummary& summary) const override {
			summary.add(plane, NAME_OF(plane), "%4d");
			summary.add(strip, NAME_OF(strip), "%4d");
			summary.add(pulse_peak, NAME_OF(pulse_peak), "%d");
			summary.add(pulse_time, NAME_OF(pulse_time), "%d");
			summary.add(pedestal, NAME_OF(pedestal), "%d");
			summary.add(QF, NAME_OF(QF), "%d");
			summary.add(nsamples_integral, NAME_OF(nsamples_integral), "%d");
			summary.add(nsamples_pedestal, NAME_OF(nsamples_pedestal), "%d");
		}
};

#endif // _DTRDDigiHit_

