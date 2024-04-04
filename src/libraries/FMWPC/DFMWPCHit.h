// $Id$
//
//    File: DFMWPCHit.h
// Created: Tue Jun 16 07:07:54 EDT 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _DFMWPCHit_
#define _DFMWPCHit_

#include <JANA/JObject.h>


class DFMWPCHit:public JObject{
	public:
		JOBJECT_PUBLIC(DFMWPCHit);

		int layer;   // 1-8
		int wire;    // 1-144
		float q;     // charge deposited
		float amp;   // peak amplitude
		float t;     // time in ns
		int QF;      // quality factor
		float ped;   // pedestal

		void Summarize(JObjectSummary& summary) const {
			summary.add(layer, "layer", "%d");
			summary.add(wire, "wire", "%d");
			summary.add(q, "q",      "%10.2f");
			summary.add(amp, "amp",    "%10.2f");
			summary.add(t, "t", "%3.3f");
			summary.add(QF, "QF", "%d");
			summary.add(ped, "ped", "%10.2f");
		}

};

#endif // _DFMWPCHit_

