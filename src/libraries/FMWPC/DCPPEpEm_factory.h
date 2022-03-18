// $Id$
//
//    File: DCPPEpEm_factory.h
// Created: Thu Mar 17 14:49:42 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DCPPEpEm_factory_
#define _DCPPEpEm_factory_

#include <JANA/JFactory.h>
#include "DCPPEpEm.h"
#include <TRACKING/DTrackTimeBased.h>
#include <PID/DNeutralParticleHypothesis.h>
#include <PID/DBeamPhoton.h>
#include <PID/DChargedTrack.h>
#include <PID/DNeutralParticle.h>
#include <KINFITTER/DKinFitter.h>
#include <ANALYSIS/DKinFitUtils_GlueX.h>

class DCPPEpEm_factory:public jana::JFactory<DCPPEpEm>{
public:
DCPPEpEm_factory(){};
~DCPPEpEm_factory(){};

private:
jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

void DoKinematicFit(const DBeamPhoton *beamphoton,
		      const DTrackTimeBased *negative,
		      const DTrackTimeBased *positive,
		      DKinFitUtils_GlueX *dKinFitUtils,
		      DKinFitter *dKinFitter,
		      DAnalysisUtilities *dAnalysisUtilities) const;
bool VetoNeutrals(double t0_rf,const DVector3 &vect,
		    vector<const DNeutralParticle*>&neutrals) const;

double SPLIT_CUT,FCAL_THRESHOLD,BCAL_THRESHOLD,GAMMA_DT_CUT;
};

#endif // _DCPPEpEm_factory_

