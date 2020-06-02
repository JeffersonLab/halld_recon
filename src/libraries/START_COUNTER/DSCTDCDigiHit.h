// $Id$
//
//    File: DSCTDCDigiHit.h
// Created: Tue Aug  6 13:02:22 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DSCTDCDigiHit_
#define _DSCTDCDigiHit_

#include <JANA/JObject.h>

class DSCTDCDigiHit:public JObject{
	public:
		JOBJECT_PUBLIC(DSCTDCDigiHit);
		
		int sector;		///< sector number 1-24
		uint32_t time;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(sector, NAME_OF(sector), "%d");
			summary.add(time, NAME_OF(time), "%d");
		}

};

#endif // _DSCTDCDigiHit_

