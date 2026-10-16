// $Id$
//
//    File: DBeamPhoton_factory_TRUTH.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include <iostream>
#include <iomanip>
using namespace std;

#include "DBeamPhoton_factory_TRUTH.h"

#include "DANA/DEvent.h"


//------------------
// Init
//------------------
void DBeamPhoton_factory_TRUTH::Init()
{
}

//------------------
// BeginRun
//------------------
void DBeamPhoton_factory_TRUTH::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	DGeometry* locGeometry = DEvent::GetDGeometry(event);
	dTargetCenterZ = 0.0;
	locGeometry->GetTargetZ(dTargetCenterZ);
}

//------------------
// Process
//------------------
void DBeamPhoton_factory_TRUTH::Process(const std::shared_ptr<const JEvent>& event)
{
	DVector3 pos(0.0, 0.0, dTargetCenterZ);

	vector<const DTAGMHit*> tagm_hits;
	event->Get(tagm_hits, "TRUTH");
	for (unsigned int ih=0; ih < tagm_hits.size(); ++ih)
	{
		if (tagm_hits[ih]->row > 0) continue;
		DVector3 mom(0.0, 0.0, tagm_hits[ih]->E);
		DBeamPhoton *gamma = new DBeamPhoton;
		gamma->setPID(Gamma);
		gamma->setMomentum(mom);
		gamma->setPosition(pos);
		gamma->setTime(tagm_hits[ih]->t);
		gamma->dSystem = SYS_TAGM;
		gamma->dCounter = tagm_hits[ih]->column;
		gamma->AddAssociatedObject(tagm_hits[ih]);
		Insert(gamma);
	}

	vector<const DTAGHHit*> tagh_hits;
	event->Get(tagh_hits, "TRUTH");
	for (unsigned int ih=0; ih < tagh_hits.size(); ++ih)
	{
		DVector3 mom(0.0, 0.0, tagh_hits[ih]->E);
		DBeamPhoton *gamma = new DBeamPhoton;
		gamma->setPID(Gamma);
		gamma->setMomentum(mom);
		gamma->setPosition(pos);
		gamma->setTime(tagh_hits[ih]->t);
		gamma->dSystem = SYS_TAGH;
		gamma->dCounter = tagh_hits[ih]->counter_id;
		gamma->AddAssociatedObject(tagh_hits[ih]);
		Insert(gamma);
	}

	return;
}

//------------------
// EndRun
//------------------
void DBeamPhoton_factory_TRUTH::EndRun()
{
}

//------------------
// Finish
//------------------
void DBeamPhoton_factory_TRUTH::Finish()
{
}

