// $Id$
//
//    File: DBCALSiPMHit.h
// Created: Fri May 13 11:06:38 EDT 2011
// Creator: davidl (on Darwin eleanor.jlab.org 10.7.0 i386)
//

#ifndef _DBCALSiPMHit_
#define _DBCALSiPMHit_

#include "BCAL/DBCALGeometry.h"

#include <JANA/JObject.h>

// WARNING: This class represents the SiPM hits and is intended
// for debugging of simulated data only. The information contained
// here may not be available in real data (use the DBCALHit objects
// for that).
//
// Objects of this class hold hold data from the bcalSiPMUpHit and
// bcalSiPMDownHit structures from HDDM

class DBCALSiPMHit: public JObject {
	public:
		JOBJECT_PUBLIC(DBCALSiPMHit);

		DBCALSiPMHit(){}
		virtual ~DBCALSiPMHit(){}
		
		int module;
		int layer;
		int sector;
		DBCALGeometry::End end;
		float E;
		float t;
		
		int cellId;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(cellId, "cellId", "%d");
			summary.add(module, "module", "%d");
			summary.add(layer, "layer", "%d");
			summary.add(sector, "sector", "%d");
			summary.add(end==0 ? "upstream":"downstream" , "end", "%s");
			summary.add(E, "E(GeV)", "%2.3f");
			summary.add(t, "t(ns)", "%4.2f");
		}
};

#endif // _DBCALSiPMHit_

