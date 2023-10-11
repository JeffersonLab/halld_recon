// $Id$
//
//    File: DChargedTrack_factory.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DChargedTrack_factory.h"

#include "DANA/DObjectID.h"


//------------------
// Init
//------------------
void DChargedTrack_factory::Init()
{
}

//------------------
// BeginRun
//------------------
void DChargedTrack_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DChargedTrack_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DChargedTrackHypothesis*> locChargedTrackHypotheses;
	event->Get(locChargedTrackHypotheses);

	map<oid_t, vector<const DChargedTrackHypothesis*> > locHypothesesByTrackID;
	for(size_t loc_i = 0; loc_i < locChargedTrackHypotheses.size(); loc_i++)
		locHypothesesByTrackID[locChargedTrackHypotheses[loc_i]->Get_TrackTimeBased()->candidateid].push_back(locChargedTrackHypotheses[loc_i]);

	map<oid_t, vector<const DChargedTrackHypothesis*> >::iterator locIterator = locHypothesesByTrackID.begin();
	for(; locIterator != locHypothesesByTrackID.end(); ++locIterator)
	{
		DChargedTrack* locChargedTrack = new DChargedTrack();
		locChargedTrack->candidateid = locIterator->first;
		locChargedTrack->dChargedTrackHypotheses = locIterator->second;
		Insert(locChargedTrack);
	}
}

//------------------
// EndRun
//------------------
void DChargedTrack_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DChargedTrack_factory::Finish()
{
}


