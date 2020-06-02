#ifndef _DTPOLSectorDigiHit_
#define _DTPOLSectorDigiHit_

#include <JANA/JObject.h>

class DTPOLSectorDigiHit : public JObject {
	public:
		JOBJECT_PUBLIC(DTPOLSectorDigiHit);

		int      sector;             // sector number 1-32
		uint32_t pulse_integral;    // identified pulse integral as returned by FPGA algorithm
		uint32_t pulse_time;        // identified pulse time as returned by FPGA algorithm
		uint32_t pedestal;          // pedestal info used by FPGA (if any)
		uint32_t QF;                // Quality Factor from FPGA algorithms
		uint32_t nsamples_integral; //  number of samples used in integral 
		uint32_t nsamples_pedestal; //  number of samples used in pedestal
		uint32_t pulse_peak;        //  maximum sample in pulse
		uint32_t datasource;        //  0=window raw data, 1=old(pre-Fall16) firmware, 2=Df250PulseData


		void Summarize(JObjectSummary& summary) const override {
			summary.add(sector, NAME_OF(sector), "%d");
			summary.add(pulse_integral, NAME_OF(pulse_integral), "%d");
			summary.add(pulse_peak, NAME_OF(pulse_peak), "%d");
			summary.add(pedestal, NAME_OF(pedestal), "%d");
			summary.add(QF, NAME_OF(QF), "%d");
			summary.add(nsamples_integral, NAME_OF(nsamples_integral), "%d");
			summary.add(nsamples_pedestal, NAME_OF(nsamples_pedestal), "%d");
		}
};

#endif // _DTPOLSectorDigiHit_
