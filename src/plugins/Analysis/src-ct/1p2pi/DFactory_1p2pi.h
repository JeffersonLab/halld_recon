// $Id$
//
//    File: DFactory_1p2pi.h
//

#ifndef _DFactory_1p2pi_
#define _DFactory_1p2pi_

#include <iostream>
#include <iomanip>

#include <JANA/JFactory.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

#include <TRIGGER/DTrigger.h>
#include <PID/DBeamPhoton.h>

#include "ANALYSIS/DAnalysisUtilities.h"
#include "ANALYSIS/DTreeInterface.h"
#include <KINFITTER/DKinFitter.h>
#include <ANALYSIS/DKinFitUtils_GlueX.h>

#include <PID/DChargedTrack.h>
#include <PID/DParticleID.h>

#include "D1p2piData.h"

using namespace std;
using namespace jana;

class DFactory_1p2pi : public jana::JFactory<D1p2piData>
{
	public:
		DFactory_1p2pi()
		{
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event. 
			//SetFactoryFlag(PERSISTANT);
		}
		const char* Tag(void){return "";}

	private:
		jerror_t brun(JEventLoop* locEventLoop, int32_t locRunNumber);
		jerror_t evnt(JEventLoop* locEventLoop, uint64_t locEventNumber);
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		double dBeamBunchPeriod;
		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks

		DKinFitUtils_GlueX *dKinFitUtils;
		DKinFitter *dKinFitter;
		const DAnalysisUtilities* dAnalysisUtilities;


  		void GetHypotheses(vector<const DChargedTrack *> &tracks,
		     map<Particle_t, int> &particles,
		     map<Particle_t, vector<const DChargedTrackHypothesis*> > &assignmentHypothesis,
		     vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > &hypothesisList
	        ) const;


};

#endif // _DFactory_1p2pi_

