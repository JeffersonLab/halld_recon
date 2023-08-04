// $Id$
//
//    File: DEventRFBunch_factory_KLong.cc
//

#include "DEventRFBunch_factory_KLong.h"
#include <deque>

using namespace jana;

//------------------
// init
//------------------
jerror_t DEventRFBunch_factory_KLong::init(void)
{
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventRFBunch_factory_KLong::brun(jana::JEventLoop *locEventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventRFBunch_factory_KLong::evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber)
{

    vector<const DVertex*> locVertices;
    locEventLoop->Get(locVertices);
    
    if(locVertices.size() == 0)
    	return NOERROR;
    	
    const DVertex *locVertex = locVertices[0];

	// build "RF bunch" for KLong beam based on reconstructed event time
	DEventRFBunch *locEventRFBunch = new DEventRFBunch;
	locEventRFBunch->dTime = locVertex->dSpacetimeVertex.T();
	locEventRFBunch->dTimeVariance = locVertex->dCovarianceMatrix(3,3);
	locEventRFBunch->dNumParticleVotes = locVertex->dKinFitNDF; /// this is an overcount! figure this out better
	locEventRFBunch->dTimeSource = SYS_NULL;   // could say SYS_START?  but I think that this implies that we should measure with respect the accelerator RF
	_data.push_back(locEventRFBunch);

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DEventRFBunch_factory_KLong::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventRFBunch_factory_KLong::fini(void)
{
	return NOERROR;
}

