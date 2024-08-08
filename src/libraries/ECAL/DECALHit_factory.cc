/*
 *  File: DECALHit_factory.cc
 *
 *  Created on 01/16/2024 by A.S.  
 */

#include <iostream>
#include <iomanip>
using namespace std;

#include "ECAL/DECALDigiHit.h"
#include "ECAL/DECALGeometry.h"
#include "ECAL/DECALHit_factory.h"

#include "DAQ/Df250PulseIntegral.h"
#include "DAQ/Df250PulsePedestal.h"

#include "DAQ/Df250Config.h"
#include "TTAB/DTTabUtilities.h"

using namespace jana;

//----------------
// Constructor
//----------------
DECALHit_factory::DECALHit_factory(){

  HIT_DEBUG    =  0;
  DB_PEDESTAL  =  1;    //   1  -  take from DB
                        //   0  -  event-by-event pedestal subtraction
  
  gPARMS->SetDefaultParameter("ECAL:HIT_DEBUG",    HIT_DEBUG);
  gPARMS->SetDefaultParameter("ECAL:DB_PEDESTAL",  DB_PEDESTAL);

}


//------------------
// init
//------------------
jerror_t DECALHit_factory::init(void)
{
    // initialize calibration tables
    vector< vector<double > > gains_tmp(DECALGeometry::kECALBlocksTall, 
            vector<double>(DECALGeometry::kECALBlocksWide));
    vector< vector<double > > pedestals_tmp(DECALGeometry::kECALBlocksTall, 
            vector<double>(DECALGeometry::kECALBlocksWide));

    vector< vector<double > > time_offsets_tmp(DECALGeometry::kECALBlocksTall, 
            vector<double>(DECALGeometry::kECALBlocksWide));
    vector< vector<double > > adc_offsets_tmp(DECALGeometry::kECALBlocksTall, 
            vector<double>(DECALGeometry::kECALBlocksWide));

    gains         =   gains_tmp;
    pedestals     =   pedestals_tmp;
    time_offsets  =   time_offsets_tmp;
    adc_offsets   =   adc_offsets_tmp;


    adc_en_scale    =  0;
    adc_time_scale  =  0;
    
    base_time  = 0;
    

    return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DECALHit_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{

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

    // extract the ECAL Geometry
    vector<const DECALGeometry*> ecalGeomVect;
    eventLoop->Get( ecalGeomVect );
    if (ecalGeomVect.size() < 1)
      return OBJECT_NOT_AVAILABLE;
    const DECALGeometry& ecalGeom = *(ecalGeomVect[0]);


    vector< double > ecal_gains_ch;
    vector< double > ecal_pedestals_ch;
    vector< double > time_offsets_ch;
    vector< double > adc_offsets_ch;

    // load scale factors
    map<string,double> scale_factors;

    if (eventLoop->GetCalib("/ECAL/digi_scales", scale_factors))
        jout << "Error loading /ECAL/digi_scales !" << endl;
    if (scale_factors.find("ADC_EN_SCALE") != scale_factors.end())
        adc_en_scale = scale_factors["ADC_EN_SCALE"];
    else
        jerr << "Unable to get ADC_EN_SCALE from /ECAL/digi_scales !" << endl;
    if (scale_factors.find("ADC_TIME_SCALE") != scale_factors.end())
        adc_time_scale = scale_factors["ADC_TIME_SCALE"];
    else
        jerr << "Unable to get ADC_TIME_SCALE from /ECAL/digi_scales !" << endl;

    map<string,double> base_time_offset;
    if (eventLoop->GetCalib("/ECAL/base_time_offset",base_time_offset))
        jout << "Error loading /ECAL/base_time_offset !" << endl;
    if (base_time_offset.find("BASE_TIME") != base_time_offset.end())
        base_time = base_time_offset["BASE_TIME"];
    else
        jerr << "Unable to get BASE_TIME from /ECAL/base_time_offset !" << endl;


    if (eventLoop->GetCalib("/ECAL/gains", ecal_gains_ch))
      jout << "DECALHit_factory: Error loading /ECAL/gains !" << endl;
    if (eventLoop->GetCalib("/ECAL/pedestals", ecal_pedestals_ch))
      jout << "DECALHit_factory: Error loading /ECAL/pedestals !" << endl;

    if (eventLoop->GetCalib("/ECAL/timing_offsets", time_offsets_ch))
        jout << "Error loading /ECAL/timing_offsets !" << endl;
    if (eventLoop->GetCalib("/ECAL/adc_offsets", adc_offsets_ch))
        jout << "Error loading /ECAL/adc_offsets !" << endl;


    LoadECALConst(gains, ecal_gains_ch, ecalGeom);
    LoadECALConst(pedestals, ecal_pedestals_ch, ecalGeom);
    LoadECALConst(time_offsets, time_offsets_ch, ecalGeom);
    LoadECALConst(adc_offsets, adc_offsets_ch, ecalGeom);

    
    if(HIT_DEBUG  == 1){


      cout << endl;
      cout << " -------  Gains -----------" << endl;

      for(int ii = 0; ii < 40; ii++){
	for(int jj = 0; jj < 40; jj++){
	  cout << "  " << gains[ii][jj];	
	}
	cout << endl;
      }
      
      cout << endl;
      cout << " -------  Pedestals -----------" << endl;
      
      for(int ii = 0; ii < 40; ii++){
	for(int jj = 0; jj < 40; jj++){
	  cout << "  " << pedestals[ii][jj];	
	}
	cout << endl;
      }
      
      cout << endl;
      cout << " -------  Timing offsets -----------" << endl;
      
      for(int ii = 0; ii < 40; ii++){
	for(int jj = 0; jj < 40; jj++){
	  cout << "  " << time_offsets[ii][jj];	
	}
	cout << endl;
      }
      
      cout << endl;
      cout << " -------  ADC offsets -----------" << endl;
      
      for(int ii = 0; ii < 40; ii++){
	for(int jj = 0; jj < 40; jj++){
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
    
    return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DECALHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
    /// Generate DECALHit object for each DECALDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.


    // extract the ECAL Geometry
    vector<const DECALGeometry*> ecalGeomVect;
    eventLoop->Get( ecalGeomVect );
    if (ecalGeomVect.size() < 1)
      return OBJECT_NOT_AVAILABLE;
    const DECALGeometry& ecalGeom = *(ecalGeomVect[0]);


    vector<const DECALDigiHit*> digihits;

    loop->Get(digihits);

    for (unsigned int i = 0; i < digihits.size(); i++) {

        const DECALDigiHit *digihit = digihits[i];

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
	  DECALHit *hit = new DECALHit;
	  
	  hit->row    = digihit->row;
	  hit->column = digihit->column;
	  
	  hit->E = pulse_int_ped_subt;
	  hit->t = pulse_time_correct;
	  
	  // Get position of blocks on front face. (This should really come from
	  // hdgeant directly so the poisitions can be shifted in mcsmear.)
	  DVector2 pos = ecalGeom.positionOnFace(hit->row, hit->column);
	  hit->x = pos.X();
	  hit->y = pos.Y();
	  
	  if(pulse_amplitude > 0){
	    hit->intOverPeak = (pulse_int - integratedPedestal)/pulse_amplitude;	    
	  } else 
	    hit->intOverPeak = 0;

	  hit->AddAssociatedObject(digihit);
	  _data.push_back(hit);

	}   // Good hit

    }
    
    return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DECALHit_factory::erun(void)
{
    return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DECALHit_factory::fini(void)
{
    return NOERROR;
}


void  DECALHit_factory::LoadECALConst(ecal_constants_t &table, const vector<double> &ecal_const_ch, 
                                         const DECALGeometry  &ecalGeom){
  
  char str[256];
  
  //  if (ecalGeom.numActiveBlocks() != ECAL_MAX_CHANNELS) {
  //    sprintf(str, "ECAL geometry is wrong size! channels=%d (should be %d)", 
  //            ecalGeom.numActiveBlocks(), ECAL_MAX_CHANNELS);
  //    throw JException(str);
  //  }
  
  
  for (int ch = 0; ch < static_cast<int>(ecal_const_ch.size()); ch++) {
    
    // make sure that we don't try to load info for channels that don't exist
    if (ch == ecalGeom.numActiveBlocks())
      break;
    
    int row = ecalGeom.row(ch);
    int col = ecalGeom.column(ch);
    
    // results from DECALGeometry should be self consistent, but add in some
    // sanity checking just to be sure

    if (ecalGeom.isBlockActive(row,col) == false) {
      sprintf(str, "DECALHit: Loading ECAL constant for inactive channel!  "
              "row=%d, col=%d", row, col);
      throw JException(str);
    }    
    
    table[row][col] = ecal_const_ch[ch];
  }
  
}



