// $Id$
//
/*! **File**: DSCHit_factory.cc
 *+ Created: Tue Aug  6 12:53:32 EDT 2013
 *+ Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
 *+ Purpose: Implementation of DSCHit_factory methods ::init(), ::brun(), ::evnt()
*/
/// \addtogroup SCDetector

/*! \file DSCHit_factory.cc
 * Implementation of start counter hit factory methods ::init(), ::brun(), ::evnt()
 * Generates DSCHit objects of start counter hits with energy (GeV) and time (ns)
 */


#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
using namespace std;

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include <DANA/DEvent.h>
#include <DANA/DGeometryManager.h>
#include <HDGEOMETRY/DGeometry.h>
#include <START_COUNTER/DSCDigiHit.h>
#include <START_COUNTER/DSCTDCDigiHit.h>
#include <TTAB/DTTabUtilities.h>
#include <DAQ/Df250PulsePedestal.h>
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250Config.h>

#include "DSCHit_factory.h"



bool DSCHit_fadc_cmp(const DSCDigiHit *a,const DSCDigiHit *b)
{
  if (a->sector==b->sector) return (a->pulse_time<b->pulse_time);
  return (a->sector<b->sector);
}

bool DSCHit_tdc_cmp(const DSCTDCDigiHit *a,const DSCTDCDigiHit *b)
{
  if (a->sector==b->sector) return (a->time<b->time);
  return (a->sector<b->sector);
}

//------------------
// Init
//------------------
/// \fn jerror_t DSCHit_factory::Init(void)
/// initialize timing window (default = 20ns) for hit matching between ADC and TDC hits
/// this parameter can be modified on the command line: -PSC:DELTA_T_ADC_TDC_MAX=xx
/// same for the timing window for hits in general: -PSC:HIT_TIME_WINDOW=xx
/// the default for this parameter is 60ns
/// A default threshold on the ADC value is set at 120 ADC counts and can also be modfied
/// on the command line with -PSC:ADC_THRESHOLD=xxx
/// the time walk correction is by default applied but can be turned off on the 
/// command line with : -PSC:USE_TIMEWALK_CORRECTION=0
/// by default it is required that a DSCHit has to have both ADC and TDC matched data. This
/// requirement can be turned off with -PSC:REQUIRE_ADC_TDC_MATCH=false
/// by default ADC errors are checked and such data is ignored. This can be
/// turned off on the command line like: -PSC:CHECK_FADC_ERRORS=false
void DSCHit_factory::Init()
{
  auto app = GetApplication();
  DELTA_T_ADC_TDC_MAX = 20.0; // ns
  //DELTA_T_ADC_TDC_MAX = 50.0; // ns
  //DELTA_T_ADC_TDC_MAX = 3600.0; // ns
  app->SetDefaultParameter("SC:DELTA_T_ADC_TDC_MAX", DELTA_T_ADC_TDC_MAX,
			   "Maximum difference in ns between a (calibrated) fADC time and"
			   " F1TDC time for them to be matched in a single hit");
  
  HIT_TIME_WINDOW = 60.0; //ns
  app->SetDefaultParameter("SC:HIT_TIME_WINDOW", HIT_TIME_WINDOW,
			   "Time window of trigger corrected TDC time in which a hit in"
			   " in the TDC will match to a hit in the fADC to form an ST hit");
  
  //ADC_THRESHOLD = 200.; // adc counts (= 50 mV threshold)
  ADC_THRESHOLD = 120.; // adc counts (= 10 Mv threshold)
  app->SetDefaultParameter("SC:ADC_THRESHOLD", ADC_THRESHOLD,
			   "Software pulse integral threshold");
  
  USE_TIMEWALK_CORRECTION = 1.;
  app->SetDefaultParameter("SC:USE_TIMEWALK_CORRECTION", USE_TIMEWALK_CORRECTION,
			   "Flag to decide if timewalk corrections should be applied.");
  
  REQUIRE_ADC_TDC_MATCH=true;
  app->SetDefaultParameter("SC:REQUIRE_ADC_TDC_MATCH",
			      REQUIRE_ADC_TDC_MATCH,
			   "Flag to decide if a match between adc and tdc hits is required.");
  
  /// set the base conversion scales
  a_scale    = 0.0001; 
  t_scale    = 0.0625;   // 62.5 ps/count
  t_base     = 0.;       // ns
  t_tdc_base = 0.;
  
  CHECK_FADC_ERRORS = true;
  app->SetDefaultParameter("SC:CHECK_FADC_ERRORS", CHECK_FADC_ERRORS, "Set to 1 to reject hits with fADC250 errors, ser to 0 to keep these hits");
}

//------------------
// BeginRun
//------------------
/// \fn void DSCHit_factory::BeginRun(const std::shared_ptr<const JEvent> &event)
/// \param eventLoop
/// \param runnumber
/// this function is called every time the run number changes and calibration parameters
/// will be loaded in from CCDB like timing offsets and walk corrections

void DSCHit_factory::BeginRun(const std::shared_ptr<const JEvent> &event)
{

  auto runnumber = event->GetRunNumber();
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
  
  /// Load geometry - just need the number of sectors
  DGeometry* locGeometry = DEvent::GetDGeometry(event);
  
  // Get start counter geometry
  vector<vector<DVector3> >sc_norm; 
  vector<vector<DVector3> >sc_pos;
  MAX_SECTORS=0;
  if (locGeometry->GetStartCounterGeom(sc_pos, sc_norm))
    MAX_SECTORS = sc_pos.size();
  else
    jerr << "Cannot load Start Counter geometry information!" << endl;
  
  /// Read in calibration constants
  if(print_messages) jout << "In DSCHit_factory, loading constants..." << endl;
  
  // load scale factors
  map<string,double> scale_factors;
  // a_scale (SC_ADC_SCALE)
  if (DEvent::GetCalib(event,"/START_COUNTER/digi_scales", scale_factors))
    jout << "Error loading /START_COUNTER/digi_scales !" << endl;
  if (scale_factors.find("SC_ADC_ASCALE") != scale_factors.end())
    a_scale = scale_factors["SC_ADC_ASCALE"];
  else
    jerr << "Unable to get SC_ADC_ASCALE from /START_COUNTER/digi_scales !" 
	 << endl;
  // t_scale (SC_ADC_SCALE)
  if (scale_factors.find("SC_ADC_TSCALE") != scale_factors.end())
    t_scale = scale_factors["SC_ADC_TSCALE"];
  else
    jerr << "Unable to get SC_ADC_TSCALE from /START_COUNTER/digi_scales !" 
	 << endl;
  
  // load base time offset
  map<string,double> base_time_offset;
  // t_base (SC_BASE_TIME_OFFSET)
  if (DEvent::GetCalib(event,"/START_COUNTER/base_time_offset",base_time_offset))
    jout << "Error loading /START_COUNTER/base_time_offset !" << endl;
  if (base_time_offset.find("SC_BASE_TIME_OFFSET") != base_time_offset.end())
    t_base = base_time_offset["SC_BASE_TIME_OFFSET"];
  else
    jerr << "Unable to get SC_BASE_TIME_OFFSET from /START_COUNTER/base_time_offset !" << endl;
  // t_tdc_base (SC_TDC_BASE_TIME_OFFSET)
  if (base_time_offset.find("SC_TDC_BASE_TIME_OFFSET") != base_time_offset.end())
    t_tdc_base = base_time_offset["SC_TDC_BASE_TIME_OFFSET"];
  else
    jerr << "Unable to get SC_BASE_TIME_OFFSET from /START_COUNTER/base_time_offset !" << endl;
  
  // load constant tables
  // a_gains (gains)
  if (DEvent::GetCalib(event,"/START_COUNTER/gains", a_gains))
    jout << "Error loading /START_COUNTER/gains !" << endl;
  // a_pedestals (pedestals)
  if (DEvent::GetCalib(event,"/START_COUNTER/pedestals", a_pedestals))
    jout << "Error loading /START_COUNTER/pedestals !" << endl;
  // adc_time_offsets (adc_timing_offsets)
  if (DEvent::GetCalib(event,"/START_COUNTER/adc_timing_offsets", adc_time_offsets))
    jout << "Error loading /START_COUNTER/adc_timing_offsets !" << endl;
  // tdc_time_offsets (tdc_timing_offsets)
  if (DEvent::GetCalib(event,"/START_COUNTER/tdc_timing_offsets", tdc_time_offsets))
    jout << "Error loading /START_COUNTER/tdc_timing_offsets !" << endl;
  // timewalk_parameters (timewalk_parms)
  if(DEvent::GetCalib(event,"START_COUNTER/timewalk_parms_v2", timewalk_parameters))
    jout << "Error loading /START_COUNTER/timewalk_parms_v2 !" << endl;
  
}

//------------------
// Process
//------------------
/// \fn void DSCHit_factory::Process(const std::shared_ptr<const JEvent> &event)
/// \param loop : 
/// \param eventnumber:
void DSCHit_factory::Process(const std::shared_ptr<const JEvent> &event)
{
    /// Generate DSCHit object for each DSCDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.

    /// First, make DSCHit(s) out of all fADC250 DSCDigiHit(s)
    vector<const DSCDigiHit*> digihits;
    event->Get(digihits);
    sort(digihits.begin(),digihits.end(),DSCHit_fadc_cmp);

	const DTTabUtilities* locTTabUtilities = nullptr;
	event->Get(&locTTabUtilities);

    char str[256];
    vector<DSCHit *>temp_schits;
    /// Loop over all ADC Digi Hits apply offset parameters
    /// this includes subracting pedestals from ADC amplituded and integrals before
    /// converting to energy in GeV
    for (unsigned int i = 0; i < digihits.size(); i++)  {
        const DSCDigiHit *digihit = digihits[i];

        // Make sure sector is in valid range
        if( (digihit->sector <= 0) && (digihit->sector > MAX_SECTORS))  {
            sprintf(str, "DSCDigiHit sector out of range! sector=%d (should be 1-%d)", 
                    digihit->sector, MAX_SECTORS);
            throw JException(str);
        }

        // Throw away hits with firmware errors (post-summer 2016 firmware)
        if(CHECK_FADC_ERRORS && !locTTabUtilities->CheckFADC250_NoErrors(digihit->QF))
            continue;
	if (digihit->pulse_time == 0 || digihit->pedestal == 0 || digihit->pulse_peak == 0) continue;
	// Find the time from the pulse and apply calibration constants
        double T = (double)digihit->pulse_time;
	double t_fadc= t_scale * T - adc_time_offsets[digihit->sector-1] + t_base;
	if (fabs(t_fadc) > HIT_TIME_WINDOW) continue;

        // Initialize pedestal to one found in CCDB, but override it
        // with one found in event if is available (?)
        // For now, only keep events with a correct pedestal
        double pedestal = a_pedestals[digihit->sector-1];
        double nsamples_integral = digihit->nsamples_integral;
        double nsamples_pedestal = digihit->nsamples_pedestal;

        // nsamples_pedestal should always be positive for valid data - err on the side of caution for now
        if(nsamples_pedestal == 0) {
            jerr << "DSCDigiHit with nsamples_pedestal == 0 !   Event = " << event->GetEventNumber() << jendl;
            continue;
        }

        // digihit->pedestal is the sum of "nsamples_pedestal" samples
        // Calculate the average pedestal per sample
        if( (digihit->pedestal>0) && locTTabUtilities->CheckFADC250_PedestalOK(digihit->QF) ) {
            pedestal = (double)digihit->pedestal/nsamples_pedestal;
        }
        // Subtract pedestal from pulse peak      
        double pulse_peak = digihit->pulse_peak - pedestal;

        // Subtract pedestal from pulse integral
        double A = (double)digihit->pulse_integral;
        A -= pedestal*nsamples_integral;

        //if ( ((double)digihit->pulse_integral) < ADC_THRESHOLD) continue; // Will comment out until this is set to something useful by default

        DSCHit *hit = new DSCHit;
        // Sectors are numbered from 1-30
        hit->sector = digihit->sector;

        hit->dE = a_scale * a_gains[digihit->sector-1] * A;
        hit->t_fADC = t_fadc;
        hit->t_TDC = numeric_limits<double>::quiet_NaN();

        hit->has_TDC = false;
        hit->has_fADC = true;
	
        hit->t = hit->t_fADC; // set time from fADC in case no TDC hit
        hit->pulse_height = pulse_peak;
	
        hit->AddAssociatedObject(digihit);
	
        temp_schits.push_back(hit);
    }
    

    /// Next, loop over TDC hits, matching them to the
    /// existing fADC hits where possible and updating
    /// their time information. If no match is found, then
    /// create a new hit with just the TDC info.
    vector<const DSCTDCDigiHit*> tdcdigihits;
    event->Get(tdcdigihits);
    sort(tdcdigihits.begin(),tdcdigihits.end(),DSCHit_tdc_cmp);
    
    for (unsigned int i = 0; i < tdcdigihits.size(); i++) {
      const DSCTDCDigiHit *digihit = tdcdigihits[i];
      
      // Make sure sector is in valid range
      if((digihit->sector <= 0) && (digihit->sector > MAX_SECTORS)) {
	sprintf(str, "DSCDigiHit sector out of range! sector=%d (should be 1-%d)",
		digihit->sector, MAX_SECTORS);
	throw JException(str);
      }
      
      unsigned int id = digihit->sector - 1;
      double T = locTTabUtilities->Convert_DigiTimeToNs_F1TDC(digihit) - tdc_time_offsets[id] + t_tdc_base;
      
      // Look for existing hits to see if there is a match
      //   or create new one if there is no match
      // Require that the trigger corrected TDC time fall within
      //   a reasonable time window so that when a hit is associated with
      //   a hit in the TDC and not the ADC it is a "decent" TDC hit
      if (fabs(T) < HIT_TIME_WINDOW) {
	DSCHit *hit = FindMatch(temp_schits,digihit->sector, T);
	if (hit == nullptr) {
	  hit = new DSCHit;
	  hit->sector = digihit->sector;
	  hit->dE = 0.0;
	  hit->t_fADC= numeric_limits<double>::quiet_NaN();
	  hit->has_fADC=false;
	  temp_schits.push_back(hit);
	}
	
	hit->has_TDC=true;
	hit->t_TDC=T;
	
	if (USE_TIMEWALK_CORRECTION && (hit->dE > 0.0) ) {
	  /// Correct for time walk
	  /// The correction is the form t=t_tdc- C1 (A^C2 - A0^C2)
	  /// double A  = hit->dE;
	  /// double C1 = timewalk_parameters[id][1];
	  /// double C2 = timewalk_parameters[id][2];
	  /// double A0 = timewalk_parameters[id][3];
	  /// T -= C1*(pow(A,C2) - pow(A0,C2));
	  
	  // Correct for timewalk using pulse peak instead of pulse integral
	  double A        = hit->pulse_height;
	  // double C0       = timewalk_parameters[id][0];
	  double C1       = timewalk_parameters[id][1];
	  double C2       = timewalk_parameters[id][2];
	  double A_THRESH = timewalk_parameters[id][3];
	  double A0       = timewalk_parameters[id][4];
	  // do correction
	  T -= C1*(pow(A/A_THRESH, C2) - pow(A0/A_THRESH, C2));
	}
	
	hit->t=T;

	hit->AddAssociatedObject(digihit);
      } // Hit time window cut
      
    }
    
    for (unsigned int i=0;i<temp_schits.size();i++){
      if (REQUIRE_ADC_TDC_MATCH==false){
	mData.push_back(temp_schits[i]);
      }
      else if (temp_schits[i]->has_fADC && temp_schits[i]->has_TDC){
	mData.push_back(temp_schits[i]);
      }
      else delete temp_schits[i];
    }
}

//------------------
// FindMatch
//------------------
DSCHit* DSCHit_factory::FindMatch(vector<DSCHit*>&schits,int sector, double T)
{
    DSCHit *best_match = nullptr;

    // Loop over existing hits (from fADC) and look for a match
    // in both the sector and the time.
    for(unsigned int i = 0; i < schits.size(); i++)
    {
        DSCHit *hit = schits[i];

        if (! isfinite(hit->t_fADC))
	  continue; // only match to fADC hits, not bachelor TDC hits

        if (hit->sector != sector)
	  continue; // require identical sectors fired

        double delta_T = fabs(hit->t - T);
        if (delta_T > DELTA_T_ADC_TDC_MAX)
	  continue;
	  
        if (best_match != nullptr)
        {
            if (delta_T < fabs(best_match->t - T))
                best_match = hit;
        } else best_match = hit;
    }

    return best_match;
}

//------------------
// EndRun
//------------------
void DSCHit_factory::EndRun()
{
    return;
}

//------------------
// Finish
//------------------
void DSCHit_factory::Finish()
{
    return;
}


//------------------------------------
// GetConstant
//   Allow a few different interfaces
//------------------------------------
const double DSCHit_factory::GetConstant(const vector<double> &the_table,
        const int in_sector) const
{
    char str[256];

    if ( (in_sector < 0) || (in_sector >= MAX_SECTORS)) 
    {
        sprintf(str, "Bad sector # requested in DSCHit_factory::GetConstant()!"
                " requested=%d , should be %ud", in_sector, MAX_SECTORS);
        cerr << str << endl;
        throw JException(str);
    }

    return the_table[in_sector];
}

const double DSCHit_factory::GetConstant(const vector<double> &the_table,
        const DSCDigiHit *in_digihit) const 
{
    char str[256];

    if ( (in_digihit->sector < 0) || (in_digihit->sector >= MAX_SECTORS)) 
    {
        sprintf(str, "Bad sector # requested in DSCHit_factory::GetConstant()!"
                " requested=%d , should be %ud", 
                in_digihit->sector, MAX_SECTORS);
        cerr << str << endl;
        throw JException(str);
    }

    return the_table[in_digihit->sector];
}

const double DSCHit_factory::GetConstant(const vector<double> &the_table,
        const DSCHit *in_hit) const 
{

    char str[256];

    if ( (in_hit->sector < 0) || (in_hit->sector >= MAX_SECTORS)) 
    {
        sprintf(str, "Bad sector # requested in DSCHit_factory::GetConstant()!"
                " requested=%d , should be %ud",
                in_hit->sector, MAX_SECTORS);
        cerr << str << endl;
        throw JException(str);
    }

    return the_table[in_hit->sector];
}
/*
   const double DSCHit_factory::GetConstant(const vector<double> &the_table,
   const DTranslationTable *ttab,
   const int in_rocid,
   const int in_slot, 
   const int in_channel) const
   {
   char str[256];

   DTranslationTable::csc_t daq_index = { in_rocid, in_slot, in_channel };
   DTranslationTable::DChannelInfo channel_info = ttab->GetDetectorIndex(daq_index);

   if( (channel_info.sc.sector <= 0) 
   || (channel_info.sc.sector > static_cast<unsigned int>(MAX_SECTORS))) {
   sprintf(str, "Bad sector # requested in DSCHit_factory::GetConstant()!"
   " requested=%d , should be %ud",
   channel_info.sc.sector, MAX_SECTORS);
   cerr << str << endl;
   throw JException(str);
   }

   return the_table[channel_info.sc.sector];
   }
   */
