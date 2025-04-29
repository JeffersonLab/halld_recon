// $Id$
//
//    File: DTRDHit_factory_Calib.cc
//

#include <iostream>
#include <iomanip>
using namespace std;

#include "DANA/DEvent.h"
#include <DAQ/Df125Config.h>
#include <DAQ/Df125FDCPulse.h>
#include "TRD/DTRDHit_factory_Calib.h"

//------------------
// Init
//------------------
void DTRDHit_factory_Calib::Init()
{
	// Initialize calibration tables
	//vector<double> new_t0s(TRD_MAX_CHANNELS);	
	//time_offsets.push_back(new_t0s); time_offsets.push_back(new_t0s);

	auto app = GetApplication();

    /// set the base conversion scales
    //a_scale      = 2.4E4/1.3E5;  // NOTE: currently fixed to FDC values, currently not used
    a_scale=1.;					// to test with real fADC pulses
	t_scale      = 8.0;     // 8 ns/count
    t_base       = { 0.,  0.};   // ns, per plane
    
    PEAK_THRESHOLD = 100.;  // fADC units
    app->SetDefaultParameter("TRD:PEAK_THRESHOLD", PEAK_THRESHOLD, 
			      "Threshold in fADC units for hit amplitudes (default: 100.)");

  	LOW_TCUT = 0.;
  	//HIGH_TCUT = 1110.;
    app->SetDefaultParameter("TRD:LOW_TCUT", LOW_TCUT, 
			      "Throw away hits which come before this time (default: 0.)");
    //app->SetDefaultParameter("TRD:HIGH_TCUT", HIGH_TCUT, 
	//		      "Throw away hits which come after this time (default: 1110.)");
	
	return;
}

//------------------
// BeginRun
//------------------
void DTRDHit_factory_Calib::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	auto runnumber = event->GetRunNumber();

	map<string,string> installed;
	DEvent::GetCalib(event, "/TRD/install_status", installed);
	if(atoi(installed["status"].data()) == 0)
		INSTALLED = false;
	else
		INSTALLED = true;
		
	if(!INSTALLED) return;

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

	
	// load base time offset
	map<string,double> base_time_offset;
	if (DEvent::GetCalib(event, "/TRD/base_time_offset",base_time_offset))
		jout << "Error loading /TRD/base_time_offset !" << endl;
	else if (base_time_offset.find("plane1") != base_time_offset.end() && base_time_offset.find("plane2") != base_time_offset.end()) {
		t_base[0] = base_time_offset["plane1"];
		t_base[1] = base_time_offset["plane2"];
	}
	else
		jerr << "Error parsing /TRD/base_time_offset !" << endl;
	
	// load geometry info so that we know how many strips are in each plane
	int num_x_strips = 0, num_y_strips = 0;
	map<string,double> geometry_info;
	if (DEvent::GetCalib(event, "/TRD/trd_geometry",geometry_info))
		jout << "Error loading /TRD/trd_geometry !" << endl;
	else if (geometry_info.find("num_x_strips") != geometry_info.end() && geometry_info.find("num_y_strips") != base_time_offset.end()) {
		num_x_strips = geometry_info["num_x_strips"];
		num_y_strips = geometry_info["num_y_strips"];
	}
	else
		jerr << "Error parsing /TRD/trd_geometry !" << endl;	
	
	// load constant tables
	trd_digi_constants_t empty_table;
	time_offsets.push_back(empty_table);
	time_offsets.push_back(empty_table);
	if (DEvent::GetCalib(event, "/TRD/plane1/timing_offsets", time_offsets[0]))
		jout << "Error loading /TRD/plane1/timing_offsets !" << endl;
	if (DEvent::GetCalib(event, "/TRD/plane2/timing_offsets", time_offsets[1]))
		jout << "Error loading /TRD/plane2/timing_offsets !" << endl;
	
	if(static_cast<long int>(time_offsets[0].size()) != num_x_strips)
		jerr << "Error loading TRD plane 1 timing offsets (found " << time_offsets[0].size() 
			 << " entries, expected " << num_x_strips << " entries)" << endl;
	if(static_cast<long int>(time_offsets[1].size()) != num_y_strips)
		jerr << "Error loading TRD plane 2 timing offsets (found " << time_offsets[1].size() 
			 << " entries, expected " << num_x_strips << " entries)" << endl;
	
	vector<int> empty_table_int;
	strip_quality.push_back(empty_table_int);
	strip_quality.push_back(empty_table_int);
	if (DEvent::GetCalib(event, "/TRD/plane1/strip_quality", strip_quality[0]))
		jout << "Error loading /TRD/plane1/strip_quality !" << endl;
	if (DEvent::GetCalib(event, "/TRD/plane2/strip_quality", strip_quality[1]))
		jout << "Error loading /TRD/plane2/strip_quality !" << endl;
	
	if(static_cast<long int>(strip_quality[0].size()) != num_x_strips)
		jerr << "Error loading TRD plane 1 strip qualities (found " << strip_quality[0].size() 
			 << " entries, expected " << num_x_strips << " entries)" << endl;
	if(static_cast<long int>(strip_quality[1].size()) != num_y_strips)
		jerr << "Error loading TRD plane 2 strip qualities (found " << strip_quality[1].size() 
			 << " entries, expected " << num_x_strips << " entries)" << endl;
	
	

	// also set time window from CCDB

    return;
}

//------------------
// Process
//------------------
void DTRDHit_factory_Calib::Process(const std::shared_ptr<const JEvent>& event)
{
    /// Generate DTRDHit object for each DTRDDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

	if(!INSTALLED) return;

    vector<const DTRDDigiHit*> digihits;
    event->Get(digihits);
    
    // make hits out of all DTRDDigiHit objects	
    for (unsigned int i=0; i < digihits.size(); i++) {
	    const DTRDDigiHit *digihit = digihits[i];
	    
	    // initial firmware version generated a bunch of junk hits with pulse_time=179
	    // explicitly reject these
	    if(digihit->pulse_time == 179)
	    	continue;

        // throw away hits from bad or noisy counters
        int quality = strip_quality[digihit->plane-1][digihit->strip-1];
        if (quality == k_counter_bad || quality == k_counter_noisy)
            continue;
	    
		// The translation table has:
		// ---------------------------------------------------
		// plane   : 1-2
		// strip   : 1-720 (plane 1) or 1-432 (plane 2)
		//
      
        // Grab the pedestal from the digihit 
        int raw_ped = digihit->pedestal;

        // There are a few values from the new data type that are critical for the interpretation of the data
        uint16_t ABIT = 0; // 2^{ABIT} Scale factor for amplitude
      	uint16_t PBIT = 0; // 2^{PBIT} Scale factor for pedestal
      	uint16_t NW   = 0; //HIGH_TCUT/t_scale;
     	//uint16_t IE   = 0;

      	int pulse_peak = 0;
      	int scaled_ped = 0;
      	const Df125FDCPulse *FDCPulseObj = NULL;
      	digihit->GetSingle(FDCPulseObj);
      	if(FDCPulseObj != nullptr) {
        	vector<const Df125Config*> configs;
        	digihit->Get(configs);

        	if(!configs.empty()){
            	const Df125Config *config = configs[0];
            	//IBIT = config->IBIT == 0xffff ? 4 : config->IBIT;
            	ABIT = config->ABIT == 0xffff ? 3 : config->ABIT;
            	PBIT = config->PBIT == 0xffff ? 0 : config->PBIT;
				//NW   = config->NW   == 0xffff ? 80 : config->NW; ///////////////////////
            	NW   = config->NW;
            	//IE   = config->IE   == 0xffff ? 16 : config->IE;
         	} else {         	
            	static int Nwarnings = 0;
            	if(Nwarnings<10) {
                	_DBG_ << "NO Df125Config object associated with Df125FDCPulse object!" << endl;
               		Nwarnings++;
               		if(Nwarnings==10) _DBG_ << " --- LAST WARNING!! ---" << endl;
            	}
         	}

			// calculate the correct pulse peak and pedestal      	
      		pulse_peak = FDCPulseObj->peak_amp << ABIT;
      		scaled_ped = raw_ped << PBIT;
      	}
		else {
			// DEBUG
			jerr << "DTRDHit_factory: error loading Df125FDCPulse object !" << endl;
		}
	
	    // subtract pedestal
	    double pulse_height = pulse_peak - scaled_ped;
	    
	    if(pulse_height < PEAK_THRESHOLD)
	    	continue;

	    // Time cut now (Set the upper Time Cut based on the firmware timestamp)
	    double T = (double)digihit->peak_time * t_scale;
	    if( (T < LOW_TCUT) || (T > ((NW-22.)*t_scale)) )
	    	continue;
	    
		// Build hit object
	    DTRDHit *hit = new DTRDHit;
	    hit->plane = digihit->plane;
	    hit->strip = digihit->strip;
	    hit->pulse_height = pulse_height;
	    hit->pedestal = scaled_ped;
 
	    // Apply calibration constants
	    hit->t = T + t_base[digihit->plane-1];
	    // hit->t = T + t_base[digihit->plane-1] + time_offsets[digihit->plane-1][digihit->strip-1];

		hit->q = a_scale * hit->pulse_height;  // probably need to set this more sensibly

	    hit->AddAssociatedObject(digihit);
	    
	    Insert(hit);
    }
		    
    return;
}

//------------------
// EndRun
//------------------
void DTRDHit_factory_Calib::EndRun()
{
}

//------------------
// Finish
//------------------
void DTRDHit_factory_Calib::Finish()
{
}
