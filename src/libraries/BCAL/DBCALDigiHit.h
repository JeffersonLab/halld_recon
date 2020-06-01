// $Id$
//
//    File: DBCALDigiHit.h
// Created: Tue Aug  6 09:14:41 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DBCALDigiHit_
#define _DBCALDigiHit_

#include <BCAL/DBCALGeometry.h>

#include <JANA/JObject.h>

class DBCALDigiHit:public JObject{

	/// This class holds a single hit from a BCAL fADC250 module.
	/// The values are in the digitized form coming from the module
	/// and are therefore uncalibrated.

	public:
		JOBJECT_PUBLIC(DBCALDigiHit);

		int module;
		int layer;
		int sector;
		DBCALGeometry::End end;
		uint32_t pulse_integral; ///< identified pulse integral as returned by FPGA algorithm
		uint32_t pulse_peak;     ///< identified pulse height as returned by FPGA algorithm
		uint32_t pulse_time;     ///< identified pulse time as returned by FPGA algorithm
		uint32_t pedestal;       ///< pedestal info used by FPGA (if any)
		uint32_t QF;             ///< Quality Factor from FPGA algorithms
		uint32_t nsamples_integral;    ///< number of samples used in integral 
		uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
		
		uint32_t datasource;           ///<  0=window raw data, 1=old(pre-Fall16) firmware, 2=Df250PulseData,  3=MC
		
		void Summarize(JObjectSummary& summary) const override {
			summary.add(module, NAME_OF(module), "%d");
			summary.add(layer, NAME_OF(layer), "%d");
			summary.add(sector, NAME_OF(sector), "%d");
			summary.add((end==0 ? "upstream":"downstream"), "end", "%s");
			summary.add(pulse_integral, NAME_OF(pulse_integral), "%d");
			summary.add(pulse_peak, NAME_OF(pulse_peak), "%d");
			summary.add(pulse_time, NAME_OF(pulse_time), "%d");
			summary.add(pedestal, NAME_OF(pedestal), "%d");
			summary.add(QF, NAME_OF(QF), "%d");
			summary.add(nsamples_integral, NAME_OF(nsamples_integral), "%d");
			summary.add(nsamples_pedestal, NAME_OF(nsamples_pedestal), "%d");
		}
};

#endif // _DBCALDigiHit_

