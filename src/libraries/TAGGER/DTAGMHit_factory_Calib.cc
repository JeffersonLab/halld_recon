// $Id$
//
//    File: DTAGMHit_factory_Calib.cc
// Created: Sat Aug  2 12:23:43 EDT 2014
// Creator: jonesrt (on Linux gluex.phys.uconn.edu)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DTAGMDigiHit.h"
#include "DTAGMTDCDigiHit.h"
#include "DTAGMGeometry.h"
#include "DTAGMHit_factory_Calib.h"
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulsePedestal.h>
#include <DAQ/Df250Config.h>

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>


const int DTAGMHit_factory_Calib::k_fiber_dead;
const int DTAGMHit_factory_Calib::k_fiber_good;
const int DTAGMHit_factory_Calib::k_fiber_bad;
const int DTAGMHit_factory_Calib::k_fiber_noisy;

//------------------
// Init
//------------------
void DTAGMHit_factory_Calib::Init()
{
	auto app = GetApplication();

    DELTA_T_ADC_TDC_MAX = 10.0; // ns
    USE_ADC = 0;
    CUT_FACTOR = 1;
    app->SetDefaultParameter("TAGMHit:DELTA_T_ADC_TDC_MAX", DELTA_T_ADC_TDC_MAX,
                "Maximum difference in ns between a (calibrated) fADC time and"
                " F1TDC time for them to be matched in a single hit");
    app->SetDefaultParameter("TAGMHit:CUT_FACTOR", CUT_FACTOR, "TAGM pulse integral cut factor, 0 = no cut");
    app->SetDefaultParameter("TAGMHit:USE_ADC", USE_ADC, "Use ADC times in TAGM");

    CHECK_FADC_ERRORS = true;
    app->SetDefaultParameter("TAGMHit:CHECK_FADC_ERRORS", CHECK_FADC_ERRORS, "Set to 1 to reject hits with fADC250 errors, ser to 0 to keep these hits");

    // Initialize calibration constants
    fadc_a_scale = 0;
    fadc_t_scale = 0;
    t_base = 0;
    t_tdc_base=0;

    // calibration constants stored in row, column format
    for (int row = 0; row <= TAGM_MAX_ROW; ++row) {
        for (int col = 0; col <= TAGM_MAX_COLUMN; ++col) {
            fadc_gains[row][col] = 0;
            fadc_pedestals[row][col] = 0;
            fadc_time_offsets[row][col] = 0;
            tdc_time_offsets[row][col] = 0;
            fiber_quality[row][col] = 0;
        }
    }

    return;
}

//------------------
// BeginRun
//------------------
void DTAGMHit_factory_Calib::BeginRun(const std::shared_ptr<const JEvent>& event)
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

    /// set the base conversion scales
    fadc_a_scale    = 1.1;        // pixels per count
    fadc_t_scale    = 0.0625;     // ns per count
    t_base           = 0.;      // ns

    pthread_mutex_unlock(&print_mutex);
    if(print_messages) jout << "In DTAGMHit_factory_Calib, loading constants..." << jendl;

    // load base time offset
    map<string,double> base_time_offset;
    if (calibration->Get("/PHOTON_BEAM/microscope/base_time_offset",base_time_offset))
        jout << "Error loading /PHOTON_BEAM/microscope/base_time_offset !" << jendl;
    if (base_time_offset.find("TAGM_BASE_TIME_OFFSET") != base_time_offset.end())
        t_base = base_time_offset["TAGM_BASE_TIME_OFFSET"];
    else
        jerr << "Unable to get TAGM_BASE_TIME_OFFSET from /PHOTON_BEAM/microscope/base_time_offset !" << jendl;
    if (base_time_offset.find("TAGM_TDC_BASE_TIME_OFFSET") != base_time_offset.end())
        t_tdc_base = base_time_offset["TAGM_TDC_BASE_TIME_OFFSET"];
    else
        jerr << "Unable to get TAGM_TDC_BASE_TIME_OFFSET from /PHOTON_BEAM/microscope/base_time_offset !" << jendl;

    if (load_ccdb_constants(calibration, "fadc_gains", "gain", fadc_gains) &&
    load_ccdb_constants(calibration, "fadc_pedestals", "pedestal", fadc_pedestals) &&
    load_ccdb_constants(calibration, "fadc_time_offsets", "offset", fadc_time_offsets) &&
    load_ccdb_constants(calibration, "tdc_time_offsets", "offset", tdc_time_offsets) &&
    load_ccdb_constants(calibration, "fiber_quality", "code", fiber_quality) &&
    load_ccdb_constants(calibration, "tdc_timewalk_corrections", "c0", tw_c0) &&
    load_ccdb_constants(calibration, "tdc_timewalk_corrections", "c1", tw_c1) &&
    load_ccdb_constants(calibration, "tdc_timewalk_corrections", "c2", tw_c2) &&
    load_ccdb_constants(calibration, "tdc_timewalk_corrections", "threshold", tw_c3) &&
    load_ccdb_constants(calibration, "tdc_timewalk_corrections", "reference", ref) &&
    load_ccdb_constants(calibration, "integral_cuts", "integral", int_cuts))
    {
        return;
    }

    // The meaning of the integral_cuts table is as follows.
    // For each column col, if
    //   *) int_cuts[col] > 0 then apply an offline threshold of
    //                        int_cuts[col] * CUT_FACTOR to the fadc
    //                        pulse integral value, thits that fail
    //                        are never added to the list of TAGMHits.
    //   *) int_cuts[col] < 0 then require each TAGM hit to consist
    //                        of a fadc pulse with a tdc in coincidence,
    //                        otherwise mark hits as has_fADC=false
    //                        which leaves them in the list, but no
    //                        DBeamPhoton is made from them;
    //   *) int_cuts[col] = 0 then no offline requirement is applied,
    //                        all raw fADC hits show up with has_fADC
    //                        set to True, regardless of pulse height
    //                        or presence of a tdc in coincidence.
    // The individual int_cuts[col] values are multiplied by a
    // global commandline parameter CUT_FACTOR (default 1) before
    // the above logic is applied, which makes it possible to suspend
    // or reverse the logic of the above cuts at reconstruction time.

    // return UNRECOVERABLE_ERROR;
    throw JException("DTAGMHit_factory_Calib: Unrecoverable error");
}

//------------------
// Process
//------------------
void DTAGMHit_factory_Calib::Process(const std::shared_ptr<const JEvent>& event)
{
    /// Generate DTAGMHit object for each DTAGMDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

    // extract the TAGM geometry
    vector<const DTAGMGeometry*> tagmGeomVect;
    event->Get( tagmGeomVect );
    if (tagmGeomVect.size() < 1)
        return; // OBJECT_NOT_AVAILABLE;
    const DTAGMGeometry& tagmGeom = *(tagmGeomVect[0]);

    const DTTabUtilities* locTTabUtilities = nullptr;
    event->GetSingle(locTTabUtilities);

    // First loop over all TAGMDigiHits and make DTAGMHits out of them
    vector<const DTAGMDigiHit*> digihits;
    event->Get(digihits);
    for (unsigned int i=0; i < digihits.size(); i++) {
        const DTAGMDigiHit *digihit = digihits[i];

        // Throw away hits with firmware errors (post-summer 2016 firmware)
        if(CHECK_FADC_ERRORS && !locTTabUtilities->CheckFADC250_NoErrors(digihit->QF))
            continue;

        // Get pedestal, prefer associated event pedestal if it exists,
        // otherwise, use the average pedestal from CCDB
        double pedestal = fadc_pedestals[digihit->row][digihit->column];
        double nsamples_integral = (double)digihit->nsamples_integral;
        double nsamples_pedestal = (double)digihit->nsamples_pedestal;

        // nsamples_pedestal should always be positive for valid data - err on the side of caution for now
        if(nsamples_pedestal == 0) {
            jerr << "DTAGMDigiHit with nsamples_pedestal == 0 !   Event = " << event->GetEventNumber() << jendl;
            continue;
        }

        // digihit->pedestal is the sum of "nsamples_pedestal" samples
        // Calculate the average pedestal per sample
        if ( (digihit->pedestal>0) && locTTabUtilities->CheckFADC250_PedestalOK(digihit->QF) ) {
            pedestal = (double)digihit->pedestal/nsamples_pedestal;
        }

        // Subtract pedestal from pulse peak
        if (digihit->pulse_time == 0 || digihit->pedestal == 0 || digihit->pulse_peak == 0) continue;
        double pulse_peak = digihit->pulse_peak - pedestal;

        // throw away hits from bad or noisy fibers
        int quality = fiber_quality[digihit->row][digihit->column];
        if (quality == k_fiber_dead || quality == k_fiber_bad || quality == k_fiber_noisy)
            continue;

        // Apply calibration constants
        double A = digihit->pulse_integral;
        double T = digihit->pulse_time;
        A -= pedestal*nsamples_integral;
        int row = digihit->row;
        int column = digihit->column;
        if (A < CUT_FACTOR*int_cuts[row][column]) continue;

        DTAGMHit *hit = new DTAGMHit;
        hit->row = row;
        hit->column = column;
        double Elow = tagmGeom.getElow(column);
        double Ehigh = tagmGeom.getEhigh(column);
        hit->E = (Elow + Ehigh)/2;

        hit->integral = A;
        hit->pulse_peak = pulse_peak;
        hit->npix_fadc = A * fadc_a_scale * fadc_gains[row][column];
        hit->time_fadc = T * fadc_t_scale - fadc_time_offsets[row][column] + t_base;
        hit->t=hit->time_fadc;
        hit->has_TDC=false;
        hit->time_tdc = 0;

        // Interpret a negative value on the integral cut to require
        // an associated tdc hit in place of an the pulse integral cut.
        // Preemptively set it to false here, and reverse it later
        // if and when an associated tdc is found.
        if (CUT_FACTOR*int_cuts[row][column] < 0)
           hit->has_fADC=false;
        else
           hit->has_fADC=true;

        hit->AddAssociatedObject(digihit);
        Insert(hit);
    }

    // Next, loop over TDC hits, matching them to the existing fADC hits
    // where possible and updating their time information. If no match is
    // found, then create a new hit with just the TDC info.
    vector<const DTAGMTDCDigiHit*> tdcdigihits;
    event->Get(tdcdigihits);
    for (unsigned int i=0; i < tdcdigihits.size(); i++) {
        const DTAGMTDCDigiHit *digihit = tdcdigihits[i];

        // throw away hits from bad or noisy fibers
        int quality = fiber_quality[digihit->row][digihit->column];
        if (quality == k_fiber_dead || quality == k_fiber_bad || quality == k_fiber_noisy)
            continue;
        
        // Apply calibration constants here
        int row = digihit->row;
        int column = digihit->column;
        double T = locTTabUtilities->Convert_DigiTimeToNs_F1TDC(digihit) - tdc_time_offsets[row][column] + t_tdc_base;

        // Look for existing hits to see if there is a match
        // or create new one if there is no match
        DTAGMHit *hit = nullptr;
        for (unsigned int j=0; j < mData.size(); ++j) {
            if (mData[j]->row == row && mData[j]->column == column &&
                mData[j]->time_fadc != 0 && mData[j]->integral != 0 &&
                fabs(T - mData[j]->time_fadc) < DELTA_T_ADC_TDC_MAX)
            {
                hit = mData[j];
            }
        }
        if (hit == nullptr) {
            hit = new DTAGMHit;
            hit->row = row;
            hit->column = column;
            double Elow = tagmGeom.getElow(column);
            double Ehigh = tagmGeom.getEhigh(column);
            hit->E = (Elow + Ehigh)/2;
            hit->time_fadc = 0;
            hit->npix_fadc = 0;
            hit->integral = 0;
            hit->pulse_peak = 0;
            hit->has_fADC=false;
            Insert(hit);
        }
        // Interpret a negative value on the integral cut to require
        // an associated tdc hit in place of an the pulse integral cut.
        else if (CUT_FACTOR*int_cuts[row][column] < 0) {
           hit->has_fADC = (T != 0);
        }

        hit->time_tdc=T;
        hit->has_TDC=true;

        // apply time-walk corrections
        double P = hit->pulse_peak;
        double c0 = tw_c0[row][column];
        double c1 = tw_c1[row][column];
        double c2 = tw_c2[row][column];
        //double TH = thresh[row][column];
        double c3 = tw_c3[row][column];
        double t0 = ref[row][column];
        //pp_0 = TH*pow((pp_0-c0)/c1,1/c2);
        if (P > 0) {
           //T -= c1*(pow(P/TH,c2)-pow(pp_0/TH,c2));
           T -= c1*pow(1/(P+c3),c2) - (t0 - c0);
        }

        // Optionally only use ADC times
        if (USE_ADC && hit->has_fADC) hit->t = hit->time_fadc;
        else  hit->t = T;
        
        hit->AddAssociatedObject(digihit);
    }

    return;
}

//------------------
// EndRun
//------------------
void DTAGMHit_factory_Calib::EndRun()
{
    return;
}

//------------------
// Finish
//------------------
void DTAGMHit_factory_Calib::Finish()
{
    return;
}

//---------------------
// load_ccdb_constants
//---------------------
bool DTAGMHit_factory_Calib::load_ccdb_constants(
		JCalibration* calibration,
        std::string table_name,
        std::string column_name,
        double result[TAGM_MAX_ROW+1][TAGM_MAX_COLUMN+1])
{
    std::vector< std::map<std::string, double> > table;
    std::string ccdb_key = "/PHOTON_BEAM/microscope/" + table_name;
    if (calibration->Get(ccdb_key, table))
    {
        jout << "Error loading " << ccdb_key << " from ccdb!" << jendl;
        return false;
    }
    for (unsigned int i=0; i < table.size(); ++i) {
        int row = (table[i])["row"];
        int col = (table[i])["column"];
        result[row][col] = (table[i])[column_name];
    }
    return true;
}
