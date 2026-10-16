// $Id$
//
//    File: DMCTrajectoryPoint.h
// Created: Mon Jun 12 09:29:58 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.6.0 powerpc)
//

#ifndef _DMCTrajectoryPoint_
#define _DMCTrajectoryPoint_

#include "JANA/JObject.h"

class DMCTrajectoryPoint: public JObject {
	public:
		JOBJECT_PUBLIC(DMCTrajectoryPoint);
		
		float x,y,z,t;
		float px,py,pz;
		float E, dE;
		int primary_track;
		int track;
		int part;
		float radlen;
		float step;
		int mech;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(x, "x", "%1.3f");
			summary.add(y, "y", "%1.3f");
			summary.add(z, "z", "%1.3f");
			summary.add(t/1.0E-9, "t", "%1.3f");
			summary.add(px, "px", "%1.3f");
			summary.add(py, "py", "%1.3f");
			summary.add(pz, "pz", "%1.3f");
			summary.add(E, "E", "%1.3f");
			summary.add(1000.0*dE, "dE(MeV)", "%1.3f");
			summary.add(primary_track, "primary", "%d");
			summary.add(track, "track", "%d");
			summary.add(part, "part", "%d");
			summary.add(radlen, "radlen", "%1.3f");
			summary.add(step, "step", "%1.3f");
			summary.add(mech, "mech", "%d");
		}
};

#endif // _DMCTrajectoryPoint_

