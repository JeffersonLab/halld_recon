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
	for (unsigned int i=0;i<8;i++){
	  t_base[i]=-900.;
	}

	pulse_peak_threshold = 200;

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
	    
	    // subtract pedestal
	    double pulse_height = digihit->pulse_peak - digihit->pedestal;

	    // mask noisy wires for now
	    //if(digihit->plane == 4)
	    //	    if(digihit->strip == 0 || digihit->strip == 11 || digihit->strip == 12 || digihit->strip == 23) continue;

	    // separate theresholds for Wire and GEM TRD
	    if((digihit->plane == 3 || digihit->plane == 4 || digihit->plane == 5 || digihit->plane == 0 || digihit->plane == 1) && pulse_height < 250) continue;
	    if((digihit->plane == 2 || digihit->plane == 6 || digihit->plane == 7) && pulse_height < 200) continue;

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
