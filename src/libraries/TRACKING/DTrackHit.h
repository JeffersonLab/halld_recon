// $Id$
//
//    File: DTrackHit.h
// Created: Tue Aug 23 05:00:03 EDT 2005
// Creator: davidl (on Darwin Harriet.local 7.8.0 powerpc)
//

#ifndef _DTrackHit_
#define _DTrackHit_

#include <cmath>

#include <DMatrix.h>

#include <JANA/JObject.h>

#include "GlueX.h"

class DTrackHit:public JObject{
	public:
		JOBJECT_PUBLIC(DTrackHit);
		
		void InitCovarianceMatrix(void);

		float x,y,z,r,phi;
		DetectorSystem_t system;
		DMatrix cov; // covariance matrix rotated into lab x,y,z

		void Summarize(JObjectSummary& summary) const override {
			summary.add(x, "x(cm)", "%3.1f");
			summary.add(y, "y(cm)", "%3.1f");
			summary.add(z, "z(cm)", "%3.1f");
			summary.add(r, "r(cm)", "%3.1f");
			summary.add(phi*180.0/M_PI, "phi(deg)", "%3.1f");
			summary.add(SystemName(system), "system", "%s");
		}
};

#endif // _DTrackHit_

