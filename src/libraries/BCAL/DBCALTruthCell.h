// $Id$
//
//    File: DBCALTruthCell.h
// Created: Thu May  5 13:07:05 EDT 2011
// Creator: davidl (on Linux ifarm1101 2.6.18-128.7.1.el5 x86_64)
//

// This object represents the barrelEMcal->bcalCell->bcalHit
// structures from HDDM

#ifndef _DBCALTruthCell_
#define _DBCALTruthCell_

#include <JANA/JObject.h>

class DBCALTruthCell: public JObject {
	public:
		JOBJECT_PUBLIC(DBCALTruthCell);
		
		int module;
		int layer;
		int sector;
		double E;
		double t;
		double zLocal;
		
		void Summarize(JObjectSummary& summary) const override {
			summary.add(module, "module", "%2d");
			summary.add(layer, "layer", "%2d");
			summary.add(sector, "sector", "%1d");
			summary.add(E, "E", "%5.3f");
			summary.add(t, "t", "%7.2f");
			summary.add(zLocal, "zLocal", "%5.1f");
		}
		
};

#endif // _DBCALTruthCell_

