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

class DFMWPCDigiHit:public jana::JObject{
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
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "layer", "%d", layer);
			AddString(items, "wire", "%d", wire);
			AddString(items, "pulse_peak", "%d", pulse_peak);
			AddString(items, "pulse_integral", "%d", pulse_integral);
			AddString(items, "pulse_time", "%d", pulse_time);
			AddString(items, "pedestal", "%d", pedestal);
			AddString(items, "QF", "%d", QF);
			AddString(items, "nsamples_integral", "%d", nsamples_integral);
			AddString(items, "nsamples_pedestal", "%d", nsamples_pedestal);
		}		
};

#endif // _DFMWPCDigiHit_

