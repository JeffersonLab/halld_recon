/*
 *  File: DECALHit_factory.cc
 *
 *  Created on 01/16/2024 by A.S.  
 */

#include <iostream>
#include <iomanip>
using namespace std;

#include "ECAL/DECALDigiHit.h"
#include "ECAL/DECALHit_factory.h"

#include "DAQ/Df250PulseIntegral.h"
#include "DAQ/Df250PulsePedestal.h"

#include "DAQ/Df250Config.h"
#include "TTAB/DTTabUtilities.h"
#include "DANA/DEvent.h"



//----------------
// Constructor
//----------------
DECALHit_factory::DECALHit_factory()
{
}


//------------------
// Init
//------------------
void DECALHit_factory::Init(void)
{
    auto app = GetApplication();
    HIT_DEBUG    =  0;
    DB_PEDESTAL  =  1;    //   1  -  take from DB
                          //   0  -  event-by-event pedestal subtraction

    app->SetDefaultParameter("ECAL:HIT_DEBUG",    HIT_DEBUG);
    app->SetDefaultParameter("ECAL:DB_PEDESTAL",  DB_PEDESTAL);

    m_numActiveBlocks=0;
    for (int row=0;row<kECALBlocksTall;row++){
      for (int col=0;col<kECALBlocksWide;col++){
        if (col>=kECALMidBlock-1 && col<=kECALMidBlock
      && row>=kECALMidBlock-1 && row<=kECALMidBlock){
    m_activeBlock[row][col]=false;
        }
        else{
    m_row[m_numActiveBlocks]=row;
    m_column[m_numActiveBlocks]=col;
    m_channelNumber[row][col]=m_numActiveBlocks;
    m_activeBlock[row][col]=true;

    m_numActiveBlocks++;
        }
      }

    }
  
    // initialize calibration tables
    vector< vector<double > > gains_tmp(kECALBlocksTall, 
            vector<double>(kECALBlocksWide));
    vector< vector<double > > pedestals_tmp(kECALBlocksTall, 
            vector<double>(kECALBlocksWide));

    vector< vector<double > > time_offsets_tmp(kECALBlocksTall, 
            vector<double>(kECALBlocksWide));
    vector< vector<double > > adc_offsets_tmp(kECALBlocksTall, 
            vector<double>(kECALBlocksWide));

    gains         =   gains_tmp;
    pedestals     =   pedestals_tmp;
    time_offsets  =   time_offsets_tmp;
    adc_offsets   =   adc_offsets_tmp;


    adc_en_scale    =  0;
    adc_time_scale  =  0;
    
    base_time  = 0;
    

    return; //NOERROR;
}

//------------------
// BeginRun
//------------------
void DECALHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{

    // Only print messages for one thread whenever run number change
    static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
    int runnumber = event->GetRunNumber();
    static set<int> runs_announced;
    pthread_mutex_lock(&print_mutex);
    //    bool print_messages = false;
    if(runs_announced.find(runnumber) == runs_announced.end()){
      //        print_messages = true;
        runs_announced.insert(runnumber);
    }
    pthread_mutex_unlock(&print_mutex);

    vector< double > ecal_gains_ch;
    vector< double > ecal_pedestals_ch;
    vector< double > time_offsets_ch;
    vector< double > adc_offsets_ch;

    // load scale factors
    map<string,double> scale_factors;

    if (DEvent::GetCalib(event, "/ECAL/digi_scales", scale_factors))
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
    if (DEvent::GetCalib(event, "/ECAL/base_time_offset",base_time_offset))
        jout << "Error loading /ECAL/base_time_offset !" << endl;
    if (base_time_offset.find("BASE_TIME") != base_time_offset.end())
        base_time = base_time_offset["BASE_TIME"];
    else
        jerr << "Unable to get BASE_TIME from /ECAL/base_time_offset !" << endl;


    if (DEvent::GetCalib(event, "/ECAL/gains", ecal_gains_ch))
      jout << "DECALHit_factory: Error loading /ECAL/gains !" << endl;
    if (DEvent::GetCalib(event, "/ECAL/pedestals", ecal_pedestals_ch))
      jout << "DECALHit_factory: Error loading /ECAL/pedestals !" << endl;

    if (DEvent::GetCalib(event, "/ECAL/timing_offsets", time_offsets_ch))
        jout << "Error loading /ECAL/timing_offsets !" << endl;
    if (DEvent::GetCalib(event, "/ECAL/adc_offsets", adc_offsets_ch))
        jout << "Error loading /ECAL/adc_offsets !" << endl;


    LoadECALConst(gains, ecal_gains_ch);
    LoadECALConst(pedestals, ecal_pedestals_ch);
    LoadECALConst(time_offsets, time_offsets_ch);
    LoadECALConst(adc_offsets, adc_offsets_ch);

    
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
    
    return; //NOERROR;
}

//------------------
// Process
//------------------
void DECALHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
    /// Generate DECALHit object for each DECALDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

    vector<const DECALDigiHit*> digihits;

    event->Get(digihits);

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
	  
	  if(pulse_amplitude > 0){
	    hit->intOverPeak = (pulse_int - integratedPedestal)/pulse_amplitude;	    
	  } else 
	    hit->intOverPeak = 0;

	  hit->AddAssociatedObject(digihit);
	  Insert(hit);

	}   // Good hit

    }
    
    return; //NOERROR;
}

//------------------
// EndRun
//------------------
void DECALHit_factory::EndRun(void)
{
    return; //NOERROR;
}

//------------------
// Finish
//------------------
void DECALHit_factory::Finish(void)
{
    return; //NOERROR;
}


void  DECALHit_factory::LoadECALConst(ecal_constants_t &table, const vector<double> &ecal_const_ch){
  for (int ch = 0; ch < static_cast<int>(ecal_const_ch.size()); ch++) {

    int row = m_row[ch];
    int col = m_column[ch];
    
    // results from DECALGeometry should be self consistent, but add in some
    // sanity checking just to be sure

    if (isBlockActive(row,col) == false) {
      char str[256];

      sprintf(str, "DECALHit: Loading ECAL constant for inactive channel!  "
              "row=%d, col=%d", row, col);
      throw JException(str);
    }    
    
    table[row][col] = ecal_const_ch[ch];
  }
  
}



