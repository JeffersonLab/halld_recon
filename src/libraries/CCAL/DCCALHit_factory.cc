/*
 *  File: DCCALHit_factory.cc
 *
 * Created on 11/25/18 by A.S. 
 */

#include <iostream>
#include <iomanip>
using namespace std;

#include "CCAL/DCCALDigiHit.h"
#include "CCAL/DCCALGeometry.h"
#include "CCAL/DCCALHit_factory.h"

#include "DAQ/Df250PulseIntegral.h"
#include "DAQ/Df250PulsePedestal.h"

#include "DAQ/Df250Config.h"
#include "TTAB/DTTabUtilities.h"

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include "DANA/DEvent.h"

//----------------
// Constructor
//----------------
DCCALHit_factory::DCCALHit_factory(){}


//------------------
// Init
//------------------
void DCCALHit_factory::Init()
{
	auto app = GetApplication();

	HIT_DEBUG    =  0;
	DB_PEDESTAL  =  1;    //   1  -  take from DB
	//   0  -  event-by-event pedestal subtraction

	app->SetDefaultParameter("CCAL:HIT_DEBUG",    HIT_DEBUG);
	app->SetDefaultParameter("CCAL:DB_PEDESTAL",  DB_PEDESTAL);

	// Initialize calibration tables
    vector< vector<double > > gains_tmp(DCCALGeometry::kCCALBlocksTall, 
            vector<double>(DCCALGeometry::kCCALBlocksWide));
    vector< vector<double > > pedestals_tmp(DCCALGeometry::kCCALBlocksTall, 
            vector<double>(DCCALGeometry::kCCALBlocksWide));

    vector< vector<double > > time_offsets_tmp(DCCALGeometry::kCCALBlocksTall, 
            vector<double>(DCCALGeometry::kCCALBlocksWide));
    vector< vector<double > > adc_offsets_tmp(DCCALGeometry::kCCALBlocksTall, 
            vector<double>(DCCALGeometry::kCCALBlocksWide));

    gains         =   gains_tmp;
    pedestals     =   pedestals_tmp;
    time_offsets  =   time_offsets_tmp;
    adc_offsets   =   adc_offsets_tmp;


    adc_en_scale    =  0;
    adc_time_scale  =  0;
    
    base_time  = 0;
    
    INSTALLED = false;
    
}

//------------------
// BeginRun
//------------------
void DCCALHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	map<string,string> installed;
	DEvent::GetCalib(event, "/CCAL/install_status", installed);
	if(atoi(installed["status"].data()) == 0)
		INSTALLED = false;
	else
		INSTALLED = true;
		
	if(!INSTALLED) return;

	auto runnumber = event->GetRunNumber();
	auto app = event->GetJApplication();
	auto calibration = app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);

    // Only print messages for one thread whenever run number change
    static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
    static set<int> runs_announced;
    pthread_mutex_lock(&print_mutex);
    //    bool print_messages = false;
    if(runs_announced.find(runnumber) == runs_announced.end()){
      //        print_messages = true;
        runs_announced.insert(runnumber);
    }
    pthread_mutex_unlock(&print_mutex);

    // extract the CCAL Geometry
    vector<const DCCALGeometry*> ccalGeomVect;
    event->Get( ccalGeomVect );
    if (ccalGeomVect.size() < 1)
      return; // OBJECT_NOT_AVAILABLE;
    const DCCALGeometry& ccalGeom = *(ccalGeomVect[0]);


    vector< double > ccal_gains_ch;
    vector< double > ccal_pedestals_ch;
    vector< double > time_offsets_ch;
    vector< double > adc_offsets_ch;

    // load scale factors
    map<string,double> scale_factors;

    if (calibration->Get("/CCAL/digi_scales", scale_factors))
        jout << "Error loading /CCAL/digi_scales !" << jendl;
    if (scale_factors.find("ADC_EN_SCALE") != scale_factors.end())
        adc_en_scale = scale_factors["ADC_EN_SCALE"];
    else
        jerr << "Unable to get ADC_EN_SCALE from /CCAL/digi_scales !" << endl;
    if (scale_factors.find("ADC_TIME_SCALE") != scale_factors.end())
        adc_time_scale = scale_factors["ADC_TIME_SCALE"];
    else
        jerr << "Unable to get ADC_TIME_SCALE from /CCAL/digi_scales !" << endl;

    map<string,double> base_time_offset;
    if (calibration->Get("/CCAL/base_time_offset",base_time_offset))
        jout << "Error loading /CCAL/base_time_offset !" << jendl;
    if (base_time_offset.find("BASE_TIME") != base_time_offset.end())
        base_time = base_time_offset["BASE_TIME"];
    else
        jerr << "Unable to get BASE_TIME from /CCAL/base_time_offset !" << endl;


    if (calibration->Get("/CCAL/gains", ccal_gains_ch))
      jout << "DCCALHit_factory: Error loading /CCAL/gains !" << jendl;
    if (calibration->Get("/CCAL/pedestals", ccal_pedestals_ch))
      jout << "DCCALHit_factory: Error loading /CCAL/pedestals !" << jendl;

    if (calibration->Get("/CCAL/timing_offsets", time_offsets_ch))
        jout << "Error loading /CCAL/timing_offsets !" << jendl;
    if (calibration->Get("/CCAL/adc_offsets", adc_offsets_ch))
        jout << "Error loading /CCAL/adc_offsets !" << jendl;


    LoadCCALConst(gains, ccal_gains_ch, ccalGeom);
    LoadCCALConst(pedestals, ccal_pedestals_ch, ccalGeom);
    LoadCCALConst(time_offsets, time_offsets_ch, ccalGeom);
    LoadCCALConst(adc_offsets, adc_offsets_ch, ccalGeom);

    
    if(HIT_DEBUG  == 1){


      cout << endl;
      cout << " -------  Gains -----------" << endl;

      for(int ii = 0; ii < 12; ii++){
	for(int jj = 0; jj < 12; jj++){
	  cout << "  " << gains[ii][jj];	
	}
	cout << endl;
      }
      
      cout << endl;
      cout << " -------  Pedestals -----------" << endl;
      
      for(int ii = 0; ii < 12; ii++){
	for(int jj = 0; jj < 12; jj++){
	  cout << "  " << pedestals[ii][jj];	
	}
	cout << endl;
      }
      
      cout << endl;
      cout << " -------  Timing offsets -----------" << endl;
      
      for(int ii = 0; ii < 12; ii++){
	for(int jj = 0; jj < 12; jj++){
	  cout << "  " << time_offsets[ii][jj];	
	}
	cout << endl;
      }
      
      cout << endl;
      cout << " -------  ADC offsets -----------" << endl;
      
      for(int ii = 0; ii < 12; ii++){
	for(int jj = 0; jj < 12; jj++){
	  cout << "  " << adc_offsets[ii][jj];	
	}
	cout << endl;
      }
      
      cout << endl;
      cout << "ADC_EN_SCALE   = " << adc_en_scale << endl;
      cout << "ADC_TIME_SCALE = " << adc_time_scale << endl;
      
      cout << endl;
      
      cout << "BASE_TIME_OFFSET = " << base_time << endl;
      
      cout << endl;

    }     
}

//------------------
// Process
//------------------
void DCCALHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
    /// Generate DCCALHit object for each DCCALDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

	if(!INSTALLED) return;

    // extract the CCAL Geometry
    vector<const DCCALGeometry*> ccalGeomVect;
    event->Get( ccalGeomVect );
    if (ccalGeomVect.size() < 1)
      return; // OBJECT_NOT_AVAILABLE;
    const DCCALGeometry& ccalGeom = *(ccalGeomVect[0]);


    vector<const DCCALDigiHit*> digihits;

    event->Get(digihits);

    for (unsigned int i = 0; i < digihits.size(); i++) {

        const DCCALDigiHit *digihit = digihits[i];

        double nsamples_integral   =  digihit->nsamples_integral;
	double nsamples_pedestal   =  digihit->nsamples_pedestal; 


	// Currently use pedestals from the DB. We'll switch to the
	// event-by-event pedestal subtraction later

	double pedestal = 0;
	
	if(DB_PEDESTAL == 1) 
	  pedestal = pedestals[digihit->row][digihit->column];
	else {
	  pedestal = (double)digihit->pedestal/nsamples_pedestal;
	}


	double integratedPedestal  =  pedestal * nsamples_integral;
        double pulse_amplitude     =  digihit->pulse_peak - pedestal;

        double pulse_int   =  (double)digihit->pulse_integral;
        double pulse_time  =  (double)digihit->pulse_time;


	if(HIT_DEBUG == 1)
	  cout << "Row = " << digihit->row << "Column = " << digihit->column << 
	    "  pulse_int =  "   << pulse_int <<  "  integratedPedestal = " << integratedPedestal << 
	    "  Pedestal  =  " << pedestal <<  endl;
	
        double pulse_int_ped_subt   =  adc_en_scale * gains[digihit->row][digihit->column] * (pulse_int - integratedPedestal);
        double pulse_time_correct   =  adc_time_scale * pulse_time + base_time - time_offsets[digihit->row][digihit->column] + 
	  adc_offsets[digihit->row][digihit->column];
	
	
	if(pulse_int_ped_subt > 0 && pulse_time > 0){
	  
	  // Build hit object
	  DCCALHit *hit = new DCCALHit;
	  
	  hit->row    = digihit->row;
	  hit->column = digihit->column;
	  
	  hit->E = pulse_int_ped_subt;
	  hit->t = pulse_time_correct;
	  
	  // Get position of blocks on front face. (This should really come from
	  // hdgeant directly so the poisitions can be shifted in mcsmear.)
	  DVector2 pos = ccalGeom.positionOnFace(hit->row, hit->column);
	  hit->x = pos.X();
	  hit->y = pos.Y();
	  
	  if(pulse_amplitude > 0){
	    hit->intOverPeak = (pulse_int - integratedPedestal)/pulse_amplitude;	    
	  } else 
	    hit->intOverPeak = 0;

	  hit->AddAssociatedObject(digihit);
	  Insert(hit);

	}   // Good hit

    }
    return;
}

//------------------
// EndRun
//------------------
void DCCALHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DCCALHit_factory::Finish()
{
}


void  DCCALHit_factory::LoadCCALConst(ccal_constants_t &table, 
	const vector<double> &ccal_const_ch, const DCCALGeometry  &ccalGeom){
	
	char str[256];
	
	//if (ccalGeom.numActiveBlocks() != CCAL_MAX_CHANNELS) {
	//    sprintf(str, "CCAL geometry is wrong size! channels=%d (should be %d)", 
	//        ccalGeom.numActiveBlocks(), CCAL_MAX_CHANNELS);
	//    throw JException(str);
	//}
	
	
	for (int ch = 0; ch < static_cast<int>(ccal_const_ch.size()); ch++) {
		
		// make sure that we don't try to load info for channels that don't exist
		if (ch == ccalGeom.numActiveBlocks())
			break;
		
		int row = ccalGeom.row(ch);
		int col = ccalGeom.column(ch);
		
		// results from DCCALGeometry should be self consistent, but add in some
		// sanity checking just to be sure
		
		if (ccalGeom.isBlockActive(row,col) == false) {
			sprintf(str, "DCCALHit: Loading CCAL constant for inactive channel!  "
				"row=%d, col=%d", row, col);
			throw JException(str);
		}
		
		table[row][col] = ccal_const_ch[ch];
	}
	
}
