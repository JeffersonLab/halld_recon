// $Id$
//
//    File: DEventRFBunch_factory_KLong.cc
//

#include "DEventRFBunch_factory_KLong.h"
#include <deque>

#include "DANA/DEvent.h"

using namespace jana;

// //------------------
// // init
// //------------------
// void DEventRFBunch_factory_KLong::init(void)
// {
// }
// 
// //------------------
// // brun
// //------------------
// void DEventRFBunch_factory_KLong::brun(jana::JEventLoop *locEventLoop, int32_t runnumber)
// {
// }


DEventRFBunch_factory_KLong::DEventRFBunch_factory_KLong(void) { 
	SetTag("KLong"); 
}


//------------------
// Process
//------------------
void DEventRFBunch_factory_KLong::Process(const std::shared_ptr<const JEvent>& event) 
{

    vector<const DVertex*> locVertices;
    event->Get(locVertices);
    
    if(locVertices.size() == 0)
    	return;
    	
    const DVertex *locVertex = locVertices[0];

	// build "RF bunch" for KLong beam based on reconstructed event time
	DEventRFBunch *locEventRFBunch = new DEventRFBunch;
	locEventRFBunch->dTime = locVertex->dSpacetimeVertex.T();
	locEventRFBunch->dTimeVariance = locVertex->dCovarianceMatrix(3,3);
	locEventRFBunch->dNumParticleVotes = locVertex->dKinFitNDF; /// this is an overcount! figure this out better
	locEventRFBunch->dTimeSource = SYS_NULL;   // could say SYS_START?  but I think that this implies that we should measure with respect the accelerator RF

	locEventRFBunch->AddAssociatedObject(locVertices[0]);
	Insert(locEventRFBunch);

}

// //------------------
// // erun
// //------------------
// void DEventRFBunch_factory_KLong::erun(void)
// {
// }
// 
// //------------------
// // fini
// //------------------
// void DEventRFBunch_factory_KLong::fini(void)
// {
// }
// 
