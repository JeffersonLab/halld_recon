// $Id$
//
//    File: DTRDHit_factory.cc
//

#include <iostream>
#include <iomanip>
using namespace std;

#include "DANA/DEvent.h"

#include <DAQ/Df125Config.h>
#include <DAQ/Df125FDCPulse.h>

#include "TRD/DTRDHit_factory.h"


//------------------
// Init
//------------------
void DTRDHit_factory::Init()
{
	// Initialize calibration tables
	//vector<double> new_t0s(TRD_MAX_CHANNELS);	
	//time_offsets.push_back(new_t0s); time_offsets.push_back(new_t0s);

	auto app = GetApplication();

	IS_XY_TIME_DIFF_CUT = false;
	app->SetDefaultParameter("TRD:IS_XY_TIME_DIFF_CUT", IS_XY_TIME_DIFF_CUT, 
			      "Apply time difference cut between X and Y hits (default: false)");

	XY_TIME_DIFF = 20.;
	app->SetDefaultParameter("TRD:XY_TIME_DIFF", XY_TIME_DIFF, 
			      "Time difference between hits in X and Y planes to be considered a coincidence (default: 20.)");

	return;
}

//------------------
// BeginRun
//------------------
void DTRDHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{

    return;
}

//------------------
// Process
//------------------
void DTRDHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
    vector<const DTRDHit*> hits;
    event->Get(hits, "Calib");
    
    if(!IS_XY_TIME_DIFF_CUT) {
    	for(auto &hit : hits)
    		Insert(const_cast<DTRDHit*>(hit));
    	return;
    }

	vector<DTRDHit*> hits_plane[2]; // one for each plane
    
	// loops to check the time coincidence of hits in the two planes and add them to the _data vector
	for (unsigned int i=0; i < hits_plane[0].size(); i++) {
		for (unsigned int j=0; j < hits_plane[1].size(); j++) {
			if (abs(hits_plane[0][i]->t - hits_plane[1][j]->t) < XY_TIME_DIFF) {
				Insert(hits_plane[0][i]);
				break;
			}
		}
	}

	for (unsigned int i=0; i < hits_plane[1].size(); i++) {
		for (unsigned int j=0; j < hits_plane[0].size(); j++) {
			if (abs(hits_plane[1][i]->t - hits_plane[0][j]->t) < XY_TIME_DIFF) {
				Insert(hits_plane[1][i]);
				break;
			}
		}
	}	
		    
    return;
}

//------------------
// EndRun
//------------------
void DTRDHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DTRDHit_factory::Finish()
{
}
