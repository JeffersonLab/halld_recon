// $Id$
//
//    File: DFMWPCDigiHit.h
// Created: Tue Aug  6 11:30:10 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DFMWPCDigiHit_
#define _DFMWPCDigiHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DFMWPCDigiHit:public JObject{
	public:
		JOBJECT_PUBLIC(DFMWPCDigiHit);
		
		int layer;
		int wire;
		uint32_t pulse_peak;           ///< identified pulse first peak as returned by FPGA algorithm (could be either Df125CDCPulse::first_max_amp or Df125FDCPulse::peak_amp)
		uint32_t pulse_integral;       ///< identified pulse integral as returned by FPGA algorithm
		uint32_t pulse_time;           ///< identified pulse time as returned by FPGA algorithm
		uint32_t pedestal;             ///< pedestal info used by FPGA (if any)
		uint32_t QF;                   ///< Quality Factor from FPGA algorithms
		uint32_t nsamples_integral;    ///< number of samples used in integral 
		uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
		
		// This method is used primarily for pretty printing
		// the third argument to summary.add is printf style format
		void Summarize(JObjectSummary& summary)const{
			summary.add(layer, "layer", "%d");
			summary.add(wire, "wire", "%d");
			summary.add(pulse_peak, "pulse_peak", "%d");
			summary.add(pulse_integral, "pulse_integral", "%d");
			summary.add(pulse_time, "pulse_time", "%d");
			summary.add(pedestal, "pedestal", "%d");
			summary.add(QF, "QF", "%d");
			summary.add(nsamples_integral, "nsamples_integral", "%d");
			summary.add(nsamples_pedestal, "nsamples_pedestal", "%d");
		}		
};

#endif // _DFMWPCDigiHit_

