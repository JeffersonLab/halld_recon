// $Id$
//
//    File: DEventProcessor_fcal_hists.cc
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#include <map>
using namespace std;

#include "DEventProcessor_fcal_hists.h"

#include <TLorentzVector.h>

#include <DANA/DEvent.h>
#include <FCAL/DFCALCluster.h>
#include <TRACKING/DMCThrown.h>
#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALGeometry.h>


// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new DEventProcessor_fcal_hists());
}
} // "C"


#define FCAL_Z_OFFSET 640.0-65.0 // I don't know what this value is ???
//#define FCAL_Z_OFFSET 170.0 // I don't know what this value is ???
#define PI_ZERO_MASS 0.13497

//------------------
// Init
//------------------
void DEventProcessor_fcal_hists::Init()
{
	
	dE_over_E_vs_E = new TH2D("dE_over_E_vs_E","Smeared-unsmeared energy diff for single FCAL blocks", 50, 0.0, 1.0, 50, -0.1, 0.1);
}

//------------------
// BeginRun
//------------------
void DEventProcessor_fcal_hists::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DEventProcessor_fcal_hists::Process(const std::shared_ptr<const JEvent>& event)
{
	// extract the FCAL Geometry (for isBlockActive() and positionOnFace())
	vector<const DFCALGeometry*> fcalGeomVect;
	vector<const DFCALHit*> hits;
	vector<const DFCALHit*> truthhits;
	event->Get(fcalGeomVect);
	event->Get(hits);
	event->Get(truthhits,"TRUTH");

	const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
	if(fcalGeomVect.size()<1) throw JException("FCAL geometry not available!");
		
	// Create STL map of "real" hits that can be used below to match 
	// with MC hits.
	map<int,const DFCALHit*> hit_map;
	for(unsigned int i=0; i<hits.size(); i++){
		const DFCALHit *hit = hits[i];

		int channel = fcalGeom.channel(hit->row, hit->column);
		hit_map[channel] = hit;
	}
	
	// Since we are modifying histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
	GetLockService(event)->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

	// Loop over "truth" hits and match them to real hits by using assuming
	// one hit per channel
	for(unsigned int i=0; i<truthhits.size(); i++){
		const DFCALHit *truthhit = truthhits[i];
		map<int,const DFCALHit*>::iterator iter = hit_map.find(fcalGeom.channel(truthhit->row, truthhit->column));
		if(iter==hit_map.end())continue;
		
		const DFCALHit *hit = iter->second;
		double deltaE = hit->E - truthhit->E;
		dE_over_E_vs_E->Fill(truthhit->E, deltaE/truthhit->E);
	}

	GetLockService(event)->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void DEventProcessor_fcal_hists::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// Finish
//------------------
void DEventProcessor_fcal_hists::Finish()
{
}

