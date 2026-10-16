// $Id$
//
//    File: DBCALTDCHit.h
// Created: Thu Aug  2 11:40:16 EDT 2012
// Creator: davidl (on Darwin eleanor.jlab.org 10.8.0 i386)
//

#ifndef _DBCALTDCHit_
#define _DBCALTDCHit_

#include <BCAL/DBCALGeometry.h>

class DBCALTDCHit: public JObject {

	/// This class holds data originating from the F1TDC
	/// modules connected to the BCAL

	public:
		JOBJECT_PUBLIC(DBCALTDCHit);
		
		int module;
		int layer;
		int sector;
		DBCALGeometry::End end;
		float t;
		float t_raw;    ///< Uncalibrated time in ns

		int cellId;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(module, "module", "%d");
			summary.add(layer, "layer", "%d");
			summary.add(sector, "sector", "%d");
			summary.add(end==0 ? "upstream":"downstream" , "end", "%s");
			summary.add(t, "t(ns)", "%4.2f");
		}
};

#endif // _DBCALTDCHit_

