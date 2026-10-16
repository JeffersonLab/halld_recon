// $Id$
//
//    File: DCDCDigiHit.h
// Created: Tue Aug  6 11:30:10 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DCDCDigiHit_
#define _DCDCDigiHit_

#include <JANA/JObject.h>

class DCDCDigiHit: public JObject{
	public:
		JOBJECT_PUBLIC(DCDCDigiHit);
		
		int ring;
		int straw;
		uint32_t pulse_peak;           ///< identified pulse first peak as returned by FPGA algorithm (could be either Df125CDCPulse::first_max_amp or Df125FDCPulse::peak_amp)
		uint32_t pulse_integral;       ///< identified pulse integral as returned by FPGA algorithm
		uint32_t pulse_time;           ///< identified pulse time as returned by FPGA algorithm
		uint32_t pedestal;             ///< pedestal info used by FPGA (if any)
		uint32_t QF;                   ///< Quality Factor from FPGA algorithms
		uint32_t nsamples_integral;    ///< number of samples used in integral 
		uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
		    summary.add(ring, NAME_OF(ring), "%d");
		    summary.add(straw, NAME_OF(straw), "%d");
		    summary.add(pulse_peak, NAME_OF(pulse_peak), "%d");
		    summary.add(pulse_integral, NAME_OF(pulse_integral), "%d");
		    summary.add(pulse_time, NAME_OF(pulse_time), "%d");
		    summary.add(pedestal, NAME_OF(pedestal), "%d");
		    summary.add(QF, NAME_OF(QF), "%d");
		    summary.add(nsamples_integral, NAME_OF(nsamples_integral), "%d");
		    summary.add(nsamples_pedestal, NAME_OF(nsamples_pedestal), "%d");
		}
};

#endif // _DCDCDigiHit_

