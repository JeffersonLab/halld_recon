// $Id$
//
//    File: DTOFDigiHit.h
// Created: Wed Aug  7 09:30:38 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DTOFDigiHit_
#define _DTOFDigiHit_

#include <JANA/JObject.h>

class DTOFDigiHit:public JObject{
	public:
		JOBJECT_PUBLIC(DTOFDigiHit);

		int plane;   // plane (0: vertical, 1: horizontal)
		int bar;     // bar number
		int end;     // left/right 0/1 or North/South 0/1
		uint32_t pulse_integral; ///< identified pulse integral as returned by FPGA algorithm
		uint32_t pulse_time;     ///< identified pulse time as returned by FPGA algorithm
		uint32_t pedestal;       ///< pedestal info used by FPGA (if any)
		uint32_t QF;             ///< Quality Factor from FPGA algorithms
		uint32_t nsamples_integral;    ///< number of samples used in integral 
		uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
		uint32_t pulse_peak;           ///<  maximum sample in pulse
		uint32_t datasource;           ///<  0=window raw data, 1=old(pre-Fall16) firmware, 2=Df250PulseData


		void Summarize(JObjectSummary& summary) const override {
			summary.add(bar, NAME_OF(bar), "%d");
			summary.add(plane, NAME_OF(plane), "%d");
			summary.add(end, NAME_OF(end), "%d");
			summary.add(pulse_integral, NAME_OF(pulse_integral), "%d");
			summary.add(pulse_peak, NAME_OF(pulse_peak), "%d");
			summary.add(pulse_time, NAME_OF(pulse_time), "%d");
			summary.add(pedestal, NAME_OF(pedestal), "%d");
			summary.add(QF, NAME_OF(QF), "%d");
			summary.add(nsamples_integral, NAME_OF(nsamples_integral), "%d");
			summary.add(nsamples_pedestal, NAME_OF(nsamples_pedestal), "%d");
		}
};

#endif // _DTOFDigiHit_

