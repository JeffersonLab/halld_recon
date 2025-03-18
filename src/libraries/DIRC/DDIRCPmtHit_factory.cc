// $Id$
//
//    File: DDIRCPmtHit_factory.cc
//

#include <iostream>
#include <iomanip>
using namespace std;

#include "DIRC/DDIRCGeometry.h"
#include "DIRC/DDIRCPmtHit_factory.h"
#include "TTAB/DTTabUtilities.h"
#include "DAQ/DDIRCTriggerTime.h"

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>


//------------------
// Init
//------------------
void DDIRCPmtHit_factory::Init()
{
	auto app = GetApplication();
	DIRC_SKIP = false;
	app->SetDefaultParameter("DIRC:SKIP",DIRC_SKIP);
	DIRC_TIME_OFFSET = true;
	app->SetDefaultParameter("DIRC:TIME_OFFSET",DIRC_TIME_OFFSET);
	DIRC_TIMEWALK = true;
	app->SetDefaultParameter("DIRC:TIMEWALK",DIRC_TIMEWALK);

	// Initialize calibration tables
	vector<double> new_t0s(DIRC_MAX_CHANNELS);
	vector<int> new_status(DIRC_MAX_CHANNELS);
	
	time_offsets.push_back(new_t0s); time_offsets.push_back(new_t0s); 
	channel_status.push_back(new_status); channel_status.push_back(new_status);
}

//------------------
// BeginRun
//------------------
void DDIRCPmtHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	auto runnumber = event->GetRunNumber();
	auto app = event->GetJApplication();
	auto calibration = app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);

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
	
	if(print_messages) jout << "In DDIRCPmtHit_factory, loading constants..." << jendl;
	
	// load base time offset
	map<string,double> base_time_offset;
	if (calibration->Get("/DIRC/base_time_offset",base_time_offset))
		jout << "Error loading /DIRC/base_time_offset !" << jendl;
	else if (base_time_offset.find("t0_North") != base_time_offset.end() && base_time_offset.find("t0_South") != base_time_offset.end()) {
		t_base[0] = base_time_offset["t0_North"];
		t_base[1] = base_time_offset["t0_South"];
	}
	else
		jerr << "Unable to get t0s from /DIRC/base_time_offset !" << jendl;
	
	// load constant tables
	if (calibration->Get("/DIRC/North/timing_offsets", time_offsets[0]))
		jout << "Error loading /DIRC/North/timing_offsets !" << jendl;
	if (calibration->Get("/DIRC/North/channel_status", channel_status[0]))
		jout << "Error loading /DIRC/North/channel_status !" << jendl;
	if (calibration->Get("/DIRC/South/timing_offsets", time_offsets[1]))
		jout << "Error loading /DIRC/South/timing_offsets !" << jendl;
	if (calibration->Get("/DIRC/South/channel_status", channel_status[1]))
		jout << "Error loading /DIRC/South/channel_status !" << jendl;

}

//------------------
// Process
//------------------
void DDIRCPmtHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
    /// Generate DDIRCPmtHit object for each DDIRCDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

    if(DIRC_SKIP) 
      return;

    // check that SSP board timestamps match for all modules 
    vector<const DDIRCTriggerTime*> timestamps;
    event->Get(timestamps);
    if(timestamps.size() > 0) {
	    for (unsigned int i=0; i < timestamps.size()-1; i++) {
		    if(timestamps[i]->time != timestamps[i+1]->time) 
			    return;
	    }
    }

    vector<const DCODAROCInfo*> locCODAROCInfos;
    event->Get(locCODAROCInfos);
    uint64_t locReferenceClockTime = 0;
    for (const auto& locCODAROCInfo : locCODAROCInfos) {
        if(locCODAROCInfo->rocid == 92) {
                locReferenceClockTime = locCODAROCInfo->timestamp;
        }
    }

    vector<const DDIRCTDCDigiHit*> digihits;
    event->Get(digihits);
    
    // loop over leading edges
    for (unsigned int i=0; i < digihits.size(); i++) {
	    const DDIRCTDCDigiHit *digihit_lead = digihits[i];
	    if(digihit_lead->edge == 0) continue; // remove trailing edges 
	    // Note this doesn't match SSP data format document, but appears correct for data...
	    
	    double timeOverThreshold = 0.;

	    // loop over trailing edges
	    const DDIRCTDCDigiHit *digihit_trail = NULL;
	    for (unsigned int j=0; j < digihits.size(); j++) {
		    digihit_trail = digihits[j];
		    if(i==j || digihit_trail->edge == 1) continue; // remove leading edges
		    // Note this doesn't match SSP data format document, but appears correct for data...
		    
		    // discard hits from different channels
		    if(digihit_lead->channel != digihit_trail->channel) continue; 
		    int channel = digihit_lead->channel;
		    int box = (channel < DIRC_MAX_CHANNELS) ? 1 : 0; // North=0 and South=1
		    if(box == 0) channel -= DIRC_MAX_CHANNELS; // box-local channel to index CCDB tables

		    // get time-over-threshold
		    timeOverThreshold = (double)digihit_trail->time - (double)digihit_lead->time;
		    
		    // discard bad time-over-threshold (negative or too long > 100 ns)
		    if(timeOverThreshold < 0 || timeOverThreshold > 100) continue;
		    
		    // throw away hits from bad or noisy channels
		    dirc_status_state status = static_cast<dirc_status_state>(channel_status[box][channel]);
		    if ( (status==BAD) || (status==NOISY) ) continue;
		    
		    // Build hit object
		    DDIRCPmtHit *hit = new DDIRCPmtHit;
		    hit->ch   = digihit_lead->channel;
		    hit->tot  = timeOverThreshold;
		    
		    // Apply calibration constants
		    double T = (double)digihit_lead->time;
		    hit->t = T;
		    if(locReferenceClockTime%2 == 0) 
			hit->t += 4;

		    hit->t = hit->t + t_base[box];
		    if(DIRC_TIME_OFFSET) {
			    hit->t = hit->t - time_offsets[box][channel];
		    }
		    if(DIRC_TIMEWALK) {
			    double slope = 0.3;
			    double timeOverThresholdPeak = 50;
			    hit->t += slope*(timeOverThreshold - timeOverThresholdPeak);
		    }
 
		    hit->AddAssociatedObject(digihit_lead);
		    hit->AddAssociatedObject(digihit_trail);
		    Insert(hit);
	    }
    }
}

//------------------
// EndRun
//------------------
void DDIRCPmtHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DDIRCPmtHit_factory::Finish()
{
}
