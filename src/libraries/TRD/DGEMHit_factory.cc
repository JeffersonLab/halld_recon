// $Id$
//
//    File: DGEMHit_factory.cc
//

#include <iostream>
#include <iomanip>
#include <memory>
using namespace std;

#include "TRD/DGEMHit_factory.h"
using namespace jana;

double ns_per_sample = 25;

//------------------
// init
//------------------
jerror_t DGEMHit_factory::init(void)
{
	// initialize calibration tables
	//vector<double> new_t0s(TRD_MAX_CHANNELS);	
	//time_offsets.push_back(new_t0s); time_offsets.push_back(new_t0s);

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DGEMHit_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
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
	
	if(print_messages) jout << "In DGEMHit_factory, loading constants..." << endl;

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
jerror_t DGEMHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
    /// Generate DGEMHit object for each DGEMDigiWindowRawData 
    /// object which has a hit above the predefined threshold.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

    vector<const DGEMDigiWindowRawData*> windowrawdata;
    eventLoop->Get(windowrawdata);
    if(windowrawdata.size() == 0) 
      return NOERROR;

    // determine pedestals for each APV and time slice
    const DGEMSRSWindowRawData* srswindow;
    windowrawdata[windowrawdata.size()-1]->GetSingle(srswindow);
    uint nSamples = srswindow->samples.size();
    uint nAPV = srswindow->apv_id + 1;

    // vector of each time slice for given APV
    //vector<double> pedestalAPV[nAPV]; 
	std::shared_ptr< vector<double> > sptr_pedestalAPV( new vector<double>[nAPV] );
	vector<double>* pedestalAPV = sptr_pedestalAPV.get();
    for(uint i=0; i<nAPV; i++) pedestalAPV[i].resize(nSamples);
    
    for (const auto& window : windowrawdata) {
	const DGEMSRSWindowRawData* srswindow;
	window->GetSingle(srswindow);
        
        vector<uint16_t> samples = srswindow->samples;
        for(uint isample=0; isample<samples.size(); isample++) {
          pedestalAPV[srswindow->apv_id][isample] += samples[isample]/128.;
        }
    }

    // loop over GEM channels and subtract pedestal
    for (const auto& window : windowrawdata) {
	const DGEMSRSWindowRawData* srswindow;
	window->GetSingle(srswindow);
	
	int plane = window->plane;
	int strip = window->strip;
        vector<uint16_t> samples = srswindow->samples;

	// loop over samples to get pulse peak and time
	double pulse_peak = 0;
	double pulse_time = 0;
	for(uint isample=1; isample<samples.size(); isample++) {
                int adc_zs = -1. * (samples[isample]-pedestalAPV[srswindow->apv_id][isample]); // invert zero suppressed ADC
                //cout<<samples[isample]<<" "<<pedestalAPV[srswindow->apv_id][isample]<<" "<<adc_zs<<endl;
		if(adc_zs > pulse_peak) {
			pulse_peak = adc_zs;
			pulse_time = isample * ns_per_sample;
		}
	}

	if(pulse_peak < pulse_peak_threshold) continue;
  
	// Build hit object
	DGEMHit *hit = new DGEMHit;
	hit->pulse_height = pulse_peak;
	hit->plane = plane;
	hit->strip = strip;
	
	// Apply calibration constants
	double T = pulse_time;
	hit->t = T;
	
	hit->AddAssociatedObject(window);
	_data.push_back(hit);
    }
		    
    return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DGEMHit_factory::erun(void)
{
    return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DGEMHit_factory::fini(void)
{
    return NOERROR;
}
