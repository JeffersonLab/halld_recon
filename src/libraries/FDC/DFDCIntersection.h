// $Id$
//
//    File: DFDCIntersection.h
// Created: Tue Oct 30 11:24:53 EDT 2007
// Creator: davidl (on Darwin fwing-dhcp95.jlab.org 8.10.1 i386)
//

#ifndef _DFDCIntersection_
#define _DFDCIntersection_

#include <cmath>

#include <JANA/JObject.h>
#include <DVector3.h>
#include "FDC/DFDCHit.h"
#include "FDC/DFDCWire.h"

class DFDCIntersection:public JObject{
	public:
		JOBJECT_PUBLIC(DFDCIntersection);
		
		const DFDCHit *hit1;
		const DFDCHit *hit2;
		const DFDCWire *wire1;
		const DFDCWire *wire2;
		DVector3 pos;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(wire1->layer, "layer1", "%d");
			summary.add(wire1->wire, "wire1", "%d");
			summary.add(wire1->angle*180.0/M_PI, "angle1(deg)", "%3.1f");
			summary.add(wire2->layer, "layer2", "%d");
			summary.add(wire2->wire, "wire2", "%d");
			summary.add(wire2->angle*180.0/M_PI, "angle2(deg)", "%3.1f");
		}
};

#endif // _DFDCIntersection_

