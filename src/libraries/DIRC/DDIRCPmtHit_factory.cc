// $Id$
//
//    File: DDIRCPmtHit_factory.cc
//

#include <iostream>
#include <iomanip>
using namespace std;

//#include "DIRC/DDIRCDigiHit.h"
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
	vector<double> new_qualities(DIRC_MAX_CHANNELS);
	
	time_offsets = new_t0s;
	block_qualities = new_qualities;
	
	// set the base conversion scales --
	t_scale = 0.0625;   // 62.5 ps/count
	t_base  = 0.;       // ns
	
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
	vector< double > raw_block_qualities;    // we should change this to an int?
	
	if(print_messages) jout << "In DDIRCPmtHit_factory, loading constants..." << endl;
	
	// load base time offset
	map<string,double> base_time_offset;
	if (eventLoop->GetCalib("/DIRC/base_time_offset",base_time_offset))
		jout << "Error loading /DIRC/base_time_offset !" << endl;
	if (base_time_offset.find("DIRC_BASE_TIME_OFFSET") != base_time_offset.end())
		t_base = base_time_offset["DIRC_BASE_TIME_OFFSET"];
	else
		jerr << "Unable to get DIRC_BASE_TIME_OFFSET from /DIRC/base_time_offset !" << endl;
	
	// load constant tables
	if (eventLoop->GetCalib("/DIRC/timing_offsets", raw_time_offsets))
		jout << "Error loading /DIRC/timing_offsets !" << endl;
	if (eventLoop->GetCalib("/DIRC/block_quality", raw_block_qualities))
		jout << "Error loading /DIRC/block_quality !" << endl;
	
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
    char str[256];

#if 0
    // extract the DIRC Geometry (for positionOnFace())
    vector<const DDIRCGeometry*> dircGeomVect;
    eventLoop->Get( dircGeomVect );
    if (dircGeomVect.size() < 1)
        return OBJECT_NOT_AVAILABLE;
    const DDIRCGeometry& dircGeom = *(dircGeomVect[0]);

    const DTTabUtilities* locTTabUtilities = nullptr;
    loop->GetSingle(locTTabUtilities);

    vector<const DDIRCDigiHit*> digihits;
    loop->Get(digihits);
    for (unsigned int i=0; i < digihits.size(); i++) {

        const DDIRCDigiHit *digihit = digihits[i];

        // Throw away hits with firmware errors (post-summer 2016 firmware)
        if(CHECK_FADC_ERRORS && !locTTabUtilities->CheckFADC250_NoErrors(digihit->QF))
            continue;

        // Check to see if the hit corresponds to a valid channel
        if (dircGeom.isBlockActive(digihit->row,digihit->column) == false) {
            sprintf(str, "DDIRCPmtHit corresponds to inactive channel!  "
                    "row=%d, col=%d", 
                    digihit->row, digihit->column);
            throw JException(str);
        }

        // throw away hits from bad or noisy channels
        dirc_quality_state quality = 
            static_cast<dirc_quality_state>(block_qualities[digihit->row][digihit->column]);
        if ( (quality==BAD) || (quality==NOISY) ) continue;

        // get pedestal from CCDB -- we should use it instead
        // of the event-by-event pedestal
        // Corresponds to the value of the pedestal for a single sample
        double pedestal = pedestals[digihit->row][digihit->column];

        // we should use the fixed database pedestal
        // object as it is less susceptible to noise
        // than the event-by-event pedestal
        double nsamples_integral = digihit->nsamples_integral;
        double nsamples_pedestal = digihit->nsamples_pedestal;

        // if the database pedestal is zero then try
        // the event-by-event one:
        if(pedestal == 0) {
            // nsamples_pedestal should always be positive for valid data - err on the side of caution for now
            if(nsamples_pedestal == 0) {
                jerr << "DDIRCDigiHit with nsamples_pedestal == 0 !   Event = " << eventnumber << endl;
                continue;
            }

            // digihit->pedestal is the sum of "nsamples_pedestal" samples
            // Calculate the average pedestal per sample
            if( (digihit->pedestal>0) && locTTabUtilities->CheckFADC250_PedestalOK(digihit->QF) ) {
                pedestal = (double)digihit->pedestal/nsamples_pedestal;
            }
        }

        // Subtract pedestal from pulse peak
        if (digihit->pulse_time == 0 || digihit->pedestal == 0 || digihit->pulse_peak == 0) continue;
        double pulse_amplitude = digihit->pulse_peak - pedestal;

        double integratedPedestal = pedestal * nsamples_integral;

        // Build hit object
        DDIRCPmtHit *hit = new DDIRCPmtHit;
        hit->row    = digihit->row;
        hit->column = digihit->column;

        // Apply calibration constants
        double A = (double)digihit->pulse_integral;
        double T = (double)digihit->pulse_time;
        hit->E = a_scale * gains[hit->row][hit->column] * (A - integratedPedestal);
        hit->t = t_scale * T - time_offsets[hit->row][hit->column] + t_base + ADC_Offsets[hit->row][hit->column];;

        // Get position of blocks on front face. (This should really come from
        // hdgeant directly so the poisitions can be shifted in mcsmear.)
        DVector2 pos = dircGeom.positionOnFace(hit->row, hit->column);
        hit->x = pos.X();
        hit->y = pos.Y();

        // recored the pulse integral to peak ratio since this is
        // a useful quality metric for the PMT pulse
        hit->intOverPeak = ( A - integratedPedestal ) / pulse_amplitude;
        
        // do some basic quality checks before creating the objects
        if( ( hit->E > 0 ) &&
            ( digihit->pulse_time > 0 )  ) {
            hit->AddAssociatedObject(digihit);
            _data.push_back(hit);
        } else {
            delete hit;
        }
    }
#endif

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
