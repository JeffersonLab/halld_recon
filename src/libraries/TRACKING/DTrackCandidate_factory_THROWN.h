// $Id$
//
//    File: DTrackCandidate_factory_THROWN.h
// Created: Tue Dec 12 12:42:56 EST 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.8.0 powerpc)
//

#ifndef _DTrackCandidate_factory_THROWN_
#define _DTrackCandidate_factory_THROWN_

#include <JANA/JFactoryT.h>
#include <TRACKING/DReferenceTrajectory.h>
#include "DTrackCandidate.h"
#include "PID/DParticleID.h"

class DParticleID;
class DTrackFitter;
class DTrackHitSelector;

/// \htmlonly
/// <A href="index.html#legend">
///	<IMG src="ND.png" width="100">
///	</A>
/// \endhtmlonly

/// Generate DTrackCandiate objects based on the generated particles.
/// This uses "truth" information (information that won't be available
/// in read data) to make a list of more or less perfect track candidates.
/// Because tracks can multiple scatter or even hadronically scatter in
/// the target, the generated value may not always be correct. However,
/// on average, it is going to be very accurate.
///
/// This is used for debugging purposes in the tracking code and is not
/// normally used otherwise. 

class DTrackCandidate_factory_THROWN:public JFactoryT<DTrackCandidate>{
	public:
		DTrackCandidate_factory_THROWN();
		~DTrackCandidate_factory_THROWN() override = default;

	private:

		//void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		//void EndRun() override;
		//void Finish() override;

		DTrackFitter *fitter;
		const DTrackHitSelector *hitselector;
		vector<DReferenceTrajectory*> rt_pool;
		const DMagneticFieldMap *bfield;
		const DParticleID* dParticleID;

};


#endif // _DTrackCandidate_factory_THROWN_

