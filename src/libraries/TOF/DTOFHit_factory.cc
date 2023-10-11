// $Id$
//
/*! **File**: DTOFHit_factory.cc
 *+ Created: Wed Aug  7 09:30:17 EDT 2013
 *+ Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
 *+ Purpose: Implementation of the DTOFHit factory. Creating DTOFHit objects from raw ADC
 * and TDC data (DTOFDigiHit and DTOFTDCDigiHit objects, respectively). Matches between
 * ADC and TDC hits are identified within a given time window. Calibration parameters
 * for timing and energy conversion are loaded from CCDB and updated in the method ::brun()
 * any time the run number changes. For found matches between ADC and TDC hits the TDC timing
 * is corrected for detector walk based on the associated ADC amplituded.
*/
/// \addtogroup TOFDetector


/*! \file DTOFHit_factory.cc implementation of the 5 JFactory member functions
 * the execution of these functions are controlled by the JANA/DANA framework
 * \fn ::init() called at start up
 *              initialize basic variables that do not depend on the run number
 * \fn ::brun() called whenever the run number changes
 *              initialize variables that can change from run to run
 *              read these calibration parameters from CCDB and RCDB
 * \fn ::evnt() called for every event
 *              process event, get raw TOF data from ADC and TDC match these hits based on timing
 *              after timing offset parameters are applied, the TDC timing will be corrected for 
 *              detector walk in case of matched hits
 * \fn ::erun() called at the end of a run
 * \fn ::fini() called at the end of the last processed event
 
 * additional helper methods are implemented:
 * \fn ::FindMatch() find match in ADC raw data for raw TDC time data, return ADC match
 * match to be found within time window: DELTA_T_ADC_TDC_MAX defaults to 20ns can be modified
 * by command line: -PTOF:DELTA_T_ADC_TDC_MAX=XX where XX is time window in ns 
 * \param plane:
 * \param bar:
 * \param end: 
 * \param T: TDC time

 * \fn ::FillCalibTable()
 * \fn ::GetConstant(), overloaded 4 times

 * there are 5 different implementation of the walk correction, the last one CalcWalkCorrNEW5PAR() is
 * the latest and currently used algorithm as defined in the table TOF2/walkcorr_type 
 * \fn ::CalcWalkCorrIntegral()
 * \fn ::CalcWalkCorrAmplitude()
 * \fn ::CalcWalkCorrNEW()
 * \fn ::CalcWalkCorrNEWAMP()
 * \fn ::CalcWalkCorrNEW5PAR()

 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <limits>

#include <TMath.h>

using namespace std;

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include <TOF/DTOFDigiHit.h>
#include <TOF/DTOFTDCDigiHit.h>
#include "DTOFHit_factory.h"
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250Config.h>
#include <DAQ/DCODAROCInfo.h>
#include <DANA/DEvent.h>


static bool COSMIC_DATA = false;
static bool OVERRIDE_HIGH_TIME_CUT = false;
static bool OVERRIDE_LOW_TIME_CUT = false;

int TOF_DEBUG = 0;

//------------------
// Init
//------------------
void DTOFHit_factory::Init()
{
  auto app = GetApplication();
  app->SetDefaultParameter("TOF:DEBUG_TOF_HITS", TOF_DEBUG,
			      "Generate DEBUG output");

  USE_NEWAMP_4WALKCORR = 0; // flag to use this walk correction
  USE_AMP_4WALKCORR = 0;    // flag to use this walk correction
  USE_NEW_WALK_NEW = 0;     // flag to use this walk correction
  USE_NEW_4WALKCORR = 0; // this is new always zero and not used!

  DELTA_T_ADC_TDC_MAX = 20.0; // ns
  //	DELTA_T_ADC_TDC_MAX = 30.0; // ns, value based on the studies from cosmic events
  app->SetDefaultParameter("TOF:DELTA_T_ADC_TDC_MAX", DELTA_T_ADC_TDC_MAX, 
			      "Maximum difference in ns between a (calibrated) fADC time and F1TDC time for them to be matched in a single hit");
  
  int analyze_cosmic_data = 0;
  app->SetDefaultParameter("TOF:COSMIC_DATA", analyze_cosmic_data,
			      "Special settings for analysing cosmic data");
  if(analyze_cosmic_data > 0)
    COSMIC_DATA = true;
  
  CHECK_FADC_ERRORS = true;
  app->SetDefaultParameter("TOF:CHECK_FADC_ERRORS", CHECK_FADC_ERRORS, "Set to 1 to reject hits with fADC250 errors, set to 0 to keep these hits");

  app->SetDefaultParameter("TOF:OVERRIDE_HIGH_TIME_CUT", OVERRIDE_HIGH_TIME_CUT, "Set to 1 to override the high side time cut, set to 0 to use the values from CCDB");
  app->SetDefaultParameter("TOF:HIGH_TIME_CUT", hi_time_cut, "Set the value of the high side time cut");
  app->SetDefaultParameter("TOF:OVERRIDE_LOW_TIME_CUT", OVERRIDE_LOW_TIME_CUT, "Set to 1 to override the low side time cut, set to 0 to use the values from CCDB");
  app->SetDefaultParameter("TOF:LOW_TIME_CUT", lo_time_cut, "Set the value of the low side time cut");
  
  
  /// Set basic conversion constants
  a_scale    = 0.2/5.2E5;
  t_scale    = 0.0625;   // 62.5 ps/count
  t_base     = 0.;       // ns
  t_base_tdc = 0.; // ns
  
  if(COSMIC_DATA)
    // Hardcoding of 110 taken from cosmics events
    tdc_adc_time_offset = 110.;
  else 
    tdc_adc_time_offset = 0.;
  
  // default values, will override from DTOFGeometry
  TOF_NUM_PLANES = 2;
  TOF_NUM_BARS = 44;
  TOF_MAX_CHANNELS = 176;
}

//------------------
// BeginRun
//------------------
void DTOFHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  /// This method is called every time the run number changes and calibration
  /// parameters can be loaded from CCDB. These parameters include timing offsets
  /// walk fit parameters, attenuation lengths, effective speed of light in paddles,
  /// At this point the selection of which walk correction is to be applied is done.


    auto runnumber = event->GetRunNumber();
    auto calibration = DEvent::GetJCalibration(event);
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
    
    // read in geometry information
    vector<const DTOFGeometry*> tofGeomVect;
    event->Get( tofGeomVect );
    if(tofGeomVect.size()<1)  return; // OBJECT_NOT_AVAILABLE;
    const DTOFGeometry& tofGeom = *(tofGeomVect[0]);
    
    TOF_NUM_PLANES = tofGeom.Get_NPlanes();
    TOF_NUM_BARS = tofGeom.Get_NBars();
    TOF_MAX_CHANNELS = TOF_NUM_PLANES*TOF_NUM_BARS*2;  // total number of bars * 2 ends
    
    /// Read in calibration constants
    vector<double> raw_adc_pedestals;
    vector<double> raw_adc_gains;
    vector<double> raw_adc_offsets;
    vector<double> raw_tdc_offsets;
    vector<double> raw_adc2E;
    
    if(print_messages) jout << "In DTOFHit_factory, loading constants..." << endl;
    
    // load timing cut values
    if(OVERRIDE_HIGH_TIME_CUT && OVERRIDE_LOW_TIME_CUT) {

      double loli = lo_time_cut;
      double hili = hi_time_cut;
      TimeCenterCut = hili - (hili-loli)/2.;
      TimeWidthCut = (hili-loli)/2.;

    } else {

      vector<double> time_cut_values;
      string locTOFHitTimeCutTable = tofGeom.Get_CCDB_DirectoryName() + "/HitTimeCut";
      if(calibration->Get(locTOFHitTimeCutTable.c_str(), time_cut_values)){
	jout << "Error loading " << locTOFHitTimeCutTable << " SET DEFUALT to 0 and 100!" << jendl;
	TimeCenterCut = 0.;
	TimeWidthCut = 100.;
      } else {

	double loli = 0., hili = 0.;
	if(OVERRIDE_LOW_TIME_CUT)
	  loli = lo_time_cut;
	else
	  loli = time_cut_values[0];
	if(OVERRIDE_HIGH_TIME_CUT)
	  hili = hi_time_cut;
	else
	  hili = time_cut_values[1];
	TimeCenterCut = hili - (hili-loli)/2.;
	TimeWidthCut = (hili-loli)/2.;
	//jout<<"TOF Timing Cuts for PRUNING: "<<TimeCenterCut<<" +/- "<<TimeWidthCut<<endl;
      }

    }

    // load scale factors
    map<string,double> scale_factors;
	string locTOFDigiScalesTable = tofGeom.Get_CCDB_DirectoryName() + "/digi_scales";
    if(calibration->Get(locTOFDigiScalesTable.c_str(), scale_factors))
      jout << "Error loading " << locTOFDigiScalesTable << " !" << jendl;
    if( scale_factors.find("TOF_ADC_ASCALE") != scale_factors.end() ) {
      ;	//a_scale = scale_factors["TOF_ADC_ASCALE"];
    } else {
      jerr << "Unable to get TOF_ADC_ASCALE from " << locTOFDigiScalesTable << " !" << endl;
    }
    if( scale_factors.find("TOF_ADC_TSCALE") != scale_factors.end() ) {
      ; //t_scale = scale_factors["TOF_ADC_TSCALE"];
    } else {
      jerr << "Unable to get TOF_ADC_TSCALE from " << locTOFDigiScalesTable << " !" << endl;
    }
    
    // load base time offset
    map<string,double> base_time_offset;
	string locTOFBaseTimeOffsetTable = tofGeom.Get_CCDB_DirectoryName() + "/base_time_offset";
    if (calibration->Get(locTOFBaseTimeOffsetTable.c_str(),base_time_offset))
      jout << "Error loading " << locTOFBaseTimeOffsetTable << " !" << jendl;
    if (base_time_offset.find("TOF_BASE_TIME_OFFSET") != base_time_offset.end())
      t_base = base_time_offset["TOF_BASE_TIME_OFFSET"];
    else
      jerr << "Unable to get TOF_BASE_TIME_OFFSET from "<<locTOFBaseTimeOffsetTable<<" !" << endl;
    
    if (base_time_offset.find("TOF_TDC_BASE_TIME_OFFSET") != base_time_offset.end())
      t_base_tdc = base_time_offset["TOF_TDC_BASE_TIME_OFFSET"];
    else
      jerr << "Unable to get TOF_TDC_BASE_TIME_OFFSET from "<<locTOFBaseTimeOffsetTable<<" !" << endl;
    
    // load constant tables
    string locTOFPedestalsTable = tofGeom.Get_CCDB_DirectoryName() + "/pedestals";
    if(calibration->Get(locTOFPedestalsTable.c_str(), raw_adc_pedestals))
      jout << "Error loading " << locTOFPedestalsTable << " !" << jendl;
    string locTOFGainsTable = tofGeom.Get_CCDB_DirectoryName() + "/gains";
    if(calibration->Get(locTOFGainsTable.c_str(), raw_adc_gains))
      jout << "Error loading " << locTOFGainsTable << " !" << jendl;
    string locTOFADCTimeOffetsTable = tofGeom.Get_CCDB_DirectoryName() + "/adc_timing_offsets";
    if(calibration->Get(locTOFADCTimeOffetsTable.c_str(), raw_adc_offsets))
      jout << "Error loading " << locTOFADCTimeOffetsTable << " !" << jendl;
    
    // check which walk correction to use:
    string locTOFWalkCorrectionType = tofGeom.Get_CCDB_DirectoryName() + "/walkcorr_type";
    vector<int> walkcorrtype;
    if(calibration->Get(locTOFWalkCorrectionType.c_str(), walkcorrtype)) {
      jout<<"\033[1;31m";  // red text";
      jout<< "Error loading "<<locTOFWalkCorrectionType<<" !\033[0m" << endl;
      return; // (jerror_t)101;
    }
    
    switch ((int)walkcorrtype[0]) {
      
    case 1:  // walk corrections based on Integral values
      {
	if(print_messages) jout<<"TOF: USE WALK CORRECTION TYPE 1"<<endl;
	string locTOFTimewalkTable = tofGeom.Get_CCDB_DirectoryName() + "/timewalk_parms";
	if(calibration->Get(locTOFTimewalkTable.c_str(), timewalk_parameters)){
	  jout << "Error loading "<<locTOFTimewalkTable<<" !" << jendl;
	}
	string locTOFChanOffsetTable1 = tofGeom.Get_CCDB_DirectoryName() + "/timing_offsets";
	if(calibration->Get(locTOFChanOffsetTable1.c_str(), raw_tdc_offsets)){
	  jout << "Error loading "<<locTOFChanOffsetTable1<<" !" << jendl;
	}
      }
      break;

    case 2:  // walk correction based on peak Amplitudes single function 
      {
	if(print_messages) jout<<"TOF: USE WALK CORRECTION TYPE 2"<<endl;
	USE_AMP_4WALKCORR = 1;
	string locTOFTimewalkAMPTable = tofGeom.Get_CCDB_DirectoryName() + "/timewalk_parms_AMP";
	if(calibration->Get(locTOFTimewalkAMPTable.c_str(), timewalk_parameters_AMP)){
	  jout << "Error loading "<<locTOFTimewalkAMPTable<<" !" << jendl;
	}
	string locTOFChanOffsetTable2 = tofGeom.Get_CCDB_DirectoryName() + "/timing_offsets";
	if(calibration->Get(locTOFChanOffsetTable2.c_str(), raw_tdc_offsets)){
	  jout << "Error loading "<<locTOFChanOffsetTable2<<" !" << jendl;
	}
      }
      break;
      
    case 3:
      {
	if(print_messages) jout<<"TOF: USE WALK CORRECTION TYPE 3"<<endl;
	USE_NEWAMP_4WALKCORR = 1;
	string locTOFChanOffsetNEWAMPTable = tofGeom.Get_CCDB_DirectoryName() + "/timing_offsets_NEWAMP";
	if(calibration->Get(locTOFChanOffsetNEWAMPTable.c_str(), raw_tdc_offsets)) {
	  jout<< "Error loading "<<locTOFChanOffsetNEWAMPTable<<" !" << jendl;
	}
	string locTOFTimewalkNEWAMPTable = tofGeom.Get_CCDB_DirectoryName() + "/timewalk_parms_NEWAMP";
	if(calibration->Get(locTOFTimewalkNEWAMPTable.c_str(), timewalk_parameters_NEWAMP)){
	  jout << "Error loading "<<locTOFTimewalkNEWAMPTable<<" !" << jendl;
	}
      }
      break;

    case 4:
      {
	if(print_messages) jout<<"TOF: USE WALK CORRECTION TYPE 4"<<endl;
	USE_NEW_WALK_NEW = 1;
	string locTOFTimewalkNEWTable = tofGeom.Get_CCDB_DirectoryName() + "/timewalk_parms_5PAR";
	if(calibration->Get(locTOFTimewalkNEWTable.c_str(), timewalk_parameters_5PAR)){
	  jout << "Error loading "<<locTOFTimewalkNEWTable<<" !" << jendl;
	}
	string locTOFChanOffsetTable = tofGeom.Get_CCDB_DirectoryName() + "/timing_offsets_5PAR";
	if(calibration->Get(locTOFChanOffsetTable.c_str(), raw_tdc_offsets)){
	  jout << "Error loading "<<locTOFChanOffsetTable<<" !" << jendl;
	}
      }
      break;

    }
    
    
    FillCalibTable(adc_pedestals, raw_adc_pedestals, tofGeom);
    FillCalibTable(adc_gains, raw_adc_gains, tofGeom);
    FillCalibTable(adc_time_offsets, raw_adc_offsets, tofGeom);
    FillCalibTable(tdc_time_offsets, raw_tdc_offsets, tofGeom);
    
    
    string locTOFADC2ETable = tofGeom.Get_CCDB_DirectoryName() + "/adc2E";
    if(calibration->Get(locTOFADC2ETable.c_str(), raw_adc2E))
      jout << "Error loading " << locTOFADC2ETable << " !" << jendl;
    
    // make sure we have one entry per channel
    adc2E.resize(TOF_NUM_PLANES*TOF_NUM_BARS*2);
    for (unsigned int n=0; n<raw_adc2E.size(); n++){
      adc2E[n] = raw_adc2E[n];
    }
    
    /*
      CheckCalibTable(adc_pedestals,"/TOF/pedestals");
      CheckCalibTable(adc_gains,"/TOF/gains");
      CheckCalibTable(adc_time_offsets,"/TOF/adc_timing_offsets");
      CheckCalibTable(tdc_time_offsets,"/TOF/timing_offsets");
    */

}

//------------------
// Process
//------------------
void DTOFHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  /// Generate DTOFHit object for each DTOFDigiHit object.
  /// This is where the first set of calibration constants
  /// is applied to convert from digitzed units into natural
  /// units.
  /// 
  /// In a second step TDC information from DTOFTDCDigiHit objects are
  /// added to the hits if the TDC matches the ADC in time within a given timing window. 
  //  In this case the TDC time can be corrected for detector signal walk. All times
  /// are converted to nano seconds. 
  /// The ADC energy deposition in paddle is converted to GeV assuming the hit is
  /// at the center of the paddle. This will be corrected later using the hit position
  /// and the attenuation length in the DTOFHIT_Paddle factory.
  ///
  /// Note that this code does NOT get called for simulated
  /// data in HDDM format. The HDDM event source will copy
  /// the precalibrated values directly into the mData vector.

  auto eventnumber = event->GetEventNumber();

  const DTTabUtilities* locTTabUtilities = NULL;
  event->GetSingle(locTTabUtilities);
  
  // First, make hits out of all fADC250 hits
  vector<const DTOFDigiHit*> digihits;
  event->Get(digihits);
  for(unsigned int i=0; i<digihits.size(); i++){
    const DTOFDigiHit *digihit = digihits[i];
    
    // Error checking for pre-Fall 2016 firmware
    if(digihit->datasource == 1) {
      // There is a slight difference between Mode 7 and 8 data
      // The following condition signals an error state in the flash algorithm
      // Do not make hits out of these
      const Df250PulsePedestal* PPobj = NULL;
      digihit->GetSingle(PPobj);
      if (PPobj != NULL) {
	if (PPobj->pedestal == 0 || PPobj->pulse_peak == 0) continue;
      } else 
	continue;
      
      //if (digihit->pulse_time == 0) continue; // Should already be caught
    }
    
    if(CHECK_FADC_ERRORS && !locTTabUtilities->CheckFADC250_NoErrors(digihit->QF)){ 

      if (TOF_DEBUG){
	vector <const Df250PulseData *> pulses;
	digihit->Get(pulses);
	const Df250PulseData *p = pulses[0];
	
	cout<<"1: "<<eventnumber<<" P/B/E  "<<digihit->plane<<"/"<<digihit->bar<<"/"<<digihit->end
	    <<" :::>  I/Ped/P/T   "<<digihit->pulse_integral<<"/"<<digihit->pedestal<<"/"<<digihit->pulse_peak<<"/"<<digihit->pulse_time
	    <<" QF: 0x"<<hex<<digihit->QF<<dec
	    <<"       roc/slot/chan "<<p->rocid<<"/"<<p->slot<<"/"<<p->channel
	    << endl;
      }

      //continue;

    }
    // Initialize pedestal to one found in CCDB, but override it
    // with one found in event if is available (?)
    // For now, only keep events with a correct pedestal
    double pedestal = GetConstant(adc_pedestals, digihit); // get mean pedestal from DB in case we need it
    double nsamples_integral = digihit->nsamples_integral;
    double nsamples_pedestal = digihit->nsamples_pedestal;
    
    // nsamples_pedestal should always be positive for valid data - err on the side of caution for now
    if(nsamples_pedestal == 0) {
      jerr << "DTOFDigiHit with nsamples_pedestal == 0 !   Event = " << eventnumber << endl;
      continue;
    }
    
    double pedestal4Amp = pedestal;
    int AlreadyDone = 0;
    if( (digihit->pedestal>0) && locTTabUtilities->CheckFADC250_PedestalOK(digihit->QF) ) {
      pedestal = digihit->pedestal * (double)(nsamples_integral)/(double)(nsamples_pedestal); // overwrite pedestal
      pedestal4Amp = digihit->pedestal;
    } else {

      if (TOF_DEBUG){
	vector <const Df250PulseData *> pulses;
	digihit->Get(pulses);
	const Df250PulseData *p = pulses[0];
	
	cout<<"2: "<<eventnumber<<" P/B/E  "<<digihit->plane<<"/"<<digihit->bar<<"/"<<digihit->end
	    <<" :::>   I/Ped/P/T    "<<digihit->pulse_integral<<"/"<<digihit->pedestal<<"/"<<digihit->pulse_peak<<"/"<<digihit->pulse_time
	    <<" QF: 0x"<<hex<<digihit->QF<<dec
	    <<"       roc/slot/chan  "<<p->rocid<<"/"<<p->slot<<"/"<<p->channel
 	    << endl;
	
      }
      
      pedestal *= (double)(nsamples_integral); 
      pedestal4Amp *= (double)nsamples_pedestal;
      AlreadyDone = 1;
      //continue;
    }

    if ((digihit->pulse_peak == 0) && (!AlreadyDone)){
      pedestal = pedestal4Amp * (double)(nsamples_integral);
      pedestal4Amp *=  (double)nsamples_pedestal;
    }
    
    // Apply calibration constants here
    double A = (double)digihit->pulse_integral;
    double T = (double)digihit->pulse_time;
    T =  t_scale * T - GetConstant(adc_time_offsets, digihit) + t_base;
    double dA = A - pedestal;
    
    if (dA<0) {
      
      if (TOF_DEBUG){
	
	vector <const Df250PulseData *> pulses;
	digihit->Get(pulses);
	const Df250PulseData *p = pulses[0];
	
	cout<<"3: "<<eventnumber<<"  "<<dA<<"   "<<digihit->plane<<"   "<<digihit->bar<<"   "<<digihit->end
	    <<" :::>  "<<digihit->pulse_integral<<"  "<<digihit->pedestal<<"  "<<digihit->pulse_peak<<"   "<<digihit->pulse_time
	    <<"       roc/slot/chan "<<p->rocid<<"/"<<p->slot<<"/"<<p->channel
	    << endl;

      }
      // ok if Integral is below zero this is a good hint that we can not use this hit!
      continue; 
    }
    // apply Time cut to prune out of time hits
    if (TMath::Abs(T-TimeCenterCut)> TimeWidthCut ) continue;
    
    DTOFHit *hit = new DTOFHit;
    hit->plane = digihit->plane;
    hit->bar   = digihit->bar;
    hit->end   = digihit->end;
    hit->dE=dA;  // this will be scaled to energy units later
    hit->Amp = (float)digihit->pulse_peak - pedestal4Amp/(float)nsamples_pedestal;

    if (hit->Amp<1){ // this happens if pulse_peak is reported as zero, resort to use scaled Integral value
      hit->Amp = dA*0.163;
    }
    
    if(COSMIC_DATA)
      hit->dE = (A - 55*pedestal); // value of 55 is taken from (NSB,NSA)=(10,45) in the confg file
    
    hit->t_TDC=numeric_limits<double>::quiet_NaN();
    hit->t_fADC=T;
    hit->t = hit->t_fADC;  // set initial time to the ADC time, in case there's no matching TDC hit
    
    hit->has_fADC=true;
    hit->has_TDC=false;
    
    /*
      cout << "TOF ADC hit =  (" << hit->plane << "," << hit->bar << "," << hit->end << ")  " 
      << t_scale << " " << T << "  "
      << GetConstant(adc_time_offsets, digihit) << " " 
      << t_scale*GetConstant(adc_time_offsets, digihit) << " " << hit->t << endl;
    */
    
    hit->AddAssociatedObject(digihit);
    
    Insert(hit);
  }
  
  //Get the TDC hits
  vector<const DTOFTDCDigiHit*> tdcdigihits;
  event->Get(tdcdigihits);
  
  // Next, loop over TDC hits, matching them to the
  // existing fADC hits where possible and updating
  // their time information. If no match is found, then
  // create a new hit with just the TDC info.
  for(unsigned int i=0; i<tdcdigihits.size(); i++)
    {
      const DTOFTDCDigiHit *digihit = tdcdigihits[i];
      
      // Apply calibration constants here
      double T = locTTabUtilities->Convert_DigiTimeToNs_CAEN1290TDC(digihit);
      T += t_base_tdc - GetConstant(tdc_time_offsets, digihit) + tdc_adc_time_offset;
      
      // do not consider Time hits away from coincidence peak Note: This cut should be wide for uncalibrated data!!!!!
      if (TMath::Abs(T-TimeCenterCut)> TimeWidthCut ) continue;

      /*
	cout << "TOF TDC hit =  (" << digihit->plane << "," << digihit->bar << "," << digihit->end << ")  " 
	<< T << "  " << GetConstant(tdc_time_offsets, digihit) << endl;
      */
      
      // Look for existing hits to see if there is a match
      // or create new one if there is no match
      DTOFHit *hit = FindMatch(digihit->plane, digihit->bar, digihit->end, T);
      //DTOFHit *hit = FindMatch(digihit->plane, hit->bar, hit->end, T);
      if(!hit){
	continue; // Do not use unmatched TDC hits
	/*
	hit = new DTOFHit;
	hit->plane = digihit->plane;
	hit->bar   = digihit->bar;
	hit->end   = digihit->end;
	hit->dE = 0.0;
	hit->Amp = 0.0;
	hit->t_fADC=numeric_limits<double>::quiet_NaN();
	hit->has_fADC=false;
	
	Insert(hit);
	*/
      } else if (hit->has_TDC) { // this tof ADC hit has already a matching TDC, make new tof ADC hit
	DTOFHit *newhit = new DTOFHit;
	newhit->plane = hit->plane;
	newhit->bar = hit->bar;
	newhit->end = hit->end;
	newhit->dE = hit->dE;
	newhit->Amp = hit->Amp;
	newhit->t_fADC = hit->t_fADC;
	newhit->has_fADC = hit->has_fADC;
	newhit->t_TDC=numeric_limits<double>::quiet_NaN();
	newhit->t = hit->t_fADC;  // set initial time to the ADC time, in case there's no matching TDC hit	
	newhit->has_TDC=false;
	newhit->AddAssociatedObject(digihit);
	Insert(newhit);
	hit = newhit;
      }
      hit->has_TDC=true;
      hit->t_TDC=T;
      
      if (hit->dE>0.){

	// time walk correction
	// Note at this point the dE value is still in ADC units
	double tcorr = 0.;
	if (USE_AMP_4WALKCORR) {
	  // use amplitude instead of integral
	  tcorr = CalcWalkCorrAmplitude(hit);

	} else if (USE_NEW_4WALKCORR) {
	  // new functional form with 4 parameter but still using integral
	  tcorr = CalcWalkCorrNEW(hit);

	} else if (USE_NEWAMP_4WALKCORR) {
	  // new functional form with 2 functions and 4 parameter using amplitude
 	  tcorr = CalcWalkCorrNEWAMP(hit);

	} else if (USE_NEW_WALK_NEW){
	  // new functional form with 5 parameters using singal amplitude
	  tcorr = CalcWalkCorrNEW5PAR(hit);

	} else {
	  // use integral
	  tcorr = CalcWalkCorrIntegral(hit);

	}
	
	T -= tcorr;
      }
      hit->t=T;
      
      hit->AddAssociatedObject(digihit);
    }
  
  // Apply calibration constants to convert pulse integrals to energy units
  for (unsigned int i=0;i<mData.size();i++){
    int id=2*TOF_NUM_BARS*mData[i]->plane + TOF_NUM_BARS*mData[i]->end + mData[i]->bar-1;
    mData[i]->dE *= adc2E[id];
    //cout<<id<<"   "<< adc2E[id]<<"      "<<mData[i]->dE<<endl;
  }
}

//------------------
// FindMatch
//------------------
DTOFHit* DTOFHit_factory::FindMatch(int plane, int bar, int end, double T)
{
  /// This method tries to finds a corresponding match of an ADC hit to a
  /// TDC hit object from DTOFTDCDigiHit with the requirement that the hit time T is within a predefined
  /// window of size DELTA_T_ADC_TDC_MAX. This value can be over written on the
  /// command line at start up by -PTOF:DELTA_T_ADC_TDC_MAX=xx where xx is the value
  /// in ns. The default time window is 20ns. 

    DTOFHit* best_match = NULL;

    // Loop over existing hits (from fADC) and look for a match
    // in both the sector and the time.

    for(unsigned int i=0; i<mData.size(); i++){
        DTOFHit *hit = mData[i];

        if(!isfinite(hit->t_fADC)) continue; // only match to fADC hits, not bachelor TDC hits
        if(hit->plane != plane) continue;
        if(hit->bar != bar) continue;
        if(hit->end != end) continue;

        //double delta_T = fabs(hit->t - T);
        double delta_T = fabs(T - hit->t);
        if(delta_T > DELTA_T_ADC_TDC_MAX) continue;

        // if there are multiple hits, pick the one that is closest in time
        if(best_match != NULL) {
            if(delta_T < fabs(best_match->t - T))
                best_match = hit;
        } else {
            best_match = hit;
        }

    }

    return best_match;
}

//------------------
// EndRun
//------------------
void DTOFHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DTOFHit_factory::Finish()
{
}


//------------------
// FillCalibTable
//------------------
void DTOFHit_factory::FillCalibTable(tof_digi_constants_t &table, vector<double> &raw_table, 
        const DTOFGeometry &tofGeom)
{
    char str[256];
    int channel = 0;

    // reset the table before filling it
    table.clear();

    for(int plane=0; plane<tofGeom.Get_NPlanes(); plane++) {
        int plane_index=2*tofGeom.Get_NBars()*plane;
        table.push_back( vector< pair<double,double> >(tofGeom.Get_NBars()) );
        for(int bar=0; bar<tofGeom.Get_NBars(); bar++) {
            table[plane][bar] 
                = pair<double,double>(raw_table[plane_index+bar],
                        raw_table[plane_index+tofGeom.Get_NBars()+bar]);
            channel+=2;	      
        }
    }

    // check to make sure that we loaded enough channels
    if(channel != TOF_MAX_CHANNELS) { 
        sprintf(str, "Wrong number of channels for TOF table! channel=%d (should be %d)", 
                channel, TOF_MAX_CHANNELS);
        cerr << str << endl;
        throw JException(str);
    }
}


//------------------------------------
// GetConstant
//   Allow a few different interfaces
//   NOTE: LoadGeometry() must be called before calling these functions
//
//   TOF Geometry as defined in the Translation Table:
//     plane = 0-1
//     bar   = 1-44
//     end   = 0-1
//   Note the different counting schemes used
//------------------------------------
const double DTOFHit_factory::GetConstant( const tof_digi_constants_t &the_table, 
        const int in_plane, const int in_bar, const int in_end) const
{
    char str[256];

    if( (in_plane < 0) || (in_plane > TOF_NUM_PLANES)) {
        sprintf(str, "Bad module # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_plane, TOF_NUM_PLANES);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_bar <= 0) || (in_bar > TOF_NUM_BARS)) {
        sprintf(str, "Bad layer # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_bar, TOF_NUM_BARS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_end != 0) && (in_end != 1) ) {
        sprintf(str, "Bad end # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 0-1", in_end);
        cerr << str << endl;
        throw JException(str);
    }

    // we have two ends, indexed as 0/1 
    // could be north/south or up/down depending on the bar orientation
    if(in_end == 0) {
        return the_table[in_plane][in_bar].first;
    } else {
        return the_table[in_plane][in_bar].second;
    }
}

const double DTOFHit_factory::GetConstant( const tof_digi_constants_t &the_table, 
        const DTOFHit *in_hit) const
{
    char str[256];

    if( (in_hit->plane < 0) || (in_hit->plane > TOF_NUM_PLANES)) {
        sprintf(str, "Bad module # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_hit->plane, TOF_NUM_PLANES);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_hit->bar <= 0) || (in_hit->bar > TOF_NUM_BARS)) {
        sprintf(str, "Bad layer # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_hit->bar, TOF_NUM_BARS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_hit->end != 0) && (in_hit->end != 1) ) {
        sprintf(str, "Bad end # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 0-1", in_hit->end);
        cerr << str << endl;
        throw JException(str);
    }

    // we have two ends, indexed as 0/1 
    // could be north/south or up/down depending on the bar orientation
    if(in_hit->end == 0) {
        return the_table[in_hit->plane][in_hit->bar-1].first;
    } else {
        return the_table[in_hit->plane][in_hit->bar-1].second;
    }
}

const double DTOFHit_factory::GetConstant( const tof_digi_constants_t &the_table, 
        const DTOFDigiHit *in_digihit) const
{
    char str[256];

    if( (in_digihit->plane < 0) || (in_digihit->plane > TOF_NUM_PLANES)) {
        sprintf(str, "Bad module # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_digihit->plane, TOF_NUM_PLANES);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_digihit->bar <= 0) || (in_digihit->bar > TOF_NUM_BARS)) {
        sprintf(str, "Bad layer # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_digihit->bar, TOF_NUM_BARS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_digihit->end != 0) && (in_digihit->end != 1) ) {
        sprintf(str, "Bad end # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 0-1", in_digihit->end);
        cerr << str << endl;
        throw JException(str);
    }

    // we have two ends, indexed as 0/1 
    // could be north/south or up/down depending on the bar orientation
    if(in_digihit->end == 0) {
        return the_table[in_digihit->plane][in_digihit->bar-1].first;
    } else {
        return the_table[in_digihit->plane][in_digihit->bar-1].second;
    }
}

const double DTOFHit_factory::GetConstant( const tof_digi_constants_t &the_table, 
        const DTOFTDCDigiHit *in_digihit) const
{
    char str[256];

    if( (in_digihit->plane < 0) || (in_digihit->plane > TOF_NUM_PLANES)) {
        sprintf(str, "Bad module # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_digihit->plane, TOF_NUM_PLANES);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_digihit->bar <= 0) || (in_digihit->bar > TOF_NUM_BARS)) {
        sprintf(str, "Bad layer # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_digihit->bar, TOF_NUM_BARS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_digihit->end != 0) && (in_digihit->end != 1) ) {
        sprintf(str, "Bad end # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 0-1", in_digihit->end);
        cerr << str << endl;
        throw JException(str);
    }

    // we have two ends, indexed as 0/1 
    // could be north/south or up/down depending on the bar orientation
    if(in_digihit->end == 0) {
        return the_table[in_digihit->plane][in_digihit->bar-1].first;
    } else {
        return the_table[in_digihit->plane][in_digihit->bar-1].second;
    }
}

/*
   const double DTOFHit_factory::GetConstant( const tof_digi_constants_t &the_table,
   const DTranslationTable *ttab,
   const int in_rocid, const int in_slot, const int in_channel) const {

   char str[256];

   DTranslationTable::csc_t daq_index = { in_rocid, in_slot, in_channel };
   DTranslationTable::DChannelInfo channel_info = ttab->GetDetectorIndex(daq_index);

   if( (channel_info.tof.plane <= 0) 
   || (channel_info.tof.plane > static_cast<unsigned int>(TOF_NUM_PLANES))) {
   sprintf(str, "Bad plane # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", channel_info.tof.plane, TOF_NUM_PLANES);
   cerr << str << endl;
   throw JException(str);
   }
   if( (channel_info.tof.bar <= 0) 
   || (channel_info.tof.bar > static_cast<unsigned int>(TOF_NUM_BARS))) {
   sprintf(str, "Bad bar # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", channel_info.tof.bar, TOF_NUM_BARS);
   cerr << str << endl;
   throw JException(str);
   }
   if( (channel_info.tof.end != 0) && (channel_info.tof.end != 1) ) {
   sprintf(str, "Bad end # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 0-1", channel_info.tof.end);
   cerr << str << endl;
   throw JException(str);
   }

   int the_cell = DTOFGeometry::cellId(channel_info.tof.module,
   channel_info.tof.layer,
   channel_info.tof.sector);

   if(channel_info.tof.end == DTOFGeometry::kUpstream) {
// handle the upstream end
return the_table.at(the_cell).first;
} else {
// handle the downstream end
return the_table.at(the_cell).second;
}
}
*/
double DTOFHit_factory::CalcWalkCorrIntegral(DTOFHit* hit){
  int id=2*TOF_NUM_BARS*hit->plane+TOF_NUM_BARS*hit->end+hit->bar-1;
  double A=hit->dE;
  double C0=timewalk_parameters[id][1];
  double C1=timewalk_parameters[id][1];
  double C2=timewalk_parameters[id][2];
  double A0=timewalk_parameters[id][3];

  double a1 = C0 + C1*pow(A,C2);
  double a2 = C0 + C1*pow(A0,C2);

  float corr = a1 - a2;

  //cout<<id<<"   "<<A<<"    "<<a1<<"   "<<a2<<"    "<<corr<<endl;

  return corr;


}


double DTOFHit_factory::CalcWalkCorrAmplitude(DTOFHit* hit){

  int id=2*TOF_NUM_BARS*hit->plane+TOF_NUM_BARS*hit->end+hit->bar-1;
  double A  = hit->Amp;
  double C0 = timewalk_parameters_AMP[id][0];
  double C1 = timewalk_parameters_AMP[id][1];
  double C2 = timewalk_parameters_AMP[id][2];
  double C3 = timewalk_parameters_AMP[id][3];

  double hookx = timewalk_parameters_AMP[id][4]; 
  double refx = timewalk_parameters_AMP[id][5];
  double val_at_ref = C0 + C1*pow(refx,C2); 
  double val_at_hook = C0 + C1*pow(hookx,C2); 
  double slope = (val_at_hook - C3)/hookx;
  if (refx>hookx){
    val_at_ref  = slope * refx + C3; 
  }
  double val_at_A = C0 + C1*pow(A,C2);
  if (A>hookx){
    val_at_A = slope * A + C3; 
  }

  float corr = val_at_A - val_at_ref;

  //cout<<id<<"   "<<val_at_A<<"   "<<val_at_ref<<"    "<<corr<<endl;

  return corr;

}


double DTOFHit_factory::CalcWalkCorrNEW(DTOFHit* hit){
 
  int id=2*TOF_NUM_BARS*hit->plane+TOF_NUM_BARS*hit->end+hit->bar-1;
  double ADC=hit->dE;

  if (ADC<1.){
    return 0;
  }
  
  double A = timewalk_parameters_NEW[id][0];
  double B = timewalk_parameters_NEW[id][1];
  double C = timewalk_parameters_NEW[id][2];
  double D = timewalk_parameters_NEW[id][3];
  double ADCREF = timewalk_parameters_NEW[id][4];

  if (ADC>20000.){
    ADC = 20000.;
  }
  double a1 = A + B*pow(ADC,-0.5) + C*pow(ADC,-0.33) + D*pow(ADC,-0.2);
  double a2 = A + B*pow(ADCREF,-0.5) + C*pow(ADCREF,-0.33) + D*pow(ADCREF,-0.2);


  float corr = a1 - a2;

  //cout<<id<<"   "<<a1<<"   "<<a2<<"    "<<corr<<endl;

  return corr;

}

double DTOFHit_factory::CalcWalkCorrNEWAMP(DTOFHit* hit){
 
  int id=2*TOF_NUM_BARS*hit->plane+TOF_NUM_BARS*hit->end+hit->bar-1;
  double ADC=hit->Amp;
  if (ADC<50.){
    return 0;
  }
  double loc = timewalk_parameters_NEWAMP[id][8];
  int offset = 0;
  if (ADC>loc){
    offset = 4;
  }
  double A = timewalk_parameters_NEWAMP[id][0+offset];
  double B = timewalk_parameters_NEWAMP[id][1+offset];
  double C = timewalk_parameters_NEWAMP[id][2+offset];
  double D = timewalk_parameters_NEWAMP[id][3+offset];

  double ADCREF = timewalk_parameters_NEWAMP[id][9];
  double A2 = timewalk_parameters_NEWAMP[id][4];
  double B2 = timewalk_parameters_NEWAMP[id][5];
  double C2 = timewalk_parameters_NEWAMP[id][6];
  double D2 = timewalk_parameters_NEWAMP[id][7];

  double a1 = A + B*pow(ADC,-0.5) + C*pow(ADC,-0.33) + D*pow(ADC,-0.2);
  double a2 = A2 + B2*pow(ADCREF,-0.5) + C2*pow(ADCREF,-0.33) + D2*pow(ADCREF,-0.2);

  if (ADC>4095){
    a1 += 0.6; // overflow hits are off by about 0.6ns to the regular curve.
  }

  float corr = a1 - a2;

  //cout<<id<<"     "<<ADC<<"      "<<a1<<"   "<<a2<<"    "<<corr<<endl;
  
  return corr;

}

double DTOFHit_factory::CalcWalkCorrNEW5PAR(DTOFHit* hit){
 
  int id=2*TOF_NUM_BARS*hit->plane+TOF_NUM_BARS*hit->end+hit->bar-1;
  double ADC=hit->Amp;
  if (ADC<50.){
    return 0;
  }

  if (ADC>4090){
    ADC = 4090;
  }
  double A = timewalk_parameters_5PAR[id][0];
  double B = timewalk_parameters_5PAR[id][1];
  double C = timewalk_parameters_5PAR[id][2];
  double D = timewalk_parameters_5PAR[id][3];
  double E = timewalk_parameters_5PAR[id][4];

  double ADCREF = timewalk_parameters_5PAR[id][9];

  double a1 = A + B/ADC + C/ADC/ADC + D/ADC/ADC/ADC/ADC + E/sqrt(ADC);
  double a2 = A + B/ADCREF + C/ADCREF/ADCREF + D/ADCREF/ADCREF/ADCREF/ADCREF + E/sqrt(ADCREF);

  float corr = a1 - a2;

  //cout<<id<<"     "<<ADC<<"      "<<a1<<"   "<<a2<<"    "<<corr<<endl;
  
  return corr;

}

