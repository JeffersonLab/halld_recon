// $Id$
//
//    File: DBCALHit.h
// Created: Thu Jun  9 10:14:35 EDT 2005
// Creator: davidl (on Darwin wire129.jlab.org 7.8.0 powerpc)
//

#ifndef _DBCALHit_
#define _DBCALHit_

#include "BCAL/DBCALGeometry.h"

#include <JANA/JObject.h>

class DBCALHit: public JObject {

	/// This class holds data originating from the fADC250
	/// modules connected to the BCAL

	public:
		JOBJECT_PUBLIC(DBCALHit);
		
		int module;
		int layer;
		int sector;
		DBCALGeometry::End end;
        int pulse_peak; // Pedestal subtracted pulse peak (per event)
		float E;
		float t;
		float t_raw;    ///< Uncalibrated time in ns
		
		int cellId;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(module, "module", "%d");
			summary.add(layer, "layer", "%d");
			summary.add(sector, "sector", "%d");
			summary.add(end==0 ? "upstream":"downstream" , "end", "%s");
			summary.add(E, "E(GeV)", "%6.3f");
			summary.add(t, "t(ns)", "%4.2f");
		}
};

#endif // _DBCALHit_

