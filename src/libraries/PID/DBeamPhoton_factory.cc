// $Id$
//
//    File: DBeamPhoton_factory.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include "DBeamPhoton_factory.h"

#include <JANA/JEvent.h>
#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"


inline bool DBeamPhoton_SortByTime(const DBeamPhoton* locBeamPhoton1, const DBeamPhoton* locBeamPhoton2)
{
	// pseudo-randomly sort beam photons by time, using only sub-picosecond digits
	// do this by converting to picoseconds, then stripping all digits above the decimal place

	//guard against NaN
	if(std::isnan(locBeamPhoton1->time()))
		return false;
	if(std::isnan(locBeamPhoton2->time()))
		return true;
	double locT1_Picoseconds_Stripped = 1000.0*locBeamPhoton1->time() - floor(1000.0*locBeamPhoton1->time());
	double locT2_Picoseconds_Stripped = 1000.0*locBeamPhoton2->time() - floor(1000.0*locBeamPhoton2->time());
	return (locT1_Picoseconds_Stripped < locT2_Picoseconds_Stripped);
}

//------------------
// Init
//------------------
void DBeamPhoton_factory::Init()
{
    auto app = GetApplication();
    DELTA_T_DOUBLES_MAX = 1.5; // ns
    app->SetDefaultParameter("BeamPhoton:DELTA_T_DOUBLES_MAX", DELTA_T_DOUBLES_MAX,
    "Maximum time difference in ns between a TAGM-TAGH pair of beam photons"
    " for them to be merged into a single photon");
    DELTA_E_DOUBLES_MAX = 0.05; // GeV
    app->SetDefaultParameter("BeamPhoton:DELTA_E_DOUBLES_MAX", DELTA_E_DOUBLES_MAX,
    "Maximum energy difference in GeV between a TAGM-TAGH pair of beam photons"
    " for them to be merged into a single photon");

	//Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory.
	SetFactoryFlag(NOT_OBJECT_OWNER);
}

//------------------
// BeginRun
//------------------
void DBeamPhoton_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    auto runnumber = event->GetRunNumber();
    auto app = event->GetJApplication();
    auto geo_manager = app->GetService<DGeometryManager>();
    auto locGeometry = geo_manager->GetDGeometry(runnumber);

    dTargetCenterZ = 0.0;
    locGeometry->GetTargetZ(dTargetCenterZ);
}

//------------------
// Process
//------------------
void DBeamPhoton_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	auto locEventNumber = event->GetEventNumber();
	dResourcePool_BeamPhotons->Recycle(dCreated);
	dCreated.clear();
	mData.clear();

    vector<const DTAGMHit*> tagm_hits;
    event->Get(tagm_hits);

    for (unsigned int ih=0; ih < tagm_hits.size(); ++ih)
    {
        if (!tagm_hits[ih]->has_fADC) continue; // Skip TDC-only hits (i.e. hits with no ADC info.)
        if (tagm_hits[ih]->row > 0) continue; // Skip individual fiber readouts
        DBeamPhoton* gamma = Get_Resource();

        Set_BeamPhoton(gamma, tagm_hits[ih], locEventNumber);
        Insert(gamma);
    }

    vector<const DTAGHHit*> tagh_hits;
    event->Get(tagh_hits);

    for (unsigned int ih=0; ih < tagh_hits.size(); ++ih)
    {
        if (!tagh_hits[ih]->has_fADC) continue; // Skip TDC-only hits (i.e. hits with no ADC info.)
        if (!tagh_hits[ih]->has_TDC) continue;  // Skip fADC-only hits (i.e. hits with no TDC info.)
        DBeamPhoton* gamma = Get_Resource();

	Set_BeamPhoton(gamma, tagh_hits[ih], locEventNumber);
        Insert(gamma);
	   
    }

	sort(mData.begin(), mData.end(), DBeamPhoton_SortByTime);
	dCreated = mData;
}

void DBeamPhoton_factory::Set_BeamPhoton(DBeamPhoton* gamma, const DTAGMHit* hit, uint64_t locEventNumber)
{
    DVector3 pos(0.0, 0.0, dTargetCenterZ);
    DVector3 mom(0.0, 0.0, hit->E);
    gamma->setPID(Gamma);
    gamma->setMomentum(mom);
    gamma->setPosition(pos);
    gamma->setTime(hit->t);
    gamma->dCounter = hit->column;
    if(gamma->dCounter == 0)   // handle photons from simulation that miss tagger counters
    	gamma->dSystem = SYS_NULL;
    else
    	gamma->dSystem = SYS_TAGM;
    gamma->AddAssociatedObject(hit);

	auto locCovarianceMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
	locCovarianceMatrix->ResizeTo(7, 7);
	locCovarianceMatrix->Zero();
	gamma->setErrorMatrix(locCovarianceMatrix);
}

void DBeamPhoton_factory::Set_BeamPhoton(DBeamPhoton* gamma, const DTAGHHit* hit, uint64_t locEventNumber)
{
    DVector3 pos(0.0, 0.0, dTargetCenterZ);
    DVector3 mom(0.0, 0.0, hit->E);
    gamma->setPID(Gamma);
    gamma->setMomentum(mom);
    gamma->setPosition(pos);
    gamma->setTime(hit->t);
    gamma->dCounter = hit->counter_id;
    if(gamma->dCounter == 0)   // handle photons from simulation that miss tagger counters
    	gamma->dSystem = SYS_NULL;
    else
	    gamma->dSystem = SYS_TAGH;
    gamma->AddAssociatedObject(hit);

	auto locCovarianceMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
	locCovarianceMatrix->ResizeTo(7, 7);
	locCovarianceMatrix->Zero();
	gamma->setErrorMatrix(locCovarianceMatrix);
}
