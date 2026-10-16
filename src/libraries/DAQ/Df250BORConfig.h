// $Id$
//
//    File: Df250BORConfig.h
// Created: Tue Jan 26 13:04:46 EST 2016
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _Df250BORConfig_
#define _Df250BORConfig_

#include <JANA/JObject.h>

#include <DAQ/bor_roc.h>

// This class inherits both from JObject and f250config. The former
// so that it can be incorporated easily into the JANA framework.
// The latter so we can use the data struct defined in bor_roc.h.
// The file bor_roc.h exists in 2 places:
//
//  1. in the DAQ library of sim-recon
//  2. in the vme/src/rcm/monitor directory in the online
//


class Df250BORConfig:public JObject, public f250config{
	public:
		JOBJECT_PUBLIC(Df250BORConfig);

		Df250BORConfig(){}
		virtual ~Df250BORConfig(){}

		uint32_t NSA;      // extracted from adc_nsa
		uint32_t NSA_trig; // extracted from adc_nsa
		 int32_t NSB;      // extracted from adc_nsb
		uint32_t NPED;     // extracted from config7
		uint32_t MaxPed;   // extracted from config7
		uint32_t NSAT;     // extracted from adc_config[0]

		/// Extract values as read from config registers
		/// and fill in the derived members defined above.
		/// This is called from DEVIOWorkerThread::ParseBORbank
		void FillDerived(void){

			NSA      = (adc_nsa>> 0) & 0x1FF;
			NSA_trig = (adc_nsa>> 9) & 0x3F;
			NSB      = (adc_nsb>> 0) & 0x07;
			if(adc_nsb&0x08) NSB = -NSB;
			NPED     = ((config7>>10) & 0xF) +1;
			MaxPed   = (config7>> 0) & 0x3FF;
			NSAT     = ((adc_config[0]>>10)&0x3) + 1;
		}

		void Summarize(JObjectSummary& summary) const override {
			summary.add(rocid, NAME_OF(rocid), "%d");
			summary.add(slot, NAME_OF(slot), "%d");
			summary.add(version, NAME_OF(version), "0x%x");
			summary.add(ctrl1, NAME_OF(ctrl1), "0x%x");
			summary.add(ctrl2, NAME_OF(ctrl2), "0x%x");
			summary.add(blk_level, NAME_OF(blk_level), "%d");
			summary.add(adc_ptw, "ptw", "%d");
			summary.add(adc_pl, "pl", "%d");
			summary.add(NSB, NAME_OF(NSB), "%d");
			summary.add(NSA, NAME_OF(NSA), "%d");
			summary.add(NSA_trig, NAME_OF(NSA_trig), "%d");
			summary.add(NPED, NAME_OF(NPED), "%d");
			summary.add(MaxPed, NAME_OF(MaxPed), "%d");
			summary.add(NSAT, NAME_OF(NSAT), "%d");
		}

};

#endif // _Df250BORConfig_
