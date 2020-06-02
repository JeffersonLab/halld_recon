// $Id$
//
//    File: DTOFTDCDigiHit.h
// Created: Wed Aug  7 09:31:00 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DTOFTDCDigiHit_
#define _DTOFTDCDigiHit_

#include <JANA/JObject.h>

class DTOFTDCDigiHit:public JObject{
	public:
		JOBJECT_PUBLIC(DTOFTDCDigiHit);
		
		int plane;      ///< plane (0: vertical, 1: horizontal)
		int bar;        ///< bar number
		int end;        ///< left/right 0/1 or North/South 0/1
		uint32_t time;	///< hit time

		void Summarize(JObjectSummary& summary) const override {
			summary.add(bar, NAME_OF(bar), "%d");
			summary.add(plane, NAME_OF(plane), "%d");
			summary.add(end, NAME_OF(end), "%d");
			summary.add(time, NAME_OF(time), "%d");
		}
};

#endif // _DTOFTDCDigiHit_

