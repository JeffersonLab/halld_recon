// $Id$
//
//

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include "DBeamKLong_factory.h"

#include "DVertex.h"
#include "particleType.h"
using namespace jana;


//------------------
// init
//------------------
jerror_t DBeamKLong_factory::init(void)
{

	//Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory.
	SetFactoryFlag(NOT_OBJECT_OWNER);
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DBeamKLong_factory::brun(jana::JEventLoop *locEventLoop, int32_t runnumber)
{
    DApplication* dapp = dynamic_cast<DApplication*>(locEventLoop->GetJApplication());
    DGeometry* locGeometry = dapp->GetDGeometry(locEventLoop->GetJEvent().GetRunNumber());
    dTargetCenterZ = 0.0;
    locGeometry->GetTargetZ(dTargetCenterZ);

    return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DBeamKLong_factory::evnt(jana::JEventLoop *locEventLoop, uint64_t locEventNumber)
{
// 	dResourcePool_BeamPhotons->Recycle(dCreated);
// 	dCreated.clear();
// 	_data.clear();

    vector<const DVertex*> locVertices;
    locEventLoop->Get(locVertices, "KLong");
    
    if(locVertices.size() == 0)
    	return NOERROR;
    	
    const DVertex *locVertex = locVertices[0];
    
    // distance between KPT and dTargetCenterZ is assumed to be 24 m
	const double c = 29.9792;
    double KL_distance = ( locVertex->dSpacetimeVertex.Z() - dTargetCenterZ ) + ( 24. * 100. );
	double KL_propagation_time = locVertex->dSpacetimeVertex.T();  // should be wrt DRFTime
    double beta = (KL_distance / KL_propagation_time) / c;
    
    if(beta > 1.)
    	return NOERROR;
    
//     cout << "in DBeamKLong_factory::evnt() ..." << endl;
//     cout << KL_distance << " " << locVertex->dSpacetimeVertex.Z() << " " << dTargetCenterZ << endl;
//     cout << KL_propagation_time << " " << beta << endl;
    
	double momentum_mag = (ParticleMass(KLong)*beta) / (sqrt(1. - beta*beta)); // change c units from cm/ns -> m/s

	// make beam particle
	DBeamKLong* beam = new DBeamKLong;
    DVector3 mom(0.0, 0.0, momentum_mag);
    beam->setPID(KLong);
    beam->setMomentum(mom);
    beam->setPosition(locVertex->dSpacetimeVertex.Vect());
    beam->setTime(KL_propagation_time);

// 	auto locCovarianceMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
// 	locCovarianceMatrix->ResizeTo(7, 7);
// 	locCovarianceMatrix->Zero();
// 	beam->setErrorMatrix(locCovarianceMatrix);
// 
	_data.push_back(beam);

    return NOERROR;
}

