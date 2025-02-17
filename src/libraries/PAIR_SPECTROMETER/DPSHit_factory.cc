// $Id$
//
//    File: DPSHit_factory.cc
// Created: Wed Oct 15 16:45:01 EDT 2014
// Creator: staylor (on Linux gluon05.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//
#include <iostream>
#include <iomanip>
using namespace std;

#include "DPSHit_factory.h"
#include <DAQ/Df250PulsePedestal.h>
#include <DAQ/Df250PulseIntegral.h>
#include <TTAB/DTTabUtilities.h>

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>


//------------------
// Init
//------------------
void DPSHit_factory::Init()
{
  auto app = GetApplication();
  ADC_THRESHOLD = 500.0; // ADC integral counts
  app->SetDefaultParameter("PSHit:ADC_THRESHOLD",ADC_THRESHOLD,
			      "pedestal-subtracted pulse integral threshold");	
	
  /// set the base conversion scales
  a_scale    = 0.0001; 
  t_scale    = 0.0625;   // 62.5 ps/count
  t_base     = 0.;    // ns
	
  CHECK_FADC_ERRORS = true;
  app->SetDefaultParameter("PSHit:CHECK_FADC_ERRORS", CHECK_FADC_ERRORS, "Set to 1 to reject hits with fADC250 errors, ser to 0 to keep these hits");
}

//------------------
// BeginRun
//------------------
void DPSHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
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
	
  /// Read in calibration constants
  if(print_messages) jout << "In DPSHit_factory, loading constants..." << jendl;
	
  // extract the PS Geometry
  vector<const DPSGeometry*> psGeomVect;
  event->Get(psGeomVect);
  if (psGeomVect.size() < 1)
    return; // OBJECT_NOT_AVAILABLE;
  const DPSGeometry& psGeom = *(psGeomVect[0]);
	
  // load scale factors
  map<string,double> scale_factors;
  if (calibration->Get("/PHOTON_BEAM/pair_spectrometer/digi_scales", scale_factors))
    jout << "Error loading /PHOTON_BEAM/pair_spectrometer/digi_scales !" << jendl;
  if (scale_factors.find("PS_ADC_ASCALE") != scale_factors.end())
    a_scale = scale_factors["PS_ADC_ASCALE"];
  else
    jerr << "Unable to get PS_ADC_ASCALE from /PHOTON_BEAM/pair_spectrometer/digi_scales !" 
	 << jendl;
  if (scale_factors.find("PS_ADC_TSCALE") != scale_factors.end())
    t_scale = scale_factors["PS_ADC_TSCALE"];
  else
    jerr << "Unable to get PS_ADC_TSCALE from /PHOTON_BEAM/pair_spectrometer/digi_scales !" 
	 << jendl;
	
  // load base time offset
  map<string,double> base_time_offset;
  if (calibration->Get("/PHOTON_BEAM/pair_spectrometer/base_time_offset",base_time_offset))
    jout << "Error loading /PHOTON_BEAM/pair_spectrometer/base_time_offset !" << jendl;
  if (base_time_offset.find("PS_FINE_BASE_TIME_OFFSET") != base_time_offset.end())
    t_base = base_time_offset["PS_FINE_BASE_TIME_OFFSET"];
  else
    jerr << "Unable to get PS_FINE_BASE_TIME_OFFSET from /PHOTON_BEAM/pair_spectrometer/base_time_offset !" << jendl;
	
  FillCalibTable(calibration, adc_pedestals, "/PHOTON_BEAM/pair_spectrometer/fine/adc_pedestals", psGeom);
  FillCalibTable(calibration, adc_gains, "/PHOTON_BEAM/pair_spectrometer/fine/adc_gain_factors", psGeom);
  FillCalibTable(calibration, adc_time_offsets, "/PHOTON_BEAM/pair_spectrometer/fine/adc_timing_offsets", psGeom);
	
  return;
}

//------------------
// Process
//------------------
void DPSHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  /// Generate DPSHit object for each DPSDigiHit object.
  /// This is where the first set of calibration constants
  /// is applied to convert from digitzed units into natural
  /// units.
  ///
  /// Note that this code does NOT get called for simulated
  /// data in HDDM format. The HDDM event source will copy
  /// the precalibrated values directly into the _data vector.

  // extract the PS Geometry
  vector<const DPSGeometry*> psGeomVect;
  event->Get(psGeomVect);
  if (psGeomVect.size() < 1)
    return; // OBJECT_NOT_AVAILABLE;
  const DPSGeometry& psGeom = *(psGeomVect[0]);

  const DTTabUtilities* locTTabUtilities = nullptr;
  event->GetSingle(locTTabUtilities);

  // First, make hits out of all fADC250 hits
  vector<const DPSDigiHit*> digihits;
  event->Get(digihits);
  char str[256];

  for (unsigned int i=0; i < digihits.size(); i++) {
    const DPSDigiHit *digihit = digihits[i];

    // Make sure channel id is in valid range
    if( (digihit->arm < 0) || (digihit->arm >= psGeom.NUM_ARMS) ) {
      sprintf(str, "DPSDigiHit arm out of range! arm=%d (should be 0-%d)", 
	      digihit->arm, psGeom.NUM_ARMS);
      throw JException(str);
    }
    if( (digihit->column <= 0) || (digihit->column > psGeom.NUM_FINE_COLUMNS) ) {
      sprintf(str, "DPSDigiHit column out of range! column=%d (should be 0-%d)", 
	      digihit->column, psGeom.NUM_FINE_COLUMNS);
      throw JException(str);
    }

    // Throw away hits with firmware errors (post-summer 2016 firmware)
    if(CHECK_FADC_ERRORS && !locTTabUtilities->CheckFADC250_NoErrors(digihit->QF))
        continue;

    // Get pedestal, prefer associated event pedestal if it exists,
    // otherwise, use the average pedestal from CCDB
    double pedestal = GetConstant(adc_pedestals,digihit,psGeom);
    double nsamples_integral = (double)digihit->nsamples_integral;
    double nsamples_pedestal = (double)digihit->nsamples_pedestal;

    // nsamples_pedestal should always be positive for valid data - err on the side of caution for now
    if(nsamples_pedestal == 0) {
        jerr << "DPSDigiHit with nsamples_pedestal == 0 !   Event = " << event->GetEventNumber() << jendl;
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
    double A = (double)digihit->pulse_integral;
    A -= pedestal*nsamples_integral;

    // Throw away hits with small pedestal-subtracted integrals
    if (A < ADC_THRESHOLD) continue;

    // Apply calibration constants
    double T = (double)digihit->pulse_time;

    DPSHit *hit = new DPSHit;
    // The PSHit class labels hits as
    //   arm:     North/South (0/1)
    //   column:    1-145
    hit->arm     = digihit->arm;
    hit->column  = digihit->column;
    hit->integral = A;
    hit->pulse_peak = pulse_peak;
    hit->npix_fadc = A * a_scale * GetConstant(adc_gains, digihit, psGeom);
    hit->t = t_scale * T - GetConstant(adc_time_offsets, digihit, psGeom) + t_base;
    hit->E = 0.5*(psGeom.getElow(digihit->arm,digihit->column) + psGeom.getEhigh(digihit->arm,digihit->column));

    hit->AddAssociatedObject(digihit);
                
    Insert(hit);
  }
}

//------------------
// EndRun
//------------------
void DPSHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DPSHit_factory::Finish()
{
}

//------------------
// FillCalibTable
//------------------
void DPSHit_factory::FillCalibTable(JCalibration* calibration, ps_digi_constants_t &table, string table_name,
				    const DPSGeometry &psGeom)
{
  char str[256];

  // load constant table
  if(calibration->Get(table_name, table))
    jout << "Error loading " + table_name + " !" << jendl;

	
  // check that the size of the tables loaded are correct
  if( (int)table.size() != psGeom.NUM_FINE_COLUMNS ) {
    sprintf(str, "PS table loaded with wrong size! number of columns=%d (should be %d)", 
	    (int)table.size(), psGeom.NUM_FINE_COLUMNS );
    cerr << str << endl;
    throw JException(str);
  }
	
  for( int column=0; column < psGeom.NUM_FINE_COLUMNS; column++) {
    if( (int)table[column].size() != psGeom.NUM_ARMS ) {
      sprintf(str, "PS table loaded with wrong size! column=%d number of arms=%d (should be %d)", 
	      column, (int)table[column].size(), psGeom.NUM_ARMS );
      cerr << str << endl;
      throw JException(str);
    }
  }
}

//------------------------------------
// GetConstant
//   Allow a few different interfaces
//
//   PS Geometry as defined in the Translation Table:
//       arm:     North/South (0/1)
//       column:  1-145
//   Note the different counting schemes used
//------------------------------------
const double DPSHit_factory::GetConstant( const ps_digi_constants_t &the_table, 
					  const DPSGeometry::Arm in_arm, const int in_column,
					  const DPSGeometry &psGeom ) const
{
  char str[256];
        
  if( (in_arm != DPSGeometry::kNorth) && (in_arm != DPSGeometry::kSouth)) {
    sprintf(str, "Bad arm requested in DPSHit_factory::GetConstant()! requested=%d , should be 0-%d", 
	    static_cast<int>(in_arm), static_cast<int>(DPSGeometry::kSouth));
    cerr << str << endl;
    throw JException(str);
  }
  if( (in_column <= 0) || (in_column > psGeom.NUM_FINE_COLUMNS)) {
    sprintf(str, "Bad column # requested in DPSHit_factory::GetConstant()! requested=%d , should be 1-%d", in_column, psGeom.NUM_FINE_COLUMNS);
    cerr << str << endl;
    throw JException(str);
  }

  // the tables are indexed by column, with the different values for the two arms
  // stored in the two fields of the pair
  if(in_arm == DPSGeometry::kNorth) {
    return the_table[in_column-1][in_arm];
  } else {
    return the_table[in_column-1][in_arm];
  }
}

const double DPSHit_factory::GetConstant( const ps_digi_constants_t &the_table, 
					  const DPSHit *in_hit, const DPSGeometry &psGeom ) const
{
  char str[256];
        
  if( (in_hit->arm != DPSGeometry::kNorth) && (in_hit->arm != DPSGeometry::kSouth)) {
    sprintf(str, "Bad arm requested in DPSHit_factory::GetConstant()! requested=%d , should be 0-%d", 
	    static_cast<int>(in_hit->arm), static_cast<int>(DPSGeometry::kSouth));
    cerr << str << endl;
    throw JException(str);
  }
  if( (in_hit->column <= 0) || (in_hit->column > psGeom.NUM_FINE_COLUMNS)) {
    sprintf(str, "Bad column # requested in DPSHit_factory::GetConstant()! requested=%d , should be 1-%d", in_hit->column, psGeom.NUM_FINE_COLUMNS);
    cerr << str << endl;
    throw JException(str);
  }

  // the tables are indexed by column, with the different values for the two arms
  // stored in the two fields of the pair
  if(in_hit->arm == DPSGeometry::kNorth) {
    return the_table[in_hit->column-1][in_hit->arm];
  } else {
    return the_table[in_hit->column-1][in_hit->arm];
  }
}

const double DPSHit_factory::GetConstant( const ps_digi_constants_t &the_table, 
					  const DPSDigiHit *in_digihit, const DPSGeometry &psGeom) const
{
  char str[256];

  if( (in_digihit->arm != DPSGeometry::kNorth) && (in_digihit->arm != DPSGeometry::kSouth)) {
    sprintf(str, "Bad arm requested in DPSHit_factory::GetConstant()! requested=%d , should be 0-%d", 
	    static_cast<int>(in_digihit->arm), static_cast<int>(DPSGeometry::kSouth));
    cerr << str << endl;
    throw JException(str);
  }
  if( (in_digihit->column <= 0) || (in_digihit->column > psGeom.NUM_FINE_COLUMNS)) {
    sprintf(str, "Bad column # requested in DPSHit_factory::GetConstant()! requested=%d , should be 1-%d", in_digihit->column, psGeom.NUM_FINE_COLUMNS);
    cerr << str << endl;
    throw JException(str);
  }

  // the tables are indexed by column, with the different values for the two arms
  // stored in the two fields of the pair
  if(in_digihit->arm == DPSGeometry::kNorth) {
    return the_table[in_digihit->column-1][in_digihit->arm];
  } else {
    return the_table[in_digihit->column-1][in_digihit->arm];
  }
}

