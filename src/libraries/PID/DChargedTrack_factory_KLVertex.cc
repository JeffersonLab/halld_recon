// $Id$
//
//    File: DChargedTrack_factory_KLVertex.cc
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DChargedTrack_factory_KLVertex.h"
using namespace jana;

//------------------
// init
//------------------
jerror_t DChargedTrack_factory_KLVertex::init(void)
{
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DChargedTrack_factory_KLVertex::brun(jana::JEventLoop *locEventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DChargedTrack_factory_KLVertex::evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber)
{
	vector<const DChargedTrackHypothesis*> locChargedTrackHypotheses;
	locEventLoop->Get(locChargedTrackHypotheses, "KLVertex");

	map<JObject::oid_t, vector<const DChargedTrackHypothesis*> > locHypothesesByTrackID;
	for(size_t loc_i = 0; loc_i < locChargedTrackHypotheses.size(); loc_i++)
		locHypothesesByTrackID[locChargedTrackHypotheses[loc_i]->Get_TrackTimeBased()->candidateid].push_back(locChargedTrackHypotheses[loc_i]);

	map<JObject::oid_t, vector<const DChargedTrackHypothesis*> >::iterator locIterator = locHypothesesByTrackID.begin();
	for(; locIterator != locHypothesesByTrackID.end(); ++locIterator)
	{
		DChargedTrack* locChargedTrack = new DChargedTrack();
		locChargedTrack->candidateid = locIterator->first;
		locChargedTrack->dChargedTrackHypotheses = locIterator->second;
		_data.push_back(locChargedTrack);
	}

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DChargedTrack_factory_KLVertex::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DChargedTrack_factory_KLVertex::fini(void)
{
	return NOERROR;
}


