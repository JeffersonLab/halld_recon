// $Id$
//
//    File: DBCALCluster_factory_SINGLE.cc
// Created: Fri Sep  7 12:13:07 EDT 2012
// Creator: davidl (on Darwin harriet.jlab.org 11.4.0 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DBCALCluster_factory_SINGLE.h"
#include <BCAL/DBCALPoint.h>

#include <JANA/JEvent.h>
#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"

//------------------
// Init
//------------------
void DBCALCluster_factory_SINGLE::Init()
{
}

//------------------
// BeginRun
//------------------
void DBCALCluster_factory_SINGLE::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	auto run_number = event->GetRunNumber();
	auto app = GetApplication();
	auto geom = app->GetService<DGeometryManager>()->GetDGeometry(run_number);
	geom->GetTargetZ(m_z_target_center);

	// load BCAL geometry
	vector<const DBCALGeometry *> BCALGeomVec;
	event->Get(BCALGeomVec);
	if(BCALGeomVec.size() == 0)
		throw JException("Could not load DBCALGeometry object!");
	dBCALGeom = BCALGeomVec[0];

}

//------------------
// Process
//------------------
void DBCALCluster_factory_SINGLE::Process(const std::shared_ptr<const JEvent>& event)
{
	// Get DBCALPoint objects
	vector<const DBCALPoint*> bcalpoints;
	event->Get(bcalpoints);
	
	// Need at least one DBCALPoint object to make a cluster
	if(bcalpoints.size() == 0) return;
	
	// Create DBCALCluster object and add all DBCALPoint objects to it
	DBCALCluster *cluster = new DBCALCluster(m_z_target_center, dBCALGeom);
	int q = 0;
	for(unsigned int i=0; i<bcalpoints.size(); i++){
		cluster->addPoint(bcalpoints[i], q);
	}
	
	// Store in _data so it is published to JANA
	Insert(cluster);
}

//------------------
// EndRun
//------------------
void DBCALCluster_factory_SINGLE::EndRun()
{
}

//------------------
// Finish
//------------------
void DBCALCluster_factory_SINGLE::Finish()
{
}

