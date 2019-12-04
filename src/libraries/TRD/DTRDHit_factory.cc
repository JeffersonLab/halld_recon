// $Id$
//
//    File: DTRDHit_factory.cc
//

#include <iostream>
#include <iomanip>
using namespace std;

#include "TRD/DTRDHit_factory.h"
using namespace jana;

//------------------
// init
//------------------
jerror_t DTRDHit_factory::init(void)
{
	// initialize calibration tables
	//vector<double> new_t0s(TRD_MAX_CHANNELS);	
	//time_offsets.push_back(new_t0s); time_offsets.push_back(new_t0s);

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DTRDHit_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
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
	
	if(print_messages) jout << "In DTRDHit_factory, loading constants..." << endl;

	/*	
	// load base time offset
	map<string,double> base_time_offset;
	if (eventLoop->GetCalib("/TRD/base_time_offset",base_time_offset))
		jout << "Error loading /TRD/base_time_offset !" << endl;
	else if (base_time_offset.find("t0_wire") != base_time_offset.end() && base_time_offset.find("t0_gem") != base_time_offset.end()) {
		t_base[0] = base_time_offset["t0_wire"];
		t_base[1] = base_time_offset["t0_gem"];
	}
	else
		jerr << "Unable to get t0s from /TRD/base_time_offset !" << endl;
	
	// load constant tables
	if (eventLoop->GetCalib("/TRD/Wire/timing_offsets", time_offsets[0]))
	jout << "Error loading /TRD/Wire/timing_offsets !" << endl;
	*/

	pulse_peak_threshold = 400;

    return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DTRDHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
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
    loop->Get(digihits);
    
    // loop over leading edges
    for (unsigned int i=0; i < digihits.size(); i++) {
	    const DTRDDigiHit *digihit = digihits[i];
	    
	    if(digihit->pulse_peak < pulse_peak_threshold) continue;
	    
	    // Build hit object
	    DTRDHit *hit = new DTRDHit;
	    hit->pulse_height = digihit->pulse_peak;
	    hit->plane = digihit->plane;
	    hit->strip = digihit->strip;

	    // Apply calibration constants
	    double T = (double)digihit->pulse_time;
	    hit->t = T;
	    //hit->t = hit->t + t_base[plane] - time_offsets[plane][strip];
 
	    hit->AddAssociatedObject(digihit);
	    _data.push_back(hit);
    }
		    
    return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DTRDHit_factory::erun(void)
{
    return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DTRDHit_factory::fini(void)
{
    return NOERROR;
}
