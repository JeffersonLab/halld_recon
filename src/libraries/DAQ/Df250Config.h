// $Id$
// $HeadURL$
//
//    File: Df250Config.h
// Created: Sun Sep  7 15:52:50 EDT 2014
// Creator: davidl (on Darwin harriet.local 13.3.0 i386)
//

#ifndef _Df250Config_
#define _Df250Config_

#include <DAQ/DDAQConfig.h>

class Df250Config:public DDAQConfig{
	public:
		JOBJECT_PUBLIC(Df250Config);

		Df250Config(uint32_t rocid, uint32_t slot_mask):DDAQConfig(rocid,slot_mask),NSA(0xFFFF),NSB(0xFFFF),NSA_NSB(0xFFFF),NPED(0xFFFF){}
		Df250Config(const Df250Config *c):DDAQConfig(c->rocid,c->slot_mask),NSA(c->NSA),NSB(c->NSB),NSA_NSB(c->NSA_NSB),NPED(c->NPED){}
		
		uint16_t NSA;      // Num. samples before threshold crossing sample
		uint16_t NSB;      // Num. samples after  threshold crossing sample
		uint16_t NSA_NSB;  // NSA+NSB = total number of samples in integration window
		uint16_t NPED;     // Number of samples used to determine pedestal
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			DDAQConfig::Summarize(summary);
			summary.add(NSA, NAME_OF(NSA), "%d");
			summary.add(NSB, NAME_OF(NSB), "%d");
			summary.add(NSA_NSB, NAME_OF(NSA_NSB), "%d");
			summary.add(NPED, NAME_OF(NPED), "%d");
		}
};

#endif // _Df250Config_

