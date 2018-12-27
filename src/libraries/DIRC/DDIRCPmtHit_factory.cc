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
using namespace jana;

//------------------
// init
//------------------
jerror_t DDIRCPmtHit_factory::init(void)
{
	// initialize calibration tables
	vector<double> new_t0s(DIRC_MAX_CHANNELS);
	//vector<double> new_qualities(DIRC_MAX_CHANNELS);
	
	time_offsets = new_t0s;
	//block_qualities = new_qualities;
	
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DDIRCPmtHit_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
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
	
	// extract the DIRC Geometry
	vector<const DDIRCGeometry*> dircGeomVect;
	eventLoop->Get( dircGeomVect );
	if (dircGeomVect.size() < 1)
		return OBJECT_NOT_AVAILABLE;
	const DDIRCGeometry& dircGeom = *(dircGeomVect[0]);
	
	/// Read in calibration constants
	vector< double > raw_time_offsets;
	//vector< double > raw_block_qualities;    // we should change this to an int?
	
	if(print_messages) jout << "In DDIRCPmtHit_factory, loading constants..." << endl;
	
	// load base time offset
	map<string,double> base_time_offset;
	//if (eventLoop->GetCalib("/DIRC/base_time_offset",base_time_offset))
	//	jout << "Error loading /DIRC/base_time_offset !" << endl;
	//if (base_time_offset.find("DIRC_BASE_TIME_OFFSET") != base_time_offset.end())
	//	t_base = base_time_offset["DIRC_BASE_TIME_OFFSET"];
	//else
	//	jerr << "Unable to get DIRC_BASE_TIME_OFFSET from /DIRC/base_time_offset !" << endl;
	
	// load constant tables
	//if (eventLoop->GetCalib("/DIRC/timing_offsets", raw_time_offsets))
	//	jout << "Error loading /DIRC/timing_offsets !" << endl;
	//if (eventLoop->GetCalib("/DIRC/block_quality", raw_block_qualities))
	//	jout << "Error loading /DIRC/block_quality !" << endl;
	
	//FillCalibTable(time_offsets, raw_time_offsets, dircGeom);
	//FillCalibTable(block_qualities, raw_block_qualities, dircGeom);

    return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DDIRCPmtHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
    /// Generate DDIRCPmtHit object for each DDIRCDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

/*
    char str[256];

    // extract the DIRC Geometry (for positionOnFace())
    vector<const DDIRCGeometry*> dircGeomVect;
    eventLoop->Get( dircGeomVect );
    if (dircGeomVect.size() < 1)
        return OBJECT_NOT_AVAILABLE;
    const DDIRCGeometry& dircGeom = *(dircGeomVect[0]);
*/

    vector<const DDIRCTDCDigiHit*> digihits;
    loop->Get(digihits);
    
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
		    
		    // get time-over-threshold
		    timeOverThreshold = (double)digihit_trail->time - (double)digihit_lead->time;
		    
		    // discard bad time-over-threshold (negative or too long > 100 ns)
		    if(timeOverThreshold < 0 || timeOverThreshold > 100) continue;
		    
		    // throw away hits from bad or noisy channels
		    //dirc_quality_state quality = static_cast<dirc_quality_state>(block_qualities[digihit->row][digihit->column]);
		    //if ( (quality==BAD) || (quality==NOISY) ) continue;
		    
		    // Build hit object
		    DDIRCPmtHit *hit = new DDIRCPmtHit;
		    hit->ch   = digihit_lead->channel;
		    hit->tot  = timeOverThreshold;
		    
		    // Apply calibration constants
		    double T = (double)digihit_lead->time;
		    hit->t = T; // - time_offsets[hit->ch] + t_base;
		    
		    hit->AddAssociatedObject(digihit_lead);
		    hit->AddAssociatedObject(digihit_trail);
		    _data.push_back(hit);
	    }
    }
		    
    return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DDIRCPmtHit_factory::erun(void)
{
    return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DDIRCPmtHit_factory::fini(void)
{
    return NOERROR;
}
