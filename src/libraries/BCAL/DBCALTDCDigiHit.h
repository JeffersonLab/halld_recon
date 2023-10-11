// $Id$
//
//    File: DBCALTDCDigiHit.h
// Created: Tue Aug  6 11:04:26 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DBCALTDCDigiHit_
#define _DBCALTDCDigiHit_

#include <JANA/JObject.h>

#include <BCAL/DBCALGeometry.h>

class DBCALTDCDigiHit: public JObject{
	public:
		JOBJECT_PUBLIC(DBCALTDCDigiHit);
	
		uint32_t module;
		uint32_t layer;
		uint32_t sector;
		DBCALGeometry::End end;
		uint32_t time;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(module, NAME_OF(module), "%d");
			summary.add(layer, NAME_OF(layer), "%d");
			summary.add(sector, NAME_OF(sector), "%d");
			summary.add((end==0 ? "upstream":"downstream"), NAME_OF(end), "%s");
			summary.add(time, NAME_OF(time), "%d");
		}
};

#endif // _DBCALTDCDigiHit_

