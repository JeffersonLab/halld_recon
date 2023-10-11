// $Id: PID_init.cc 2433 2007-04-07 14:57:32Z kornicer $

#include <JANA/JFactorySet.h>
#include <JANA/Compatibility/JGetObjectsFactory.h>

#include "DBeamPhoton_factory.h"
#include "DBeamPhoton_factory_TRUTH.h"
#include "DBeamPhoton_factory_TAGGEDMCGEN.h"
#include "DBeamPhoton_factory_MCGEN.h"
#include "DParticleID_factory.h"
#include "DParticleID_factory_PID1.h"
#include "DChargedTrack_factory.h"
#include "DChargedTrack_factory_PreSelect.h"
#include "DChargedTrackHypothesis_factory.h"
#include "DNeutralParticle_factory.h"
#include "DNeutralParticle_factory_PreSelect.h"
#include "DNeutralParticleHypothesis_factory.h"
#include "DNeutralShower_factory.h"
#include "DNeutralShower_factory_PreSelect.h"
#include "DNeutralShower_factory_HadronPreSelect.h"
#include "DVertex_factory.h"
#include "DVertex_factory_THROWN.h"
#include "DEventRFBunch_factory.h"
#include "DEventRFBunch_factory_Thrown.h"
#include "DEventRFBunch_factory_Calibrations.h"
#include "DEventRFBunch_factory_CalorimeterOnly.h"
#include "DDetectorMatches_factory.h"
#include "DDetectorMatches_factory_WireBased.h"
#include "DMCThrown_factory_FinalState.h"
#include "DMCThrown_factory_Decaying.h"
#include "DMCThrown_factory_Primary.h"


#define UC_CLUSTERIZER

void PID_init(JFactorySet *factorySet)
{
	/// Create and register PID data factories
	factorySet->Add(new JGetObjectsFactory<DMCReaction>());
	factorySet->Add(new DBeamPhoton_factory);
	factorySet->Add(new DBeamPhoton_factory_TRUTH);
	factorySet->Add(new DBeamPhoton_factory_TAGGEDMCGEN);
	factorySet->Add(new DBeamPhoton_factory_MCGEN);
	factorySet->Add(new DParticleID_factory);
	factorySet->Add(new DParticleID_factory_PID1);
	factorySet->Add(new DChargedTrack_factory);
	factorySet->Add(new DChargedTrack_factory_PreSelect);
	factorySet->Add(new DChargedTrackHypothesis_factory);
	factorySet->Add(new DNeutralParticle_factory);
	factorySet->Add(new DNeutralParticle_factory_PreSelect);
	factorySet->Add(new DNeutralParticleHypothesis_factory);
	factorySet->Add(new DNeutralShower_factory);
	factorySet->Add(new DNeutralShower_factory_PreSelect);
	factorySet->Add(new DNeutralShower_factory_HadronPreSelect);
	factorySet->Add(new DVertex_factory);
	factorySet->Add(new DVertex_factory_THROWN);
	factorySet->Add(new DEventRFBunch_factory);
	factorySet->Add(new DEventRFBunch_factory_Thrown);
	factorySet->Add(new DEventRFBunch_factory_Calibrations);
	factorySet->Add(new DEventRFBunch_factory_CalorimeterOnly);
	factorySet->Add(new DDetectorMatches_factory);
	factorySet->Add(new DDetectorMatches_factory_WireBased);
	factorySet->Add(new DMCThrown_factory_FinalState);
	factorySet->Add(new DMCThrown_factory_Decaying);
	factorySet->Add(new DMCThrown_factory_Primary);
}
