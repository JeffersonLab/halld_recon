// $Id: DBCALIncidentParticle.h 9441 2012-08-06 21:56:11Z davidl $
//
//    File: DBCALIncidentParticle.h
// Created: Thu Jun  9 10:14:35 EDT 2005
// Creator: davidl (on Darwin wire129.jlab.org 7.8.0 powerpc)
//

#ifndef _DBCALIncidentParticle_
#define _DBCALIncidentParticle_

#include "BCAL/DBCALGeometry.h"

#include <JANA/JObject.h>

class DBCALIncidentParticle: public JObject {

	/// This class holds data originating from the fADC250
	/// modules connected to the BCAL

	public:
		JOBJECT_PUBLIC(DBCALIncidentParticle);
		
		int ptype;
		float px;
		float py;
		float pz;
		float x;
		float y;
		float z;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(ptype, "ptype", "%d");
			summary.add(px, "px[GeV]", "%7.5f");
			summary.add(py, "py[GeV]", "%7.5f");
			summary.add(pz, "pz[GeV]", "%7.5f");
			summary.add(x, "x[GeV]", "%7.5f");
			summary.add(y, "y[GeV]", "%7.5f");
			summary.add(z, "z[GeV]", "%7.5f");
		}
};

#endif // _DBCALIncidentParticle_

