// $Id$
//
//    File: DTrackTimeBased_factory_THROWN.h
// Created: MWed Nov 18 06:25:19 EST 2009
// Creator: davidl (on Darwin Amelia.local 9.8.0 i386)
//

#ifndef _DTrackTimeBased_factory_THROWN_
#define _DTrackTimeBased_factory_THROWN_

#include <JANA/JFactoryT.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <HDGEOMETRY/DRootGeom.h>
#include <HDGEOMETRY/DGeometry.h>
#include <HDGEOMETRY/DMagneticFieldMap.h>
#include "DTrackTimeBased.h"
#include "PID/DParticleID.h"

class DTrackFitter;
class DTrackHitSelector;
class DParticleID;

class DTrackTimeBased_factory_THROWN:public JFactoryT<DTrackTimeBased>{
	public:
		DTrackTimeBased_factory_THROWN();
		~DTrackTimeBased_factory_THROWN(){};

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

#endif // _DTrackTimeBased_factory_THROWN_

