// $Id$
//
//    File: DTRDHit_factory.cc
//

#include <iostream>
#include <iomanip>
using namespace std;

#include <DAQ/Df125Config.h>
#include <DAQ/Df125FDCPulse.h>
#include <TH1.h>
#include <TH2.h>

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

    /// set the base conversion scales
    a_scale      = 2.4E4/1.3E5;  // NOTE: currently fixed to FDC values, currently not used
    a_scale=1.;                  // to test with real fADC pulses
    t_scale      = 8.0/10.0;     // 8 ns/count and integer time is in 1/10th of sample
    t_scale=8.;
    t_base       = { 0.,  0.};   // ns, per plane
    
    PEAK_THRESHOLD = 100.;  // fADC units
    gPARMS->SetDefaultParameter("TRD:PEAK_THRESHOLD", PEAK_THRESHOLD, 
			      "Threshold in fADC units for hit amplitudes (default: 100.)");

  	LOW_TCUT = -10000.;
  	HIGH_TCUT = 10000.;
    gPARMS->SetDefaultParameter("TRD:LOW_TCUT", LOW_TCUT, 
			      "Throw away hits which come before this time (default: -10000.)");
    gPARMS->SetDefaultParameter("TRD:HIGH_TCUT", HIGH_TCUT, 
			      "Throw away hits which come after this time (default: 10000.)");

	IS_XY_TIME_DIFF_CUT = false;
	gPARMS->SetDefaultParameter("TRD:IS_XY_TIME_DIFF_CUT", IS_XY_TIME_DIFF_CUT, 
			      "Apply time difference cut between X and Y hits (default: false)");

	XY_TIME_DIFF = 20.;
	gPARMS->SetDefaultParameter("TRD:XY_TIME_DIFF", XY_TIME_DIFF, 
			      "Time difference between hits in X and Y planes to be considered a coincidence (default: 20.)");

	xycorr = new TH2I("xycorr","x-y hit correlation",720,-0.5,719.5,720,-0.5,719.5);
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

	
	// load base time offset
	map<string,double> base_time_offset;
	if (eventLoop->GetCalib("/TRD/base_time_offset",base_time_offset))
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
	if (eventLoop->GetCalib("/TRD/trd_geometry",geometry_info))
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
	if (eventLoop->GetCalib("/TRD/plane1/timing_offsets", time_offsets[0]))
		jout << "Error loading /TRD/plane1/timing_offsets !" << endl;
	if (eventLoop->GetCalib("/TRD/plane2/timing_offsets", time_offsets[1]))
		jout << "Error loading /TRD/plane2/timing_offsets !" << endl;
	
	if(time_offsets[0].size() != num_x_strips)
		jerr << "Error loading TRD plane 1 timing offsets (found " << time_offsets[0].size() 
			 << " entries, expected " << num_x_strips << " entries)" << endl;
	if(time_offsets[1].size() != num_y_strips)
		jerr << "Error loading TRD plane 2 timing offsets (found " << time_offsets[1].size() 
			 << " entries, expected " << num_x_strips << " entries)" << endl;
	
	// also set time window from CCDB

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

	vector<DTRDHit*> hits_plane[2]; // one for each plane
	vector<DTRDHit*> hits_pamp[2][10]; // one for each pre-amp board
    
    // make hits out of all DTRDDigiHit objects	
    for (unsigned int i=0; i < digihits.size(); i++) {
	    const DTRDDigiHit *digihit = digihits[i];
	    
	    // initial firmware version generated a bunch of junk hits with pulse_time=179
	    // explicitly reject these
	    if(digihit->peak_time == 179)
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
      	//uint16_t NW   = 0;
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
            	//NW   = config->NW   == 0xffff ? 80 : config->NW;
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
      		pulse_peak = digihit->pulse_peak << ABIT;
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

	    // Time cut now
	    //double T = (double)digihit->pulse_time * t_scale;
	    double T = (double)digihit->peak_time * t_scale;
	    if( (T < LOW_TCUT) || (T > HIGH_TCUT) )
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

		if (!IS_XY_TIME_DIFF_CUT) _data.push_back(hit);
		else {
			hits_plane[digihit->plane-1].push_back(hit);
		}
		// _data.push_back(hit);
    }

    
    int nfreq=17;

	// loops to check the time coincidence of hits in the two planes and add them to the _data vector
	if (IS_XY_TIME_DIFF_CUT) {
		float phase[2][10][20]={0};
		for (unsigned int i=0; i < hits_plane[0].size(); i++) {
		  if(hits_plane[0].size()>60000)continue;
			for (unsigned int j=0; j < hits_plane[1].size(); j++) {
			  if(hits_plane[1].size()>60000)continue;
			//	cout<<" i,j,diff="<<i<<" "<<j<<" "<<abs(hits_plane[0][i]->t - hits_plane[1][j]->t)<<endl;
                                int xstrip=hits_plane[0][i]->strip;
				int ystrip=hits_plane[1][j]->strip;
				//if(ystrip!=232&&(xstrip<600||(ystrip!=3&&ystrip!=4&&ystrip!=233&&ystrip!=193&&ystrip!=195&&ystrip!=196&&(ystrip<337||ystrip>344)&&(ystrip<313||ystrip>335)))){
				if (abs(hits_plane[0][i]->t - hits_plane[1][j]->t) < XY_TIME_DIFF) {

					//_data.push_back(hits_plane[0][i]);
			                int pamp=(hits_plane[0][i]->strip)/120;
					hits_pamp[0][pamp].push_back(hits_plane[0][i]);
					// time in sample modulo nfreq (number of samples correspoding to the noise freq.)
					// f ranges from 0 to nfreq-1
					int ph=((int)hits_plane[0][i]->t/8)%nfreq;
					//cout<<" x,pamp,size,f= "<<pamp<<" "<<hits_pamp[0][pamp].size()<<" "<<f<<endl;;
					phase[0][pamp][ph]+=1;
					xycorr->Fill(xstrip,ystrip);
					break;
				}
				//}
			}
		}
	
		for (unsigned int i=0; i < hits_plane[1].size(); i++) {
		  if(hits_plane[1].size()>60000)continue;
			for (unsigned int j=0; j < hits_plane[0].size(); j++) {
				if(hits_plane[0].size()>60000)continue;
				if (abs(hits_plane[1][i]->t - hits_plane[0][j]->t) < XY_TIME_DIFF) {
					//_data.push_back(hits_plane[1][i]);
			                int pamp=(hits_plane[1][i]->strip)/120;
					if(pamp>6)cout<<" y plane strip,pamp= "<<hits_plane[1][i]->strip<<" "<<pamp<<endl;
					hits_pamp[1][pamp].push_back(hits_plane[1][i]);
					int ph=((int)hits_plane[1][i]->t/8)%nfreq;
					//cout<<" y,pamp,size,f= "<<pamp<<" "<<hits_pamp[1][pamp].size()<<" "<<f<<endl;;
					phase[1][pamp][ph]+=1;
					break;
				}
			}
		}	

        int fmax[2][10]={-1};	
        int amax[2][10]={0};	
	for (int plane=0;plane<2;plane++){
	for (int pamp=0;pamp<10;pamp++){
		for (unsigned int i=0; i < nfreq; i++) {
                    int fp=phase[plane][pamp][i];
		    if(fp>amax[plane][pamp]){
		       amax[plane][pamp]=fp;
		       fmax[plane][pamp]=i;
		    }
		//if(hits_pamp[plane][pamp].size()>0)cout<<" hit_size,plne,pamp,i,phase,fmax= "<<hits_pamp[plane][pamp].size()<<" "<<plane<<" "<<pamp<<" "<<i<<" "<<phase[plane][pamp][i]<<" "<<fmax[plane][pamp]<<endl;		
		}
	}
	}


		
	for (int plane=0;plane<2;plane++){
	for (int pamp=0;pamp<10;pamp++){
		for (unsigned int i=0; i < hits_pamp[plane][pamp].size(); i++) {
		//cout<<" pamp= "<<pamp<<endl;
		// ff -> phase of the hit time
		int ff=((int)hits_pamp[plane][pamp][i]->t/8)%nfreq;
		int diff=abs(ff-fmax[plane][pamp]);
		int cdiff=std::min(diff,nfreq-diff);
		//if(!(amax[plane][pamp]<3||cdiff>3)){
		// to do: set the thresholds for amax and cdiff (5 and 3 in the example bewlow) parameters
		if(!(amax[plane][pamp]<20||cdiff>17)){
			//cout<<" plane,pamp,t,strip= "<<plane<<" "<<pamp<<" "<<(hits_pamp[plane][pamp][i]->t/8)<<" "<<hits_pamp[plane][pamp][i]->strip<<endl; 
		} else {
			_data.push_back(hits_pamp[plane][pamp][i]);
		}
		}
	}
	}

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
