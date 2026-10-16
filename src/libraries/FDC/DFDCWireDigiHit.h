// $Id$
//
//    File: DFDCWireDigiHit.h
// Created: Wed Aug  7 11:54:06 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DFDCWireDigiHit_
#define _DFDCWireDigiHit_

#include <JANA/JObject.h>

class DFDCWireDigiHit:public JObject{
	public:
		JOBJECT_PUBLIC(DFDCWireDigiHit);
		
		uint32_t package;
		uint32_t chamber;
		uint32_t wire;
		uint32_t time;
		
		void Summarize(JObjectSummary& summary) const override {
			summary.add(package, NAME_OF(package), "%d");
			summary.add(chamber, NAME_OF(chamber), "%d");
			summary.add(wire, NAME_OF(wire), "%d");
			summary.add(time, NAME_OF(time), "%d");
		}
		
};

#endif // _DFDCWireDigiHit_

