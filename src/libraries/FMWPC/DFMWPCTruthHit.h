// $Id$
//
//    File: DFMWPCTruthHit.h
// Created: Tue Jun 16 07:07:54 EDT 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _DFMWPCTruthHit_
#define _DFMWPCTruthHit_

class DFMWPCTruthHit: public JObject{

	public:
		JOBJECT_PUBLIC(DFMWPCTruthHit);

		int layer;   // 1-8
		int wire;    // 1-144
		float dE;    // GeV
		float q; // pC
		float d;    // cm
		float t;     // ns

		void Summarize(JObjectSummary& summary) const override {
                    summary.add(layer, "layer", "%d");
                    summary.add(wire, "wire", "%d");
                    summary.add(dE*1.0E6, "dE(keV)", "%3.1f");
                    summary.add(q, "q", "%3.2f");
                    summary.add(d, "d", "%3.2f");
                    summary.add(t, "t", "%3.3f");
		}

};

#endif // _DFMWPCTruthHit_

