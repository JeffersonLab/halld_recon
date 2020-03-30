// $Id$
// $HeadURL$
//
//    File: DF1TDCConfig.h
// Created: Sun Sep  7 15:52:50 EDT 2014
// Creator: davidl (on Darwin harriet.local 13.3.0 i386)
//

#ifndef _DF1TDCConfig_
#define _DF1TDCConfig_

#include <DAQ/DDAQConfig.h>

class DF1TDCConfig:public DDAQConfig{
	public:
		JOBJECT_PUBLIC(DF1TDCConfig);
		
		DF1TDCConfig(uint32_t rocid, uint32_t slot_mask):DDAQConfig(rocid,slot_mask),REFCNT(0xFFFF),TRIGWIN(0xFFFF),TRIGLAT(0xFFFF),HSDIV(0xFFFF),BINSIZE(0xFFFF),REFCLKDIV(0xFFFF){}
		DF1TDCConfig(const DF1TDCConfig *c):DDAQConfig(c->rocid,c->slot_mask),REFCNT(c->REFCNT),TRIGWIN(c->TRIGWIN),TRIGLAT(c->TRIGLAT),HSDIV(c->HSDIV),BINSIZE(c->BINSIZE),REFCLKDIV(c->REFCLKDIV){}

		uint16_t REFCNT;
		uint16_t TRIGWIN;
		uint16_t TRIGLAT;
		uint16_t HSDIV;
		uint16_t BINSIZE;
		uint16_t REFCLKDIV;

		void Summarize(JObjectSummary& summary) const override {
			DDAQConfig::Summarize(summary);
			summary.add(REFCNT, NAME_OF(REFCNT), "%d");
			summary.add(TRIGWIN, NAME_OF(TRIGWIN), "%d");
			summary.add(TRIGLAT, NAME_OF(TRIGLAT), "%d");
			summary.add(HSDIV, NAME_OF(HSDIV), "%d");
			summary.add(BINSIZE, "BINSIZE(ps)", "%d");
			summary.add(REFCLKDIV, NAME_OF(REFCLKDIV), "%d");
		}
		
};

#endif // _DF1TDCConfig_

