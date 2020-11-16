// $Id$
//
//    File: DCGEMTruthHit.h
// Created: Tue Jun 16 07:07:54 EDT 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _DCGEMTruthHit_
#define _DCGEMTruthHit_

#include <JANA/jerror.h>

class DCGEMTruthHit:public JObject{
	public:
		JOBJECT_PUBLIC(DCGEMTruthHit);

		int layer;   // 1-8
		int hole;    // 1-144
		float dE;    // GeV
		float dx;    // cm
		float t;     // ns

		void toStrings(vector<pair<string, string> >&items) const {
			AddString(items, "layer", "%d", layer);
			AddString(items, "hole", "%d", hole);
			AddString(items, "dE(keV)", "%3.1f", dE*1.0E6);
			AddString(items, "dx", "%3.2f", dx);
			AddString(items, "t", "%3.3f", t);
		}

};

#endif // _DCGEMTruthHit_

