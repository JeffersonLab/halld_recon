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

		void toStrings(vector<pair<string, string> >&items) const {
			AddString(items, "layer", "%d", layer);
			AddString(items, "wire", "%d", wire);
			AddString(items, "q",      "%10.2f",  q);
			AddString(items, "amp",    "%10.2f", amp);
			AddString(items, "t", "%3.3f", t);
			AddString(items, "QF", "%d", QF);
			AddString(items, "ped", "%10.2f", ped);
		}

};

#endif // _DFMWPCHit_

