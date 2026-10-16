// $Id$
//
//    File: DECALRefDigiHit.h
// Created: Tue Aug  6 12:23:55 EDT 2013


#ifndef _DECALRefDigiHit_
#define _DECALRefDigiHit_

#include <JANA/JObject.h>
#include <JANA/JFactoryT.h>

class DECALRefDigiHit:public JObject{
	public:
		JOBJECT_PUBLIC(DECALRefDigiHit);

		int id;    /// Reference counter number
		
		uint32_t pulse_integral; ///< identified pulse integral as returned by FPGA algorithm
		uint32_t pulse_time;     ///< identified pulse time as returned by FPGA algorithm
		uint32_t pedestal;       ///< pedestal info used by FPGA (if any)
		uint32_t QF;             ///< Quality Factor from FPGA algorithms
		uint32_t nsamples_integral;    ///< number of samples used in integral 
		uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
		uint32_t pulse_peak;           ///<  maximum sample in pulse
		
		uint32_t datasource;           ///<  0=window raw data, 1=old(pre-Fall16) firmware, 2=Df250PulseData
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const{
			summary.add(id, "id", "%d");		  
			summary.add(pulse_integral, "pulse_integral", "%d");
			summary.add(pulse_peak, "pulse_peak", "%d");
			summary.add(pulse_time, "pulse_time", "%d");
			summary.add(pedestal, "pedestal", "%d");
			summary.add(QF, "QF", "%d");
			summary.add(nsamples_integral, "nsamples_integral", "%d");
			summary.add(nsamples_pedestal, "nsamples_pedestal", "%d");
		}
		
};

#endif // _DECALRefDigiHit_

