// $Id$
//
//    File: DBCALHit_factory.cc
// Created: Tue Aug  6 09:26:13 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//


#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include <BCAL/DBCALDigiHit.h>
#include "DBCALGeometry.h"
#include "DBCALHit_factory.h"
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250Config.h>
#include <TTAB/DTTabUtilities.h>


//------------------
// Init
//------------------
void DBCALHit_factory::Init()
{
  t_scale    = 0.0625;   // There are 62.5 ps/count from the fADC
  t_base     = 0.;

  auto app = GetApplication();

  PRINTCALIBRATION = false;
  app->SetDefaultParameter("BCALHIT:PRINTCALIBRATION", PRINTCALIBRATION, "Print the calibration parameters.");
  VERBOSE = 0;
  app->SetDefaultParameter("BCALHIT:VERBOSE", VERBOSE, "Set level of verbosity.");
  CHECK_FADC_ERRORS = true;
  app->SetDefaultParameter("BCAL:CHECK_FADC_ERRORS", CHECK_FADC_ERRORS, "Set to 1 to reject hits with fADC250 errors, ser to 0 to keep these hits");
  CORRECT_FADC_SATURATION = true;
  app->SetDefaultParameter("BCAL:CORRECT_FADC_SATURATION", CORRECT_FADC_SATURATION, "Set to 1 to correct pulse integral for fADC saturation, set to 0 to not correct pulse integral. (default = 1)");
  CORRECT_SIPM_SATURATION = true;
  app->SetDefaultParameter("BCAL:CORRECT_SIPM_SATURATION", CORRECT_SIPM_SATURATION, "Set to 1 to correct for SiPM saturation, set to 0 to not correct pulse integral or peak. (default = 1)");

  // cout << " CORRECT_SIPM_SATURATION=" << CORRECT_SIPM_SATURATION << endl;
}

//------------------
// BeginRun
//------------------
void DBCALHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  auto event_number = event->GetEventNumber();
  auto run_number = event->GetRunNumber();
  auto app = GetApplication();
  auto calibration = app->GetService<JCalibrationManager>()->GetJCalibration(run_number);

  // Only print messages for one thread whenever run number changes
  static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
  static set<int> runs_announced;
  pthread_mutex_lock(&print_mutex);
  bool print_messages = false;
  if(runs_announced.find(run_number) == runs_announced.end()){
    print_messages = true;
    runs_announced.insert(run_number);
  }
  pthread_mutex_unlock(&print_mutex);

   /// Read in calibration constants
   vector<double> raw_gains;
   vector<double> raw_pedestals;
   vector<double> raw_ADC_timing_offsets;
   vector<double> raw_channel_global_offset;
   vector<double> raw_tdiff_u_d;
   vector<double> raw_bad_channels;

    if(print_messages) jout << "In DBCALHit_factory, loading constants..." << jendl;
   
   // load scale factors
   map<string,double> scale_factors;
   if (calibration->Get("/BCAL/digi_scales", scale_factors, event_number))
       jout << "Error loading /BCAL/digi_scales !" << jendl;
   if (scale_factors.find("BCAL_ADC_ASCALE") != scale_factors.end())
       a_scale = scale_factors["BCAL_ADC_ASCALE"];
   else
       jerr << "Unable to get BCAL_ADC_ASCALE from /BCAL/digi_scales !" << jendl;
   if (scale_factors.find("BCAL_ADC_TSCALE") != scale_factors.end()) {
     t_scale = scale_factors["BCAL_ADC_TSCALE"];
     if (PRINTCALIBRATION) {
       jout << "DBCALHit_factory >>BCAL_ADC_TSCALE = " << t_scale << jendl;
     }
   }
   else
       jerr << "Unable to get BCAL_ADC_TSCALE from /BCAL/digi_scales !" << jendl;

  // load base time offset
   map<string,double> base_time_offset;
   if (calibration->Get("/BCAL/base_time_offset",base_time_offset))
       jout << "Error loading /BCAL/base_time_offset !" << jendl;
   if (base_time_offset.find("BCAL_BASE_TIME_OFFSET") != base_time_offset.end()) {
     t_base = base_time_offset["BCAL_BASE_TIME_OFFSET"];
     if (PRINTCALIBRATION) {
       jout << "DBCALHit_factory >>BCAL_BASE_TIME_OFFSET = " << t_base << jendl;
     }
   }
   else
       jerr << "Unable to get BCAL_BASE_TIME_OFFSET from /BCAL/base_time_offset !" << jendl;

   // load constant tables
   if (calibration->Get("/BCAL/ADC_gains", raw_gains))
       jout << "Error loading /BCAL/ADC_gains !" << jendl;
   if (calibration->Get("/BCAL/ADC_pedestals", raw_pedestals))
       jout << "Error loading /BCAL/ADC_pedestals !" << jendl;
   if (calibration->Get("/BCAL/ADC_timing_offsets", raw_ADC_timing_offsets))
       jout << "Error loading /BCAL/ADC_timing_offsets !" << jendl;
   if(calibration->Get("/BCAL/channel_global_offset", raw_channel_global_offset))
       jout << "Error loading /BCAL/channel_global_offset !" << jendl;
   if(calibration->Get("/BCAL/tdiff_u_d", raw_tdiff_u_d))
       jout << "Error loading /BCAL/tdiff_u_d !" << jendl;
    if(calibration->Get("/BCAL/bad_channels", raw_bad_channels))
       jout << "Error loading /BCAL/bad_channels !" << endl;

   if (PRINTCALIBRATION) jout << "DBCALHit_factory >> raw_gains" << jendl;
   FillCalibTable(gains, raw_gains);
   if (PRINTCALIBRATION) jout << "DBCALHit_factory >> raw_pedestals" << jendl;
   FillCalibTable(pedestals, raw_pedestals);
   if (PRINTCALIBRATION) jout << "DBCALHit_factory >> raw_ADC_timing_offsets" << jendl;
   FillCalibTable(ADC_timing_offsets, raw_ADC_timing_offsets);
   if (PRINTCALIBRATION) jout << "DBCALHit_factory >> raw_channel_global_offset" << jendl;
   FillCalibTableShort(channel_global_offset, raw_channel_global_offset);
   if (PRINTCALIBRATION) jout << "DBCALHit_factory >> raw_tdiff_u_d" << jendl;
   FillCalibTableShort(tdiff_u_d, raw_tdiff_u_d);
   if (PRINTCALIBRATION) jout << "DBCALHit_factory >> raw_bad_channels" << endl;
   FillCalibTable(bad_channels, raw_bad_channels);
   
   std::vector<std::map<string,double> > saturation_ADC_pars;
   if(calibration->Get("/BCAL/ADC_saturation", saturation_ADC_pars))
      jout << "Error loading /BCAL/ADC_saturation !" << jendl;
   for (unsigned int i=0; i < saturation_ADC_pars.size(); i++) {
	   int end = (saturation_ADC_pars[i])["end"];
	   int layer = (saturation_ADC_pars[i])["layer"] - 1;
	   fADC_MinIntegral_Saturation[end][layer] = (saturation_ADC_pars[i])["par0"];
	   fADC_Saturation_Linear[end][layer] = (saturation_ADC_pars[i])["par1"];
	   fADC_Saturation_Quadratic[end][layer] = (saturation_ADC_pars[i])["par2"];
   } 

   // load parameters for SiPM saturation
   std::vector<std::map<string,double> > saturation_SiPM_pars;
   if(calibration->Get("/BCAL/SiPM_saturation", saturation_SiPM_pars))
      jout << "Error loading /SiPM/SiPM_saturation !" << jendl;
   for (unsigned int i=0; i < saturation_SiPM_pars.size(); i++) {
	   int end = (saturation_SiPM_pars[i])["END"];
	   int layer = (saturation_SiPM_pars[i])["LAYER"] - 1;
	   integral_to_peak[end][layer] = (saturation_SiPM_pars[i])["INTEGRAL_TO_PEAK"];
	   sipm_npixels[end][layer] = (saturation_SiPM_pars[i])["SIPM_NPIXELS"];
	   pixel_per_count[end][layer] = (saturation_SiPM_pars[i])["PIXEL_PER_COUNT"];
   } 

}

//------------------
// Process
//------------------
void DBCALHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
   /// Generate DBCALHit object for each DBCALDigiHit object.
   /// This is where the first set of calibration constants
   /// is applied to convert from digitzed units into natural
   /// units.
   ///
   /// Note that this code does NOT get called for simulated
   /// data in HDDM format. The HDDM event source will copy
   /// the precalibrated values directly into the _data vector.

   const DTTabUtilities* locTTabUtilities = event->GetSingle<DTTabUtilities>();

   vector<const DBCALDigiHit*> digihits;
   event->Get(digihits);
   for(unsigned int i=0; i<digihits.size(); i++){
      const DBCALDigiHit *digihit = digihits[i];

      // throw away hits from bad or noisy channels
      int quality = GetConstant(bad_channels,digihit);
      if ( quality > 0 ) continue;

      // Error checking for pre-Fall 2016 firmware
      if(digihit->datasource == 1) {
          // There is a slight difference between Mode 7 and 8 data
          // The following condition signals an error state in the flash algorithm
          // Do not make hits out of these
          auto PPobj = digihit->GetSingle<Df250PulsePedestal>();
          if (PPobj != NULL){
              if (PPobj->pedestal == 0 || PPobj->pulse_peak == 0) continue;
          }
      }
     
      if(CHECK_FADC_ERRORS && !locTTabUtilities->CheckFADC250_NoErrors(digihit->QF))
          continue;

      // parse digi-hit data
      double integral, pedestal, nsamples_integral, nsamples_pedestal;
      if(digihit->datasource == 1) {      // pre-fall 2016 firmware
          // Get Df250PulseIntegral object from DBCALDigiHit object
          auto PIobj = digihit->GetSingle<Df250PulseIntegral>();

          if (PIobj == NULL && digihit->pedestal == 1 && digihit->QF == 1){ // This is a simulated event.  Set the pedestal to zero.
              integral = (double)digihit->pulse_integral;
              pedestal = 0.;
              nsamples_integral = 0.;
              nsamples_pedestal = 1.;
          } else {       
              // Calculate attenuated energy for channel
              integral          = (double)PIobj->integral;
              pedestal          = (double)PIobj->pedestal;
              nsamples_integral = (double)PIobj->nsamples_integral;
              nsamples_pedestal = (double)PIobj->nsamples_pedestal;
          }
      } else {                        // post-fall 2016 firmware  (& emulated hits)
          // Calculate attenuated energy for channel
          integral          = (double)digihit->pulse_integral;
          // require an accurate pedestal
          if(locTTabUtilities->CheckFADC250_PedestalOK(digihit->QF)) {
              pedestal = (double)digihit->pedestal;
          } else {
              //pedestal = 0.;   // need to set to some reasonable default value?
              continue;
          }
          nsamples_integral = (double)digihit->nsamples_integral;
          nsamples_pedestal = (double)digihit->nsamples_pedestal;
      }

      // nsamples_pedestal should always be positive for valid data - err on the side of caution for now
      if(nsamples_pedestal == 0) {
          //throw JException("DBCALDigiHit with nsamples_pedestal == 0 !");
          if(VERBOSE>0)jerr << "DBCALDigiHit with nsamples_pedestal == 0 !   Event = " << event->GetEventNumber() << jendl;
          continue;
      }

      //double totalpedestal     = pedestal * nsamples_integral/nsamples_pedestal;
      double single_sample_ped = pedestal/nsamples_pedestal;
      double totalpedestal     = nsamples_integral * single_sample_ped;

      double gain              = GetConstant(gains,digihit);
      double hit_E = 0;
      

      // make corrections for SiPM saturation

      double integral_pedsub =0;
      if ( integral > 0 ) { 
	// compute in double precision to prevent round off errors
      integral_pedsub = integral - totalpedestal;
	if(CORRECT_FADC_SATURATION && integral_pedsub > fADC_MinIntegral_Saturation[digihit->end][digihit->layer-1]) {
		if(digihit->pulse_peak > 4094 || (digihit->pedestal == 1 && digihit->QF == 1)) { // check if fADC is saturated or is MC event
			double locSaturatedIntegral = integral_pedsub - fADC_MinIntegral_Saturation[digihit->end][digihit->layer-1];
			double locScaleFactor = 1. + fADC_Saturation_Linear[digihit->end][digihit->layer-1]*locSaturatedIntegral + fADC_Saturation_Quadratic[digihit->end][digihit->layer-1]*locSaturatedIntegral*locSaturatedIntegral;
	    		integral_pedsub *= 1./locScaleFactor;
		}
	} 
	// make corrections for SiPM saturation (after correcting for fADC saturation)
	// compute in double precision to prevent round off errors
	if (CORRECT_SIPM_SATURATION) {
	  double integral_pedsub_measured = integral_pedsub;
	  double Npixels_measured = pixel_per_count[digihit->end][digihit->layer-1]*integral_pedsub_measured;
	  double Mpixels = sipm_npixels[digihit->end][digihit->layer-1];
	  double Npixels_true = Npixels_measured < Mpixels? -Mpixels*log(1 - Npixels_measured/Mpixels) : Mpixels;
	  integral_pedsub = Npixels_true/pixel_per_count[digihit->end][digihit->layer-1];
	  // cout << " event=" << eventnumber << " Layer=" << digihit->layer << " integral_pedsub_measured=" << integral_pedsub_measured 
	  //	     << " Npixels_measured=" << Npixels_measured << " Mpixels=" << Mpixels << " integral_pedsub=" << integral_pedsub << endl;
	}
	hit_E = gain * integral_pedsub;

      }
      if (VERBOSE>2) printf("%5llu digihit %2i of %2lu, type %i time %4u, peak %3u, int %4.0f %.0f, ped %3.0f %.0f %5.1f %6.1f, gain %.1e, E=%5.0f MeV\n",
							event->GetEventNumber(),i,digihits.size(),digihit->datasource,
							digihit->pulse_time,digihit->pulse_peak,integral,nsamples_integral,
							pedestal,nsamples_pedestal,single_sample_ped,totalpedestal,gain,hit_E*1000);
      if ( hit_E <= 0 ) continue;  // Throw away negative energy hits  

      // integral and peak should be corrected separately because the ratio is not necessarily a constant.
      //
      int pulse_peak_pedsub=0;
      	if (CORRECT_SIPM_SATURATION) {
	  double pulse_peak_pedsub_measured = (int)digihit->pulse_peak - (int)single_sample_ped;
	  double Npixels_measured = pixel_per_count[digihit->end][digihit->layer-1]*pulse_peak_pedsub_measured*integral_to_peak[digihit->end][digihit->layer-1];
	  double Mpixels = sipm_npixels[digihit->end][digihit->layer-1];
	  double Npixels_true = Npixels_measured < Mpixels? -Mpixels*log(1 - Npixels_measured/Mpixels) : Mpixels;
	  pulse_peak_pedsub = round(Npixels_true/pixel_per_count[digihit->end][digihit->layer-1]/integral_to_peak[digihit->end][digihit->layer-1]);
	  // cout  << " event=" << eventnumber << " Layer=" << digihit->layer  << " pulse_peak_pedsub_measured=" << pulse_peak_pedsub_measured 
	  //	    << " Npixels_measured=" << Npixels_measured << " Mpixels=" << Mpixels << " pulse_peak_pedsub=" << pulse_peak_pedsub 
	  //       << " Int/Peak Ratio=" << integral_pedsub/pulse_peak_pedsub << endl << endl;
	}
	else {
	  pulse_peak_pedsub = (int)digihit->pulse_peak - (int)single_sample_ped;
	}
 
      // Calculate time for channel
      double pulse_time        = (double)digihit->pulse_time;
      double end_sign          = digihit->end ? -1.0 : 1.0; // Upstream = 0 -> Positive (then subtracted)
      double hit_t_raw         = t_scale * pulse_time + t_base;
      double hit_t             = t_scale * pulse_time + t_base
          + GetConstant(ADC_timing_offsets,digihit)              // low level indiviual corrections (eg 4 ns offset)
          - GetConstant(channel_global_offset,digihit)
          - (0.5 * end_sign) * GetConstant(tdiff_u_d,digihit);
      if (VERBOSE>2) printf("      %2i %i %i %i        , t: %4.0f %.4f %7.3f traw=%7.3f  %7.3f %7.3f %7.3f t=%7.3f\n",
                            digihit->module, digihit->layer, digihit->sector, digihit->end, 
                            pulse_time,t_scale,t_base,hit_t_raw,
                            GetConstant(ADC_timing_offsets,digihit),
                            GetConstant(channel_global_offset,digihit),
                            (0.5 * end_sign * GetConstant(tdiff_u_d,digihit)),hit_t);
      DBCALHit *hit = new DBCALHit;
      hit->module = digihit->module;
      hit->layer  = digihit->layer;
      hit->sector = digihit->sector;
      hit->end    = digihit->end;

      hit->E = hit_E;
      hit->pulse_peak = pulse_peak_pedsub;
      hit->t = hit_t;
      hit->t_raw = hit_t_raw;

      hit->AddAssociatedObject(digihit);

      Insert(hit);
   }
}

//------------------
// EndRun
//------------------
void DBCALHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DBCALHit_factory::Finish()
{
}


//------------------
// FillCalibTable
//------------------
void DBCALHit_factory::FillCalibTable( bcal_digi_constants_t &table, 
        const vector<double> &raw_table) 
{
    char str[256];
    int channel = 0;

    // reset the table before filling it
    table.clear();

    for (int module=1; module<=BCAL_NUM_MODULES; module++) {
        for (int layer=1; layer<=BCAL_NUM_LAYERS; layer++) {
            for (int sector=1; sector<=BCAL_NUM_SECTORS; sector++) {
                if ((channel > BCAL_MAX_CHANNELS) || (channel+1 > BCAL_MAX_CHANNELS)) {  // sanity check
                    sprintf(str, "Too many channels for BCAL table!"
                            " channel=%d (should be %d)", 
                            channel, BCAL_MAX_CHANNELS);
                    cerr << str << endl;
                    throw JException(str);
                }

                table.push_back( cell_calib_t(raw_table[channel],raw_table[channel+1]) );

                if (PRINTCALIBRATION) {
                    printf("%2i  %2i  %2i   %.10f   %.10f\n",module,layer,sector,raw_table[channel],raw_table[channel+1]);
                }

                channel += 2;
            }
        }
    }

    // check to make sure that we loaded enough channels
    if (channel != BCAL_MAX_CHANNELS) { 
        sprintf(str, "Not enough channels for BCAL table!"
                " channel=%d (should be %d)", 
                channel, BCAL_MAX_CHANNELS);
        cerr << str << endl;
        throw JException(str);
    }
}

//------------------
// FillCalibTableShort
//------------------
void DBCALHit_factory::FillCalibTableShort( bcal_digi_constants_t &table,
        const vector<double> &raw_table)
{
    char str[256];
    int channel = 0;

    // reset the table before filling it
    table.clear();

    for (int module=1; module<=BCAL_NUM_MODULES; module++) {
        for (int layer=1; layer<=BCAL_NUM_LAYERS; layer++) {
            for (int sector=1; sector<=BCAL_NUM_SECTORS; sector++) {
                if (channel > BCAL_MAX_CHANNELS/2) {  // sanity check
                    sprintf(str, "Too many channels for BCAL table!"
                            " channel=%d (should be %d)",
                            channel, BCAL_MAX_CHANNELS/2);
                    cerr << str << endl;
                    throw JException(str);
                }

                table.push_back( cell_calib_t(raw_table[channel],raw_table[channel]) );

                if (PRINTCALIBRATION) {
                    printf("%2i  %2i  %2i   %.10f\n",module,layer,sector,raw_table[channel]);
                }

                channel += 1;
            }
        }
    }

    // check to make sure that we loaded enough channels
    if (channel != BCAL_MAX_CHANNELS/2) {
        sprintf(str, "Not enough channels for BCAL table!"
                " channel=%d (should be %d)",
                channel, BCAL_MAX_CHANNELS/2);
        cerr << str << endl;
        throw JException(str);
    }
}

//------------------------------------
// GetConstant
//   Allow a few different interfaces
//------------------------------------
const double DBCALHit_factory::GetConstant(const bcal_digi_constants_t &the_table, 
        const int in_module, 
        const int in_layer,
        const int in_sector,
        const int in_end) const
{
    char str[256];

    if ( (in_module <= 0) || (in_module > BCAL_NUM_MODULES)) {
        sprintf(str, "Bad module # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 1-%d", in_module, BCAL_NUM_MODULES);
        cerr << str << endl;
        throw JException(str);
    }
    if ( (in_layer <= 0) || (in_layer > BCAL_NUM_LAYERS)) {
        sprintf(str, "Bad layer # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 1-%d", in_layer, BCAL_NUM_LAYERS);
        cerr << str << endl;
        throw JException(str);
    }
    if ( (in_sector <= 0) || (in_sector > BCAL_NUM_SECTORS)) {
        sprintf(str, "Bad sector # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 1-%d", in_sector, BCAL_NUM_SECTORS);
        cerr << str << endl;
        throw JException(str);
    }
    if ( (in_end != DBCALGeometry::kUpstream) && (in_end != DBCALGeometry::kDownstream) ) {
        sprintf(str, "Bad end # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 0-1", in_end);
        cerr << str << endl;
        throw JException(str);
    }

    const int the_cell = GetCalibIndex( in_module, in_layer, in_sector);

    if (in_end == DBCALGeometry::kUpstream) {
        // handle the upstream end
        return the_table.at(the_cell).first;
    } else {
        // handle the downstream end
        return the_table.at(the_cell).second;
    }

}

const double DBCALHit_factory::GetConstant( const bcal_digi_constants_t &the_table, 
        const DBCALHit *in_hit) const
{
    char str[256];

    if ( (in_hit->module <= 0) || (in_hit->module > BCAL_NUM_MODULES)) {
        sprintf(str, "Bad module # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 1-%d", in_hit->module, BCAL_NUM_MODULES);
        cerr << str << endl;
        throw JException(str);
    }
    if ( (in_hit->layer <= 0) || (in_hit->layer > BCAL_NUM_LAYERS)) {
        sprintf(str, "Bad layer # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 1-%d", in_hit->layer, BCAL_NUM_LAYERS);
        cerr << str << endl;
        throw JException(str);
    }
    if ( (in_hit->sector <= 0) || (in_hit->sector > BCAL_NUM_SECTORS)) {
        sprintf(str, "Bad sector # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 1-%d", in_hit->sector, BCAL_NUM_SECTORS);
        cerr << str << endl;
        throw JException(str);
    }
    if ( (in_hit->end != DBCALGeometry::kUpstream) && 
            (in_hit->end != DBCALGeometry::kDownstream) )
    {
        sprintf(str, "Bad end # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 0-1", in_hit->end);
        cerr << str << endl;
        throw JException(str);
    }

    const int the_cell = GetCalibIndex( in_hit->module, in_hit->layer, in_hit->sector );

    if (in_hit->end == DBCALGeometry::kUpstream) {
        // handle the upstream end
        return the_table.at(the_cell).first;
        //return the_table[the_cell].first;
    } else {
        // handle the downstream end
        return the_table.at(the_cell).second;
        //return the_table[the_cell].second;
    }

}

const double DBCALHit_factory::GetConstant(const bcal_digi_constants_t &the_table, 
        const DBCALDigiHit *in_digihit) const
{
    char str[256];

    if ( (in_digihit->module <= 0) || (in_digihit->module > BCAL_NUM_MODULES)) {
        sprintf(str, "Bad module # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 1-%d", 
                in_digihit->module, BCAL_NUM_MODULES);
        cerr << str << endl;
        throw JException(str);
    }
    if ( (in_digihit->layer <= 0) || (in_digihit->layer > BCAL_NUM_LAYERS)) {
        sprintf(str, "Bad layer # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 1-%d",
                in_digihit->layer, BCAL_NUM_LAYERS);
        cerr << str << endl;
        throw JException(str);
    }
    if ( (in_digihit->sector <= 0) || (in_digihit->sector > BCAL_NUM_SECTORS)) {
        sprintf(str, "Bad sector # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 1-%d", 
                in_digihit->sector, BCAL_NUM_SECTORS);
        cerr << str << endl;
        throw JException(str);
    }
    if ( (in_digihit->end != DBCALGeometry::kUpstream) &&
            (in_digihit->end != DBCALGeometry::kDownstream) )
    {
        sprintf(str, "Bad end # requested in DBCALHit_factory::GetConstant()!"
                " requested=%d , should be 0-1", in_digihit->end);
        cerr << str << endl;
        throw JException(str);
    }

    const int the_cell = GetCalibIndex( in_digihit->module, in_digihit->layer, in_digihit->sector );

    if (in_digihit->end == DBCALGeometry::kUpstream) {
        // handle the upstream end
        return the_table.at(the_cell).first;
    } else {
        // handle the downstream end
        return the_table.at(the_cell).second;
    }

}
/*
   const double DBCALHit_factory::GetConstant(const bcal_digi_constants_t &the_table,
   const DTranslationTable *ttab,
   const int in_rocid,
   const int in_slot,
   const int in_channel) const
   {
   char str[256];

   DTranslationTable::csc_t daq_index = { in_rocid, in_slot, in_channel };
   DTranslationTable::DChannelInfo channel_info = ttab->GetDetectorIndex(daq_index);

   if ( (channel_info.bcal.module <= 0) 
   || (channel_info.bcal.module > static_cast<unsigned int>(BCAL_NUM_MODULES)))
   {
   sprintf(str, "Bad module # requested in DBCALHit_factory::GetConstant()!"
   " requested=%d , should be 1-%d", channel_info.bcal.module, BCAL_NUM_MODULES);
   cerr << str << endl;
   throw JException(str);
   }
   if ( (channel_info.bcal.layer <= 0) 
   || (channel_info.bcal.layer > static_cast<unsigned int>(BCAL_NUM_LAYERS)))
   {
   sprintf(str, "Bad layer # requested in DBCALHit_factory::GetConstant()!"
   " requested=%d , should be 1-%d", channel_info.bcal.layer, BCAL_NUM_LAYERS);
   cerr << str << endl;
   throw JException(str);
   }
   if ( (channel_info.bcal.sector <= 0) 
   || (channel_info.bcal.sector > static_cast<unsigned int>(BCAL_NUM_SECTORS)))
   {
   sprintf(str, "Bad sector # requested in DBCALHit_factory::GetConstant()!"
   " requested=%d , should be 1-%d", channel_info.bcal.sector, BCAL_NUM_SECTORS);
   cerr << str << endl;
   throw JException(str);
   }
   if ( (channel_info.bcal.end != DBCALGeometry::kUpstream) 
   && (channel_info.bcal.end != DBCALGeometry::kDownstream) )
   {
   sprintf(str, "Bad end # requested in DBCALHit_factory::GetConstant()!"
   " requested=%d , should be 0-1", channel_info.bcal.end);
   cerr << str << endl;
   throw JException(str);
   }

   int the_cell = DBCALGeometry::cellId(channel_info.bcal.module,
   channel_info.bcal.layer,
   channel_info.bcal.sector);

   if (channel_info.bcal.end == DBCALGeometry::kUpstream) {
// handle the upstream end
return the_table.at(the_cell).first;
} else {
// handle the downstream end
return the_table.at(the_cell).second;
}
}
*/
