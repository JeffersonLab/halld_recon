// $Id$
//
//    File: DTAGHHit_factory_Calib.cc
// Created: Wed Aug  3 12:55:19 EDT 2016
// Creator: nsparks (on Linux cua2.jlab.org 3.10.0-327.22.2.el7.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
#include <limits>
using namespace std;

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include "DTAGHHit_factory_Calib.h"
#include "DTAGHDigiHit.h"
#include "DTAGHTDCDigiHit.h"
#include "TTAB/DTTabUtilities.h"
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulsePedestal.h>



const int DTAGHHit_factory_Calib::k_counter_dead;
const int DTAGHHit_factory_Calib::k_counter_good;
const int DTAGHHit_factory_Calib::k_counter_bad;
const int DTAGHHit_factory_Calib::k_counter_noisy;

//------------------
// Init
//------------------
void DTAGHHit_factory_Calib::Init()
{
	auto app = GetApplication();

    // set default config. parameters
    DELTA_T_ADC_TDC_MAX = 10.0; // ns
    app->SetDefaultParameter("TAGHHit:DELTA_T_ADC_TDC_MAX", DELTA_T_ADC_TDC_MAX,
    "Maximum difference in ns between a (calibrated) fADC time and"
    " F1TDC time for them to be matched in a single hit");
    ADC_THRESHOLD = 200.0; // ADC counts
    app->SetDefaultParameter("TAGHHit:ADC_THRESHOLD",ADC_THRESHOLD,
    "pulse height threshold");

    CHECK_FADC_ERRORS = true;
    app->SetDefaultParameter("TAGHHit:CHECK_FADC_ERRORS", CHECK_FADC_ERRORS, "Set to 1 to reject hits with fADC250 errors, ser to 0 to keep these hits");

    // Initialize calibration constants
    fadc_a_scale = 0;
    fadc_t_scale = 0;
    t_base = 0;
    t_tdc_base=0;

    // calibration constants stored by counter index
    for (int counter = 0; counter <= TAGH_MAX_COUNTER; ++counter) {
        fadc_gains[counter] = 0;
        fadc_pedestals[counter] = 0;
        fadc_time_offsets[counter] = 0;
        tdc_time_offsets[counter] = 0;
        counter_quality[counter] = 0;
    }
}

//------------------
// BeginRun
//------------------
void DTAGHHit_factory_Calib::BeginRun(const std::shared_ptr<const JEvent>& event)
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

    if(print_messages) jout << "In DTAGHHit_factory, loading constants..." << jendl;

    // load base time offset
    map<string,double> base_time_offset;
    if (calibration->Get("/PHOTON_BEAM/hodoscope/base_time_offset",base_time_offset))
        jout << "Error loading /PHOTON_BEAM/hodoscope/base_time_offset !" << jendl;
    if (base_time_offset.find("TAGH_BASE_TIME_OFFSET") != base_time_offset.end())
        t_base = base_time_offset["TAGH_BASE_TIME_OFFSET"];
    else
        jerr << "Unable to get TAGH_BASE_TIME_OFFSET from /PHOTON_BEAM/hodoscope/base_time_offset !" << jendl;

    if (base_time_offset.find("TAGH_TDC_BASE_TIME_OFFSET") != base_time_offset.end())
        t_tdc_base = base_time_offset["TAGH_TDC_BASE_TIME_OFFSET"];
    else
        jerr << "Unable to get TAGH_TDC_BASE_TIME_OFFSET from /PHOTON_BEAM/hodoscope/base_time_offset !" << jendl;

    if (load_ccdb_constants(calibration, "fadc_gains", "gain", fadc_gains) &&
        load_ccdb_constants(calibration, "fadc_pedestals", "pedestal", fadc_pedestals) &&
        load_ccdb_constants(calibration, "fadc_time_offsets", "offset", fadc_time_offsets) &&
        load_ccdb_constants(calibration, "tdc_time_offsets", "offset", tdc_time_offsets) &&
        load_ccdb_constants(calibration, "counter_quality", "code", counter_quality) &&
        load_ccdb_constants(calibration, "tdc_timewalk", "c0", tdc_twalk_c0) &&
        load_ccdb_constants(calibration, "tdc_timewalk", "c1", tdc_twalk_c1) &&
        load_ccdb_constants(calibration, "tdc_timewalk", "c2", tdc_twalk_c2) &&
        load_ccdb_constants(calibration, "tdc_timewalk", "c3", tdc_twalk_c3))
    {
        return;
    }
    throw JException("Unrecoverable error!");  //  TODO: Reexamine
    // return UNRECOVERABLE_ERROR;
}

//------------------
// Process
//------------------
void DTAGHHit_factory_Calib::Process(const std::shared_ptr<const JEvent>& event)
{
    /// Generate DTAGHHit object for each DTAGHDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

    vector<DTAGHHit *> results, filtered_results;

    // extract the TAGH geometry
    vector<const DTAGHGeometry*> taghGeomVect;
    event->Get( taghGeomVect );
    if (taghGeomVect.size() < 1)
        return; // OBJECT_NOT_AVAILABLE;
    const DTAGHGeometry& taghGeom = *(taghGeomVect[0]);

    const DTTabUtilities* locTTabUtilities = nullptr;
    event->GetSingle(locTTabUtilities);

    // First loop over all TAGHDigiHits and make DTAGHHits out of them
    vector<const DTAGHDigiHit*> digihits;
    event->Get(digihits);
    for (unsigned int i=0; i < digihits.size(); i++) {
        const DTAGHDigiHit *digihit = digihits[i];
        int counter = digihit->counter_id;

        // throw away hits from bad or noisy counters
        int quality = counter_quality[counter];
        if (quality == k_counter_dead || quality == k_counter_bad || quality == k_counter_noisy)
            continue;

        // Throw away hits with firmware errors (post-summer 2016 firmware)
        if(CHECK_FADC_ERRORS && !locTTabUtilities->CheckFADC250_NoErrors(digihit->QF))
            continue;

        // Get pedestal, prefer associated event pedestal if it exists,
        // otherwise, use the average pedestal from CCDB
        double pedestal = fadc_pedestals[counter];
        double nsamples_integral = (double)digihit->nsamples_integral;
        double nsamples_pedestal = (double)digihit->nsamples_pedestal;

        // nsamples_pedestal should always be positive for valid data - err on the side of caution for now
        if(nsamples_pedestal == 0) {
            jerr << "DTAGHDigiHit with nsamples_pedestal == 0 !   Event = " << event->GetEventNumber() << jendl;
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

        // Subtract pedestal from pulse integral
        double A = digihit->pulse_integral;
        A -= pedestal*nsamples_integral;

        // Pulse height cut
        if (pulse_peak < ADC_THRESHOLD) continue;

        DTAGHHit *hit = new DTAGHHit;
        hit->counter_id = counter;
        double Elow = taghGeom.getElow(counter);
        double Ehigh = taghGeom.getEhigh(counter);
        hit->E = (Elow + Ehigh)/2;

        // Apply calibration constants
        double T = digihit->pulse_time;
        hit->integral = A;
        hit->pulse_peak = pulse_peak;
        hit->npe_fadc = A * fadc_a_scale * fadc_gains[counter];
        hit->time_fadc = T * fadc_t_scale - fadc_time_offsets[counter] + t_base;
        hit->time_tdc = numeric_limits<double>::quiet_NaN();
        hit->t = hit->time_fadc;
        hit->has_fADC = true;
        hit->has_TDC = false;
        hit->is_double = false;

        hit->AddAssociatedObject(digihit);
        results.push_back(hit);
    }

    // Next, loop over TDC hits, matching them to the existing fADC hits
    // where possible and updating their time information. If no match is
    // found, then create a new hit with just the TDC info.
    vector<const DTAGHTDCDigiHit*> tdcdigihits;
    event->Get(tdcdigihits);
    for (unsigned int i=0; i < tdcdigihits.size(); i++) {
        const DTAGHTDCDigiHit *digihit = tdcdigihits[i];

        // throw away hits from bad or noisy counters
        int quality = counter_quality[digihit->counter_id];
        if (quality == k_counter_dead || quality == k_counter_bad || quality == k_counter_noisy)
            continue;

        // Apply calibration constants here
        int counter = digihit->counter_id;
        double T = locTTabUtilities->Convert_DigiTimeToNs_F1TDC(digihit) - tdc_time_offsets[counter] + t_tdc_base;

        // Look for existing hits to see if there is a match
        // or create new one if there is no match
        DTAGHHit *hit = nullptr;
        for (unsigned int j=0; j < results.size(); ++j) {
            if (results[j]->counter_id == counter &&
                fabs(T - results[j]->time_fadc) < DELTA_T_ADC_TDC_MAX)
                {
                    hit = results[j];
                }
        }
        if (hit == nullptr) {
            hit = new DTAGHHit;
            hit->counter_id = counter;
            double Elow = taghGeom.getElow(counter);
            double Ehigh = taghGeom.getEhigh(counter);
            hit->E = (Elow + Ehigh)/2;
            hit->time_fadc = numeric_limits<double>::quiet_NaN();
            hit->integral = numeric_limits<double>::quiet_NaN();
            hit->pulse_peak = numeric_limits<double>::quiet_NaN();
            hit->npe_fadc = numeric_limits<double>::quiet_NaN();
            hit->has_fADC = false;
            hit->is_double = false;
        	results.push_back(hit);
        }
        hit->time_tdc = T;
        hit->has_TDC = true;
        // apply time-walk corrections
        double c0 = tdc_twalk_c0[counter]; double c1 = tdc_twalk_c1[counter];
        double c2 = tdc_twalk_c2[counter]; double c3 = tdc_twalk_c3[counter];
        double P = hit->pulse_peak;
        if (P > 0.0) {
            T -= (P <= c3 || c3 <= 0.0) ? c0 + c1/pow(P,c2) : c0 +  c1*(1.0+c2)/pow(c3,c2) - c1*c2*P/pow(c3,1.0+c2);
        }
        hit->t = T;
        hit->AddAssociatedObject(digihit);
    }
    

    for(auto hit : results)
    	if(hit->has_fADC==true || hit->has_TDC==true)
    		filtered_results.push_back(hit);

	Set(filtered_results);
}

//------------------
// EndRun
//------------------
void DTAGHHit_factory_Calib::EndRun()
{
}

//------------------
// Finish
//------------------
void DTAGHHit_factory_Calib::Finish()
{
}

//---------------------
// load_ccdb_constants
//---------------------
bool DTAGHHit_factory_Calib::load_ccdb_constants(
	JCalibration* calibration,
    std::string table_name,
    std::string column_name,
    double result[TAGH_MAX_COUNTER+1])
{
    std::vector< std::map<std::string, double> > table;
    std::string ccdb_key = "/PHOTON_BEAM/hodoscope/" + table_name;
    if (calibration->Get(ccdb_key, table))
    {
        jout << "Error loading " << ccdb_key << " from ccdb!" << jendl;
        return false;
    }
    for (unsigned int i=0; i < table.size(); ++i) {
        int counter = (table[i])["id"];
        result[counter] = (table[i])[column_name];
    }
    return true;
}
