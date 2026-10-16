// $Id$
//
//    File: DVertex_factory_THROWN.cc
// Created: Tue Apr  6 17:01:54 EDT 2010
// Creator: pmatt (on Darwin Amelia.local 9.8.0 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include <JANA/JEvent.h>
#include <TROOT.h>
#include <TMath.h>
#include "DVertex_factory_THROWN.h"


//------------------
// Init
//------------------
void DVertex_factory_THROWN::Init()
{
}

//------------------
// BeginRun
//------------------
void DVertex_factory_THROWN::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DVertex_factory_THROWN::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DMCThrown*> locThrownTracks;
	event->Get(locThrownTracks);

	if(locThrownTracks.size() == 0)
		return; // RESOURCE_UNAVAILABLE;

	DVertex* locVertex = new DVertex;
	locVertex->dKinFitNDF = 0;
	locVertex->dKinFitChiSq = 0.0;
	locVertex->dSpacetimeVertex.SetVect(locThrownTracks[0]->position());
	locVertex->dSpacetimeVertex.SetT(locThrownTracks[0]->time());
	
	Insert(locVertex);
}

//------------------
// EndRun
//------------------
void DVertex_factory_THROWN::EndRun()
{
}

//------------------
// Finish
//------------------
void DVertex_factory_THROWN::Finish()
{
}

