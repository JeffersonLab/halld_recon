// $Id$
//
//    File: DTRDHit_factory.cc
//

#include <iostream>
#include <iomanip>
using namespace std;

#include "TRD/DTRDHit_factory.h"


//------------------
// Init
//------------------
void DTRDHit_factory::Init()
{
	// Initialize calibration tables
	//vector<double> new_t0s(TRD_MAX_CHANNELS);	
	//time_offsets.push_back(new_t0s); time_offsets.push_back(new_t0s);
}

//------------------
// BeginRun
//------------------
void DTRDHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	auto runnumber = event->GetRunNumber();

	// Only print messages for one thread whenever run number change
	static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
	static set<int> runs_announced;
	pthread_mutex_lock(&print_mutex);
	bool print_messages = false;
	if(runs_announced.find(runnumber) == runs_announced.end()){
		print_messages = true;
		runs_announced.insert(runnumber);
	}
	pthread_mutex_unlock(&print_mutex);
	
	if(print_messages) jout << "In DTRDHit_factory, loading constants..." << jendl;

	/*	
	// load base time offset
	map<string,double> base_time_offset;
	if (calibration->Get("/TRD/base_time_offset",base_time_offset))
		jout << "Error loading /TRD/base_time_offset !" << jendl;
	else if (base_time_offset.find("t0_wire") != base_time_offset.end() && base_time_offset.find("t0_gem") != base_time_offset.end()) {
		t_base[0] = base_time_offset["t0_wire"];
		t_base[1] = base_time_offset["t0_gem"];
	}
	else
		jerr << "Unable to get t0s from /TRD/base_time_offset !" << endl;
	
	// load constant tables
	if (calibration->Get("/TRD/Wire/timing_offsets", time_offsets[0]))
	jout << "Error loading /TRD/Wire/timing_offsets !" << jendl;
	*/
	for (unsigned int i=0;i<7;i++){
	  t_base[i]=-900.;
	}

	pulse_peak_threshold = 200;
}

//------------------
// Process
//------------------
void DTRDHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
    /// Generate DTRDHit object for each DTRDDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

    vector<const DTRDDigiHit*> digihits;
    event->Get(digihits);
    
    // loop over leading edges
    for (unsigned int i=0; i < digihits.size(); i++) {
	    const DTRDDigiHit *digihit = digihits[i];
	    
	    // subtract pedestal
	    double pulse_height = digihit->pulse_peak - digihit->pedestal;

	    // mask noisy wires for now
	    if(digihit->plane == 4)
		    if(digihit->strip == 0 || digihit->strip == 11 || digihit->strip == 12 || digihit->strip == 23) continue;

	    // separate theresholds for Wire and GEM TRD
	    if((digihit->plane == 4 || digihit->plane == 5 || digihit->plane == 0 || digihit->plane == 1) && pulse_height < 200) continue;
	    if((digihit->plane == 2 || digihit->plane == 6) && pulse_height < 350) continue;

	    // Time cut now
	    double T = (double)digihit->pulse_time * 0.8;
	    if(T < 145.) continue;

	    // Build hit object
	    DTRDHit *hit = new DTRDHit;
	    hit->pulse_height = pulse_height;
	    hit->plane = digihit->plane;
	    hit->strip = digihit->strip;
 
	    // Apply calibration constants
	    hit->t = T+t_base[digihit->plane];
	    //hit->t = hit->t + t_base[plane] - time_offsets[plane][strip];

	    hit->AddAssociatedObject(digihit);
	    Insert(hit);
    }
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
