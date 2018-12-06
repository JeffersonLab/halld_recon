/*
 *  File: DCCALHit_factory.cc
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */

#include <iostream>
#include <fstream>
#include <math.h>
#include <DVector3.h>
using namespace std;

#include <JANA/JEvent.h>
using namespace jana;

#include "CCAL/DCCALCluster.h"
#include "CCAL/DCCALCluster_factory.h"
#include "CCAL/DCCALHit.h"
#include "CCAL/DCCALGeometry.h"
#include "HDGEOMETRY/DGeometry.h"

//----------------
// Constructor
//----------------
DCCALCluster_factory::DCCALCluster_factory()
{
	// Set defaults
        MIN_CLUSTER_BLOCK_COUNT   =  2;
        MIN_CLUSTER_SEED_ENERGY   =  0.035; // GeV
	TIME_CUT                  =  15.0 ; //ns
	MAX_HITS_FOR_CLUSTERING   =  200;

	gPARMS->SetDefaultParameter("CCAL:MIN_CLUSTER_BLOCK_COUNT", MIN_CLUSTER_BLOCK_COUNT);
	gPARMS->SetDefaultParameter("CCAL:MIN_CLUSTER_SEED_ENERGY", MIN_CLUSTER_SEED_ENERGY);
	gPARMS->SetDefaultParameter("CCAL:MAX_HITS_FOR_CLUSTERING", MAX_HITS_FOR_CLUSTERING);
	gPARMS->SetDefaultParameter("CCAL:TIME_CUT",TIME_CUT,"time cut for associating CCAL hits together into a cluster");

}

//------------------
// brun
//------------------
jerror_t DCCALCluster_factory::brun(JEventLoop *eventLoop, int32_t runnumber)
{

	return NOERROR;
}


jerror_t DCCALCluster_factory::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{
  
         vector<const DCCALHit*> ccalhits;
	eventLoop->Get(ccalhits);
	
	if(ccalhits.size() > MAX_HITS_FOR_CLUSTERING) return NOERROR;
	
	double tot_en = 0;
	
	for (unsigned int i = 0; i < ccalhits.size(); i++) {	  
	  const DCCALHit *ccalhit = ccalhits[i];
	
	  tot_en += ccalhit->E;
	} 
	
	
	// Build hit object
	DCCALCluster *cluster = new DCCALCluster;
	
	cluster->fEnergy = tot_en;
	cluster->fTime   = 10;
	
	_data.push_back( cluster );
	
	
	return NOERROR;
	
}

