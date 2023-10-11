// $Id$
//
//    File: DTrackWireBased_factory_THROWN.h
// Created: Mon Sep  3 19:57:11 EDT 2007
// Creator: davidl (on Darwin Amelia.local 8.10.1 i386)
//

#ifndef _DTrackWireBased_factory_THROWN_
#define _DTrackWireBased_factory_THROWN_

#include <JANA/JFactoryT.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <HDGEOMETRY/DRootGeom.h>
#include <HDGEOMETRY/DGeometry.h>
#include <HDGEOMETRY/DMagneticFieldMap.h>
#include "DTrackWireBased.h"
#include "PID/DParticleID.h"

class DTrackFitter;
class DTrackHitSelector;
class DParticleID;

class DTrackWireBased_factory_THROWN:public JFactoryT<DTrackWireBased>{
	public:
		DTrackWireBased_factory_THROWN();
		~DTrackWireBased_factory_THROWN(){};

	private:
		//void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		//void EndRun() override;
		//void Finish() override;
		
		DTrackFitter *fitter;
		const DTrackHitSelector *hitselector;
		const DParticleID* dParticleID;
		vector<DReferenceTrajectory*> rt_pool;
	
		DRootGeom *RootGeom;
		DGeometry *geom;
		DMagneticFieldMap *bfield;
};

#endif // _DTrackWireBased_factory_THROWN_

