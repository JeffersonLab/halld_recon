//
//  A simplified version of Sasha's DL1MCTrigger_factory 
//  with FCAL, FCAL+BCAL, and CPP TOF2 trigger bit finding included
//  Works for the data and MC
//  Jan 23, 2026 ilarin
//

#include "DL1MCTrigger_factory_CPP.h"

#include <JANA/JApplication.h>
#include <DANA/DEvent.h>
#include <JANA/JException.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#endif
#include "FCAL/DFCALGeometry.h"
#include "BCAL/DBCALGeometry.h"
#include "TOF/DTOFGeometry.h"
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#include <RCDB/Connection.h>
#pragma GCC diagnostic pop
#include "RCDB/ConfigParser.h"

#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALDigiHit.h>
#include <BCAL/DBCALHit.h>
#include <BCAL/DBCALDigiHit.h>
#include <TOF/DTOFHit.h>
#include <TOF/DTOFDigiHit.h>
#include <DRandom2.h>
#include "TTAB/DTranslationTable.h"
#include <DAQ/Df250WindowRawData.h>

#include <numeric>
#include <iostream>
#include <iomanip>

using namespace std;

void DL1MCTrigger_factory_CPP::Init() {
  auto app = GetApplication();

  FCAL_ADC_PER_MEV  = 3.73;
  app->SetDefaultParameter("TRIG:FCAL_ADC_PER_MEV", FCAL_ADC_PER_MEV, "FCAL energy calibration for the Trigger");
  FCAL_CELL_THR = 165;
  app->SetDefaultParameter("TRIG:FCAL_CELL_THR", FCAL_CELL_THR, "FCAL energy threshold per cell");

  FCAL_NSA  = 10;
  app->SetDefaultParameter("TRIG:FCAL_NSA", FCAL_NSA, "FCAL NSA");
  FCAL_NSB  =  3;
  app->SetDefaultParameter("TRIG:FCAL_NSB", FCAL_NSB, "FCAL NSB");
  FCAL_WINDOW =  10;
  app->SetDefaultParameter("TRIG:FCAL_WINDOW", FCAL_WINDOW, "FCAL GTP integration window");

  BCAL_ADC_PER_MEV =  34.48276;
  app->SetDefaultParameter("TRIG:BCAL_ADC_PER_MEV", BCAL_ADC_PER_MEV, "BCAL energy calibration for the Trigger");
  BCAL_CELL_THR =  120;
  app->SetDefaultParameter("TRIG:BCAL_CELL_THR", BCAL_CELL_THR, "BCAL energy threshold per cell");
  BCAL_NSA  = 19;
  app->SetDefaultParameter("TRIG:BCAL_NSA", BCAL_NSA, "BCAL NSA");
  BCAL_NSB  = 3;
  app->SetDefaultParameter("TRIG:BCAL_NSB", BCAL_NSB, "BCAL NSB");
  BCAL_WINDOW = 20;
  app->SetDefaultParameter("TRIG:BCAL_WINDOW", BCAL_WINDOW, "BCAL GTP integration window");

  BCAL_OFFSET = 2;
  app->SetDefaultParameter("TRIG:BCAL_OFFSET", BCAL_OFFSET, "Timing offset between BCAL and FCAL energies at GTP (samples)");

  PEDESTAL_SIGMA = 1.2;
  app->SetDefaultParameter("TRIG:PEDESTAL_SIGMA", PEDESTAL_SIGMA, "Pedestal sigma to emulate spread");

  USE_RAW_SAMPLES = false;
  app->SetDefaultParameter("TRIG:USE_RAW_SAMPLES", USE_RAW_SAMPLES, "Use waveforms from raw mode data, instead of emulating from hits");

  TOF_FE_MATCH_WINDOW   = 10;
  app->SetDefaultParameter("TRIG:TOF_FE_MATCH_WINDOW", TOF_FE_MATCH_WINDOW, "half window size to find front-end TOF coincidences");

  TOF_BAR_MATCH_WINDOW  = 10;
  app->SetDefaultParameter("TRIG:TOF_BAR_MATCH_WINDOW", TOF_BAR_MATCH_WINDOW, "half window size to find bar-to-bar end TOF coincidences");

  TOF_ACCEPT_MIN_SAMP  = 5;
  app->SetDefaultParameter("TRIG:TOF_ACCEPT_MIN_SAMP", TOF_ACCEPT_MIN_SAMP, "TOF signal search low limit");

  TOF_ACCEPT_MAX_SAMP  = 75;
  app->SetDefaultParameter("TRIG:TOF_ACCEPT_MAX_SAMP",TOF_ACCEPT_MAX_SAMP , "TOF signal search upper limit");

  TOF_PEAK_WINDOW_MIN  = 20;
  app->SetDefaultParameter("TRIG:TOF_PEAK_WINDOW_MIN", TOF_PEAK_WINDOW_MIN, "TOF signal peak position for coincidence low limit");

  TOF_PEAK_WINDOW_MAX  = 40;
  app->SetDefaultParameter("TRIG:TOF_PEAK_WINDOW_MAX", TOF_PEAK_WINDOW_MAX, "TOF signal peak position for coincidence upper limit");

  BCAL_ACCEPT_MIN_SAMP  = 15;
  app->SetDefaultParameter("TRIG:BCAL_ACCEPT_MIN_SAMP", BCAL_ACCEPT_MIN_SAMP, "BCAL trigger GTP sample low limit");

  BCAL_ACCEPT_MAX_SAMP  = 55;
  app->SetDefaultParameter("TRIG:BCAL_ACCEPT_MAX_SAMP", BCAL_ACCEPT_MAX_SAMP, "BCAL trigger GTP sample upper limit");

  FB_ACCEPT_MIN_SAMP    = 15;
  app->SetDefaultParameter("TRIG:FB_ACCEPT_MIN_SAMP", FB_ACCEPT_MIN_SAMP, "FCAL&BCAL trigger GTP sample low limit");

  FB_ACCEPT_MAX_SAMP    = 50;
  app->SetDefaultParameter("TRIG:FB_ACCEPT_MAX_SAMP", FB_ACCEPT_MAX_SAMP, "FCAL&BCAL trigger GTP sample upper limit");

  BCAL_PEAK_WINDOW_MIN  = 30;
  app->SetDefaultParameter("TRIG:BCAL_PEAK_WINDOW_MIN", BCAL_PEAK_WINDOW_MIN, "BCAL trigger min acceptable sample");

  BCAL_PEAK_WINDOW_MAX  = 70;
  app->SetDefaultParameter("TRIG:BCAL_PEAK_WINDOW_MAX", BCAL_PEAK_WINDOW_MAX, "BCAL trigger max acceptable sample");

//  preset other parameters defaults in case if they are missing in RCDB:

  TOF_READ_NSA            =  10;
  TOF_NSAT                =   2;
  TOF_CELL_TRIG_THR       = 400;
  NSAMPLES_INTEGRAL_FCAL  =  16;
  NSAMPLES_INTEGRAL_BCAL  =  27;
  NSAMPLES_INTEGRAL_TOF   =  11;
  TOF_CELL_THR            = 160;
  TOF_NSA                 =  15;
  TOF_NSB                 =   3;
  FCAL_BASELINE           = 100;
  BCAL_BASELINE           = 100;
  TOF_BASELINE            = 100;
  TOF_W_WIDTH             =  80;

}

void DL1MCTrigger_factory_CPP::BeginRun(const std::shared_ptr<const JEvent>& event) {

  fcal_trig_mask.clear();
  bcal_trig_mask.clear();
  triggers_enabled.clear();

  int print_level = 1;
  Read_RCDB(event, print_level);

  vector<const DFCALGeometry*> fcalGeomVect;
  event->Get(fcalGeomVect);
  if(fcalGeomVect.empty())
    throw JException("L1MCTrigger: DFCALGeometry not found (fcalGeomVect is empty)");

  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
  auto calibration = DEvent::GetJCalibration(event);

  vector< double > fcal_gains_ch;
  if(calibration->Get("/FCAL/gains", fcal_gains_ch))
    throw JException("L1MCTrigger: failed to load calibration path '/FCAL/gains'");

  fcal_gains.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, 1.0));
  for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
    fcal_gains[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_gains_ch[ch];

  vector< double > fcal_pedestals_ch;
  if(calibration->Get("/FCAL/pedestals", fcal_pedestals_ch))
    throw JException("L1MCTrigger: failed to load calibration path '/FCAL/pedestals'");

  fcal_pedestals.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, FCAL_BASELINE));
  for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
    fcal_pedestals[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_pedestals_ch[ch];

  vector< double > fcal_timing_offsets_ch;
  if(calibration->Get("/FCAL/timing_offsets", fcal_timing_offsets_ch))
    throw JException("L1MCTrigger: failed to load calibration path '/FCAL/timing_offsets'");

  fcal_timing_offsets.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, 0.0));
  for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
    fcal_timing_offsets[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_timing_offsets_ch[ch];

  vector< double > fcal_ADC_Offsets_ch;
  if(calibration->Get("/FCAL/ADC_Offsets", fcal_ADC_Offsets_ch))
    throw JException("L1MCTrigger: failed to load calibration path '/FCAL/ADC_Offsets'");

  fcal_ADC_Offsets.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, 0.0));
  for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
    fcal_ADC_Offsets[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_ADC_Offsets_ch[ch];

  map<string,double> fcal_base_time_offset;

  if(calibration->Get("/FCAL/base_time_offset",fcal_base_time_offset))
    throw JException("Error loading /FCAL/base_time_offset");

  if(fcal_base_time_offset.find("FCAL_BASE_TIME_OFFSET") != fcal_base_time_offset.end()) {
    fcal_t_base = fcal_base_time_offset["FCAL_BASE_TIME_OFFSET"];
  } else {throw JException("Error loading /FCAL/base_time_offset");}

  if(calibration->Get("/BCAL/ADC_gains", bcal_gains))
    throw JException("L1MCTrigger: failed to load calibration path '/BCAL/ADC_gains'");

  if(calibration->Get("/BCAL/ADC_pedestals", bcal_pedestals))
    throw JException("L1MCTrigger: failed to load calibration path '/BCAL/ADC_pedestals'");

// load scale factors
  map<string,double> scale_factors;
  if(calibration->Get("/FCAL/digi_scales", scale_factors))
    throw JException("Error loading /FCAL/digi_scales");

  if(scale_factors.find("FCAL_ADC_ASCALE") != scale_factors.end())
    fcal_a_scale = scale_factors["FCAL_ADC_ASCALE"];
  else
    throw JException("Unable to get FCAL_ADC_ASCALE from /FCAL/digi_scales");

  if(scale_factors.find("FCAL_ADC_TSCALE") != scale_factors.end())
    fcal_t_scale = scale_factors["FCAL_ADC_TSCALE"];
  else
    throw JException("Unable to get FCAL_ADC_TSCALE from /FCAL/digi_scales");

  map<string,double> bcal_base_time_offset;

  if(calibration->Get("/BCAL/base_time_offset",bcal_base_time_offset))
    throw JException("Error loading /BCAL/base_time_offset");

  if(bcal_base_time_offset.find("BCAL_BASE_TIME_OFFSET") != bcal_base_time_offset.end()) {
    bcal_t_base = bcal_base_time_offset["BCAL_BASE_TIME_OFFSET"];
  } else {throw JException("Error loading /BCAL/base_time_offset");}

  vector<const DTOFGeometry*> tofGeomVect;
  event->Get(tofGeomVect);
  if(tofGeomVect.size()<1) throw JException("TOFGeometry is not available");
  const DTOFGeometry& tofGeom = *(tofGeomVect[0]);
    
  TOF_NUM_PLANES = tofGeom.Get_NPlanes();
  TOF_NUM_BARS = tofGeom.Get_NBars();
  TOF_MAX_CHANNELS = TOF_NUM_PLANES*TOF_NUM_BARS*2;  // total number of bars * 2 ends
  if(TOF_NUM_PLANES!=2) throw JException("TOF_NUM_PLANES!=2");

  vector<double> tof_raw_adc_pedestals;
  vector<double> tof_raw_adc_offsets;
  vector<double> tof_raw_adc2E;

  map<string,double> tof_base_time_offset;
  string locTOFBaseTimeOffsetTable = tofGeom.Get_CCDB_DirectoryName() + "/base_time_offset";

  if(calibration->Get(locTOFBaseTimeOffsetTable.c_str(),tof_base_time_offset))  throw JException("Error loading TOFBaseTimeOffsetTable");

  if(tof_base_time_offset.find("TOF_BASE_TIME_OFFSET") != tof_base_time_offset.end()) {
    tof_t_base = tof_base_time_offset["TOF_BASE_TIME_OFFSET"];
  } else {throw JException("Unable to get TOF_BASE_TIME_OFFSET");}

  string locTOFPedestalsTable = tofGeom.Get_CCDB_DirectoryName() + "/pedestals";
  if(calibration->Get(locTOFPedestalsTable.c_str(),tof_raw_adc_pedestals)) throw JException("Error loading TOFPedestalsTable");

  string locTOFADCTimeOffetsTable = tofGeom.Get_CCDB_DirectoryName() + "/adc_timing_offsets";
  if(calibration->Get(locTOFADCTimeOffetsTable.c_str(), tof_raw_adc_offsets)) throw JException("Error loading TOFADCTimeOffetsTable");

  FillCalibTable(tof_adc_pedestals, tof_raw_adc_pedestals, tofGeom);
  FillCalibTable(tof_adc_time_offsets, tof_raw_adc_offsets, tofGeom);

  string locTOFADC2ETable = tofGeom.Get_CCDB_DirectoryName() + "/adc2E";
  if(calibration->Get(locTOFADC2ETable.c_str(), tof_raw_adc2E)) throw JException("Error loading TOFADC2ETable");
    
  tof_adc2E.resize(TOF_NUM_PLANES*TOF_NUM_BARS*2);
  for(size_t n=0; n<tof_raw_adc2E.size(); n++) tof_adc2E[n] = tof_raw_adc2E[n];

// debug printout:

  cout << "TOF_READ_NSA " << TOF_READ_NSA<< endl;
  cout << "TOF_NSAT " << TOF_NSAT<< endl;
  cout << " TOF_CELL_TRIG_THR " << TOF_CELL_TRIG_THR<< endl;
  cout << " NSAMPLES_INTEGRAL_FCAL " << NSAMPLES_INTEGRAL_FCAL<< endl;
  cout << " NSAMPLES_INTEGRAL_BCAL " << NSAMPLES_INTEGRAL_BCAL<< endl;
  cout << " NSAMPLES_INTEGRAL_TOF " << NSAMPLES_INTEGRAL_TOF<< endl;
  cout << " TOF_CELL_THR " << TOF_CELL_THR<< endl;
  cout << " TOF_NSA " << TOF_NSA<< endl;
  cout << "TOF_NSB " << TOF_NSB<< endl;
  cout << "TOF_BASELINE " << TOF_BASELINE<< endl;
  cout << "BCAL_BASELINE " << BCAL_BASELINE<< endl;
  cout << "FCAL_BASELINE " <<FCAL_BASELINE << endl;
  cout << "TOF_W_WIDTH " << TOF_W_WIDTH<< endl;

}

void DL1MCTrigger_factory_CPP::Process(const std::shared_ptr<const JEvent>& event) {

  vector<int> fcal_ssp(data_sample,0), fcal_gtp(data_sample,0),
              bcal_ssp(data_sample,0), bcal_gtp(data_sample,0);

  vector<FCAL_SIGNAL> fcal_signal_hits, fcal_merged_hits, fraw_signal_hits;
  vector<BCAL_SIGNAL> bcal_signal_hits, bcal_merged_hits, braw_signal_hits;
  vector<TOF_SIGNAL>   tof_signal_hits,  tof_merged_hits, traw_signal_hits;

  vector<const DFCALHit*>  fcal_hits;
  event->Get(fcal_hits);
  vector<const DBCALHit*>  bcal_hits;
  event->Get(bcal_hits);
  vector<const DTOFHit*>   tof_hits;
  event->Get(tof_hits);

//  FCAL Part

  double FCAL_HIT_EN_SUM = 0;

  for(size_t i = 0; i < fcal_hits.size(); ++i) {
    int row  = fcal_hits[i]->row;
    int col  = fcal_hits[i]->column;

    double t = fcal_hits[i]->t;
    double e = fcal_hits[i]->E;

    bool masked = any_of(fcal_trig_mask.begin(),fcal_trig_mask.end(), [row, col](const fcal_mod& mask) {
      return mask.row == row && mask.col == col;});

    if(masked) continue; // skip masked channel

    FCAL_HIT_EN_SUM += e;
    FCAL_SIGNAL fcal_signal(row,col);

    double E  = e / fcal_a_scale / fcal_gains[row][col];
    Emulate_Waveform(E, t, fcal_signal);
    fcal_signal_hits.push_back(fcal_signal);
  }

  vector<const DFCALDigiHit*> fcal_digihits;
  event->Get(fcal_digihits);

  if(USE_RAW_SAMPLES) {

    for(size_t i = 0; i < fcal_digihits.size(); ++i) {
      const DFCALDigiHit *digihit = fcal_digihits[i];

  	  int col     = digihit->column;
	    int row     = digihit->row;
      double ped  = fcal_pedestals[row][col];

      bool masked = any_of(fcal_trig_mask.begin(),fcal_trig_mask.end(),
        [row, col](const fcal_mod& mask) {
        return mask.row == row && mask.col == col;});
      if(masked) continue; // skip masked channel

      FCAL_SIGNAL fraw_signal(row,col);

			const Df250PulseData* pulse = nullptr;
      digihit->GetSingle(pulse);
      if(!pulse) continue;

      const Df250WindowRawData* wdata = nullptr;
      pulse->GetSingle(wdata);
      if(!wdata) continue;

			const vector<uint16_t> &s = wdata->samples;
			size_t ns = s.size();
      if(ns>data_sample) {throw JException("sample size exceeds maximum");}

      auto it = max_element(s.begin(), s.end());
      if(it!=s.end()) {
        if(*it>=4096) continue; //  skip overflow channel
      }

      for(size_t is = 0; is<ns; ++is)
        fraw_signal.adc[is]   = (double)s[is] - ped;

      bool this_rawhit_is_duplication_due_to_system_glitch = false;
      for(size_t ii = 0; ii < fraw_signal_hits.size(); ++ii) {
        auto& hh = fraw_signal_hits[ii];
        if( hh.row    == fraw_signal.row    &&
            hh.column == fraw_signal.column &&
            exact_equal(hh.adc, fraw_signal.adc) ) {
          this_rawhit_is_duplication_due_to_system_glitch = true;
          break;
        }
      }
      if(this_rawhit_is_duplication_due_to_system_glitch) continue;
      fraw_signal_hits.push_back(fraw_signal);
    }
  }

// Merge FCAL hits:

  auto &fcal_signal_hits_to_use = USE_RAW_SAMPLES ? fraw_signal_hits : fcal_signal_hits;

  for(size_t i = 0; i < fcal_signal_hits_to_use.size(); ++i) {
    auto& base_hit = fcal_signal_hits_to_use[i];
    if(base_hit.merged) continue;

    FCAL_SIGNAL merged_hit(base_hit.row, base_hit.column);
    merged_hit.adc = base_hit.adc;

    for(size_t j = i+1; j < fcal_signal_hits_to_use.size(); ++j) {
      auto& compare_hit = fcal_signal_hits_to_use[j];
      if(compare_hit.row == base_hit.row && compare_hit.column == base_hit.column) {
        compare_hit.merged = true;
        for(size_t k = 0; k < merged_hit.adc.size(); ++k)  merged_hit.adc[k] += compare_hit.adc[k];
      }
    }

    fcal_merged_hits.push_back(merged_hit);
  }

  FADC_SSP(fcal_merged_hits, fcal_ssp, 1);
  GTP(fcal_ssp, fcal_gtp, FCAL_WINDOW);

//  BCAL Part

  vector<const DBCALDigiHit*> bcal_digihits;
  event->Get(bcal_digihits);

  double BCAL_HIT_EN_SUM = 0;

  if(USE_RAW_SAMPLES) {
    for(size_t i = 0; i < bcal_digihits.size(); ++i) {
      const DBCALDigiHit *digihit = bcal_digihits[i];

  	  int module  = digihit->module;
	    int layer   = digihit->layer;
	    int sector  = digihit->sector;
  	  int end     = digihit->end;
      double ped  = digihit->pedestal/double(digihit->nsamples_pedestal);

      bool masked = any_of(bcal_trig_mask.begin(),bcal_trig_mask.end(),
        [module, layer, sector, end](const bcal_mod& mask) {
        return mask.module == module && mask.layer == layer && mask.sector == sector && mask.end == end;});
      if(masked) continue; // skip masked channel

      BCAL_SIGNAL braw_signal(module,layer,sector,end);

			const Df250PulseData* pulse = nullptr;
      digihit->GetSingle(pulse);
      if(!pulse) continue;

      const Df250WindowRawData* wdata = nullptr;
      pulse->GetSingle(wdata);
      if(!wdata) continue;

			const vector<uint16_t> &s = wdata->samples;
			size_t ns = s.size();
      if(ns>data_sample) {throw JException("sample size exceeds maximum");}

      auto it = max_element(s.begin(), s.end());
      if(it!=s.end()) {
        if(*it>=4096) continue;  // skip overflown channel
      }

      for(size_t is = 0; is<ns; ++is)
        braw_signal.adc[is]   = (double)s[is] - ped;

      bool this_rawhit_is_duplication_due_to_system_glitch = false;
      for(size_t ii = 0; ii < braw_signal_hits.size(); ++ii) {
        auto& hh = braw_signal_hits[ii];
        if( hh.module == braw_signal.module &&
            hh.layer  == braw_signal.layer  &&
            hh.sector == braw_signal.sector &&
            hh.end    == braw_signal.end    &&
            exact_equal(hh.adc, braw_signal.adc) ) {
          this_rawhit_is_duplication_due_to_system_glitch = true;
          break;
        }
      }
      if(this_rawhit_is_duplication_due_to_system_glitch) continue;
      braw_signal_hits.push_back(braw_signal);
    }
  }

  for(size_t i = 0; i < bcal_hits.size(); ++i) {

    double t    = bcal_hits[i]->t_raw;
    double e    = bcal_hits[i]->E;
	  int module  = bcal_hits[i]->module;
	  int layer   = bcal_hits[i]->layer;
	  int sector  = bcal_hits[i]->sector;
	  int end     = bcal_hits[i]->end;

    bool masked = any_of(bcal_trig_mask.begin(),bcal_trig_mask.end(),
      [module, layer, sector, end](const bcal_mod& mask) {
      return mask.module == module && mask.layer == layer && mask.sector == sector && mask.end == end;});

    if(masked) continue; // skip masked channel

    BCAL_HIT_EN_SUM += e;
    BCAL_SIGNAL bcal_signal(module,layer,sector,end);

    e /= bcal_gains[GetBCALCalibIndex(module,layer,sector,end)];  // e/gain = counts ped subtractued

    Emulate_Waveform(e, t, bcal_signal);
    bcal_signal_hits.push_back(bcal_signal);
  }

// Merge BCAL hits

  auto &bcal_signal_hits_to_use = USE_RAW_SAMPLES ? braw_signal_hits : bcal_signal_hits;

  for(size_t i = 0; i < bcal_signal_hits_to_use.size(); ++i) {
    auto& base_hit = bcal_signal_hits_to_use[i];
    if(base_hit.merged) continue;

    BCAL_SIGNAL merged_hit(base_hit.module, base_hit.layer, base_hit.sector, base_hit.end);
    merged_hit.adc = base_hit.adc;

    for(size_t j = i+1; j < bcal_signal_hits_to_use.size(); ++j) {
      auto& compare_hit = bcal_signal_hits_to_use[j];
      if(compare_hit.module == base_hit.module && compare_hit.layer == base_hit.layer &&
         compare_hit.sector == base_hit.sector && compare_hit.end   == base_hit.end) {
        compare_hit.merged = true;
        for(size_t k = 0; k < merged_hit.adc.size(); ++k)  merged_hit.adc[k] += compare_hit.adc[k];
      }
    }
    bcal_merged_hits.push_back(merged_hit);
  }

  FADC_SSP(bcal_merged_hits, bcal_ssp, 2);
  GTP(bcal_ssp, bcal_gtp, BCAL_WINDOW);


//  TOF Part

  vector<const DTOFDigiHit*> tof_digihits;
  event->Get(tof_digihits);

  if(USE_RAW_SAMPLES) {

    for(size_t i = 0; i < tof_digihits.size(); ++i) {
      const DTOFDigiHit *digihit = tof_digihits[i];

      int plane = digihit->plane;
      int bar   = digihit->bar;
      int end   = digihit->end;

			const Df250PulseData* pulse = nullptr;
      digihit->GetSingle(pulse);
      if(!pulse) continue;

      const Df250WindowRawData* wdata = nullptr;
      pulse->GetSingle(wdata);
      if(!wdata) continue;

			vector<uint16_t> s = wdata->samples;
			size_t ns = s.size();
      if(ns>data_sample) {throw JException("sample size exceeds maximum");}
      for(auto &x : s) x = (x>TOF_BASELINE) ? x-TOF_BASELINE : 0.;

      int first = -1, last = -1;
      if(auto it = find_if(s.begin(),s.end(),[&](auto x)   {return x>TOF_CELL_TRIG_THR-TOF_BASELINE;}); it != s.end())   first = distance(s.begin(),it);
      if(auto it = find_if(s.rbegin(),s.rend(),[&](auto x) {return x>TOF_CELL_TRIG_THR-TOF_BASELINE;}); it != s.rend())  last  = distance(s.begin(), it.base());
      if(first<TOF_ACCEPT_MIN_SAMP || first>TOF_ACCEPT_MAX_SAMP || last-first < 1) continue;

      TOF_SIGNAL traw_signal(plane,bar,end,first);

      double ped  = GetConstant(tof_adc_pedestals,plane,bar,end);
      for(size_t is = 0; is<ns; ++is)
        traw_signal.adc[is]   = (double)s[is] - ped;

      bool this_rawhit_is_duplication_due_to_system_glitch = false;
      for(size_t ii = 0; ii < traw_signal_hits.size(); ++ii) {
        auto& hh = traw_signal_hits[ii];
        if( hh.plane  == traw_signal.plane  &&
            hh.bar    == traw_signal.bar    &&
            hh.end    == traw_signal.end    &&
            exact_equal(hh.adc, traw_signal.adc) ) {
          this_rawhit_is_duplication_due_to_system_glitch = true;
          break;
        }
      }
      if(this_rawhit_is_duplication_due_to_system_glitch) continue;
      traw_signal_hits.push_back(traw_signal);

    }
  }

  for(size_t i = 0; i < tof_hits.size(); ++i) {
    int plane = tof_hits[i]->plane;
    int bar   = tof_hits[i]->bar;
    int end   = tof_hits[i]->end;
    int id    = 2*TOF_NUM_BARS*plane + TOF_NUM_BARS*end + bar - 1;

    double offset = GetConstant(tof_adc_time_offsets,plane,bar,end);
    double t      = (tof_hits[i]->t_fADC + offset - tof_t_base)/data_time_bin;
    int samp     = int(t);

    if(samp<TOF_PEAK_WINDOW_MIN || samp>TOF_PEAK_WINDOW_MAX) continue;
    double e  = tof_hits[i]->dE/tof_adc2E[id];

    TOF_SIGNAL tof_signal(plane,bar,end,samp);

    Emulate_Waveform(e, t, tof_signal);

    auto it = max_element(tof_signal.adc.begin(), tof_signal.adc.end());
    if(it==tof_signal.adc.end()) continue;
    double emulated_pulse_peak = *it;
    if(emulated_pulse_peak<TOF_CELL_TRIG_THR-TOF_BASELINE) continue;

    tof_signal_hits.push_back(tof_signal);

  }

  if(!USE_RAW_SAMPLES) Digitize(tof_signal_hits);

// Merge TOF hits:

  auto &tof_signal_hits_to_use = USE_RAW_SAMPLES ? traw_signal_hits : tof_signal_hits;
  auto coincidences = find_coincidences(tof_signal_hits_to_use,TOF_FE_MATCH_WINDOW);
  bool  emulated_tof_trigger = false;

  if(coincidences.size()>3)
  for(size_t isample = 0; isample < data_sample-TOF_BAR_MATCH_WINDOW; ++isample) {

    uint32_t bit0 = BuildTOFGroupBits(coincidences,0,isample,TOF_BAR_MATCH_WINDOW);
    uint32_t bit1 = BuildTOFGroupBits(coincidences,1,isample,TOF_BAR_MATCH_WINDOW);

    if(bit0*bit1>0 && popcount(bit0)+popcount(bit1)>3) {
      emulated_tof_trigger = true;
      break;
    }
  }

  auto* trigger = new DL1MCTrigger;

  trigger->trig_mask   = emulated_tof_trigger ? TOF_mask: 0;
  trigger->fcal_gtp    = 0;
  trigger->fcal_gtp_en = 0;
  trigger->bcal_gtp    = 0;
  trigger->bcal_gtp_en = 0;
  trigger->fcal_en     = FCAL_HIT_EN_SUM;
  trigger->bcal_en     = BCAL_HIT_EN_SUM;

  FindTriggers(*trigger,fcal_gtp,bcal_gtp);

  if(trigger->trig_mask) {
    Insert(trigger);
  } else {
    delete trigger;
  }

}

void DL1MCTrigger_factory_CPP::EndRun() {
    // cleanup per run if needed
}

void DL1MCTrigger_factory_CPP::Finish() {
}

//------------------
// Creates a pulse shape array from the given energy and time
//------------------
void DL1MCTrigger_factory_CPP::Emulate_Waveform(double energy, double time, FCAL_SIGNAL& hit) {

  float decay_constant  = 1.43;
  float toffset         = fcal_t_base + fcal_ADC_Offsets[hit.row][hit.column] - fcal_timing_offsets[hit.row][hit.column];
  float t0              = (time-toffset)/data_time_bin;


// Length (in data_samples) of the digitized pulse
  const int pulse_window = 20;

// Calculate starting data_sample index based on input time
  int first_data_sample_index = int(t0);
  int start_index = first_data_sample_index - 6;
  int   end_index = start_index + pulse_window;

// Check if time is out of ADC digitization range
  if(start_index >= int(data_sample)) return;

  if(start_index < 0)  start_index = 0;
  if(end_index >= int(data_sample)) end_index = data_sample - 1;

// Add shaped pulse to adc_en

  double sum = 0.;
  vector<double> waveform(data_sample, 0.0);
  for(int i = start_index; i <= end_index; ++i) {
    double adc_time = i - t0;  // Time since signal peak
    double x = adc_time;
    double a = decay_constant;
    double f = (x<0) ? exp(-x*x*a*a/2.) : exp(-0.3*x*a*tanh(0.2*0.3*x*a));
    waveform[i] = (f>0) ? f : 0;
    if(i<start_index+NSAMPLES_INTEGRAL_FCAL)  sum += f;
  }

  if(sum>0.)
  for(int i = start_index; i <= end_index; ++i) {
    double fr = waveform[i] * energy / sum;
    hit.adc[i]  += fr;  // add normilized signal to the existing amplitudes
  }

}

void DL1MCTrigger_factory_CPP::Emulate_Waveform(double energy, double time, BCAL_SIGNAL& hit) {

  float decay_constant    = 0.024;
  float toffset           = bcal_t_base+20;

  const int pulse_window  = 60;

// Calculate starting data_sample index based on input time
  int first_data_sample_index = int(floor((time-toffset) / data_time_bin));
  int start_index = first_data_sample_index + 1;
  int   end_index = start_index + pulse_window;

// Check if time is out of ADC digitization range
  if(start_index >= int(data_sample)) return;

  if(start_index < 0)  start_index = 0;
  if(end_index >= int(data_sample)) end_index = data_sample - 1;

// Add shaped pulse to adc_en

  double sum = 0.;
  vector<double> waveform(data_sample, 0.0);
  for(int i = start_index; i <= end_index; ++i) {
    double adc_time = i*data_time_bin - time;  // Time since signal peak
    double x = adc_time;
    double a = decay_constant;
    double f = (1.+erf(0.25*(x-29.*data_time_bin))) * x*exp(-x*a) * (1+0.75*exp(-5.e-4*(x-63*data_time_bin)*(x-63*data_time_bin)));
    waveform[i] = (f>0) ? f : 0;
    if(i<start_index+NSAMPLES_INTEGRAL_BCAL) sum += f;
  }

  if(sum>0.)
  for(int i = start_index; i <= end_index; ++i) {
    double fr = waveform[i] * energy / sum;
    hit.adc[i]  += fr;  // add normilized signal to the existing amplitudes
  }
}

void DL1MCTrigger_factory_CPP::Emulate_Waveform(double energy, double time, TOF_SIGNAL& hit) {

// Length (in data_samples) of the digitized pulse
  const int pulse_window = 60;

// Calculate starting data_sample index based on input time
  int first = static_cast<int>(floor(time));
  if(first<0) first = 0;
  if(first>=static_cast<int>(data_sample)) return;
 
  size_t first_data_sample_index = size_t(first);
  size_t start_index = (first_data_sample_index>6) ? (first_data_sample_index-6) : 0;
  size_t   end_index = start_index + pulse_window;

// Check if time is out of ADC digitization range
  if(start_index >= int(data_sample)) return;
  if(end_index >= int(data_sample)) end_index = data_sample - 1;

// Add shaped pulse to adc_en

  double sum = 0.;
  vector<double> waveform(data_sample, 0.0);
  for(size_t i = start_index; i <= end_index; ++i) {
    double adc_time = i - time;  // Time since signal peak
    double x = adc_time;
    double x0 = 0.95;
    double f = (x<x0) ? exp(0.3*(x0-x)*(x0+x)) : exp(3.3*(log((1+0.1*x0)/(1+0.1*x))));
    waveform[i] = (f>0) ? f : 0;
    size_t win_hi = min(first_data_sample_index+(size_t)NSAMPLES_INTEGRAL_TOF,data_sample);
    if(i>=first_data_sample_index && i<win_hi)  sum += f;
  }

  if(sum>0.)
  for(size_t i = start_index; i <= end_index; ++i) {
    double fr = waveform[i] * energy / sum;
    hit.adc[i]  += fr;  // add normilized signal to the existing amplitudes
  }

  int t0 = static_cast<int>(floor(time));
  int n  = static_cast<int>(hit.adc.size());

  int i1i = t0;
  if(i1i < 0) i1i = 0;
  if(i1i > n) i1i = n;

  int i2i = t0 + NSAMPLES_INTEGRAL_TOF;
  if(i2i < 0) i2i = 0;
  if(i2i > n) i2i = n;

  size_t i1 = static_cast<size_t>(i1i);
  size_t i2 = static_cast<size_t>(i2i);

  double s1 = accumulate(hit.adc.begin()+i1,hit.adc.begin()+i2,0.0);
  if(s1>0.) for(double& x : hit.adc) x *= energy/s1;

}

void DL1MCTrigger_factory_CPP::Digitize(vector<FCAL_SIGNAL>& hits) {
  for(auto& hit : hits) {
    double pedestal = fcal_pedestals[hit.row][hit.column];
    for(size_t samp = 0; samp < hit.adc.size(); ++samp) {
        hit.adc[samp] += pedestal + gRandom->Gaus(0.0, PEDESTAL_SIGMA);
        if(hit.adc[samp] > max_fadc) hit.adc[samp] = max_fadc;
        hit.adc[samp] -= FCAL_BASELINE;
    }
  }
}

void DL1MCTrigger_factory_CPP::Digitize(vector<BCAL_SIGNAL>& hits) {
  for(auto& hit : hits) {
    for(size_t samp = 0; samp < hit.adc.size(); ++samp) {
        hit.adc[samp] += 4.5*PEDESTAL_SIGMA - fabs(gRandom->Gaus(0.0, 5*PEDESTAL_SIGMA)) + BCAL_BASELINE; // don't see real BCAL pedestals in ccdb
        if(hit.adc[samp] > max_fadc) hit.adc[samp] = max_fadc;
        hit.adc[samp] -= BCAL_BASELINE;
    }
  }
}

void DL1MCTrigger_factory_CPP::Digitize(vector<TOF_SIGNAL>& hits) {
  for(auto& hit : hits) {
    double pedestal = GetConstant(tof_adc_pedestals,hit.plane,hit.bar,hit.end);
    for(size_t samp = 0; samp < hit.adc.size(); ++samp) {
        hit.adc[samp] += pedestal + gRandom->Gaus(0.0, PEDESTAL_SIGMA);
        if(hit.adc[samp] > max_fadc) hit.adc[samp] = max_fadc;
        hit.adc[samp] -= TOF_BASELINE;
    }
  }
}

template <typename T>
void DL1MCTrigger_factory_CPP::FADC_SSP(vector<T>& merged_hits, vector<int>& ssp, int det) {

  Digitize(merged_hits);

  int EN_THR, NSA, NSB;
  switch(det) {
    case 1:
      EN_THR = FCAL_CELL_THR-FCAL_BASELINE;
      NSA    = FCAL_NSA;
      NSB    = FCAL_NSB;
      break;
    case 2:
      EN_THR = BCAL_CELL_THR-BCAL_BASELINE;
      NSA    = BCAL_NSA;
      NSB    = BCAL_NSB;
      break;
    default:
      throw JException("FADC_SSP: Unknown detector type");
  }

  ssp.assign(data_sample,0);

  for(const auto& hit : merged_hits) {
    const vector<double>& adc = hit.adc;
    int sample_size = adc.size();
    int index_max = -1;
    for(int i = 0; i < sample_size; ++i) {
      if(adc[i]<EN_THR) continue;

      int index_min = i - NSB;
      if(index_min<0) index_min = 0;
      if(index_min<=index_max) index_min = index_max + 1;

      index_max = i + NSA - 1;
      if(index_max>=sample_size) index_max = sample_size - 1;

// Extend NSA window if signal continues above threshold
      while(index_max+1<sample_size && adc[index_max+1]>=EN_THR)  ++index_max;

      for(int j = index_min; j<=index_max; ++j) // Fill SSP buffer
        if(adc[j]>0) ssp[j] += adc[j];  // here the integration is over channels, not over time samples

      i = index_max; // Skip processed region
    }
  }

}

//------------------
//  GTP function computes a running sum over a sliding window of size integration_window on the input vector SSP, storing results in GTP
//------------------
//
void DL1MCTrigger_factory_CPP::GTP(const vector<int>& ssp, vector<int>& gtp, int window) {
  gtp.assign(data_sample, 0);
  size_t W = size_t(window);

  vector<int> prefix(ssp.size() + 1, 0);
  for (size_t i = 0; i < ssp.size(); ++i) prefix[i + 1] = prefix[i] + ssp[i];

  size_t n = std::min(ssp.size(), (size_t)data_sample);
  for (size_t i = 0; i < n; ++i) {
    // window of up to W samples ending at i
    size_t lo = (i + 1 >= W) ? i + 1 - W : 0;
    size_t hi = i;  // inclusive [lo .. hi], size = hi-lo+1 = W

    gtp[i] = prefix[hi + 1] - prefix[lo];
  }
}

void DL1MCTrigger_factory_CPP::FindTriggers(DL1MCTrigger &trigger, const vector<int>& fcal_gtp, const vector<int>& bcal_gtp) {

  int ntriggers_found = 0;
  int max_bcal_gtp  = 0, max_bcal_samp  = -1;

  fill_n(trigger.trig_time, 32, -1);

  for(size_t i = 0; i < triggers_enabled.size(); ++i) {
    const auto& trigger_config = triggers_enabled[i];

    if(trigger_config.type != 2 && trigger_config.type != 3) continue; // BCAL and F+BCAL triggers for now

    int en_bit = trigger_config.bit;

    bool trigger_condition_met = false;

    for(int samp = 0; samp < static_cast<int>(data_sample); ++samp) {
      int bcal_samp = samp - BCAL_OFFSET;
      int bcal_energy = (bcal_samp >= 0 && bcal_samp < static_cast<int>(data_sample)) ? bcal_gtp[bcal_samp] : 0;

      if(max_bcal_gtp<bcal_energy) {max_bcal_gtp = bcal_energy; max_bcal_samp = samp;}
      if(bcal_energy<trigger_config.gtp.bcal_min) continue;

      int gtp_energy = trigger_config.gtp.bcal * bcal_energy;

      size_t trig_time_ind = 2;         // 2 == BCAL slot
      if(trigger_config.type == 3) {
        if(fcal_gtp[samp] <= trigger_config.gtp.fcal_min) continue;
        gtp_energy += trigger_config.gtp.fcal * fcal_gtp[samp];
        trig_time_ind = 0;              // 0 == FCAL slot
        if(samp<FB_ACCEPT_MIN_SAMP   || samp>FB_ACCEPT_MAX_SAMP) continue;
      } else {
        if(samp<BCAL_ACCEPT_MIN_SAMP || samp>BCAL_ACCEPT_MAX_SAMP) continue;
      }

      if(gtp_energy<trigger_config.gtp.en_thr)  continue;

      if(!trigger_condition_met) {

        trigger.trig_mask |= (1u << en_bit);

        if(trigger.trig_time[trig_time_ind] == -1) {
          trigger.trig_time[trig_time_ind] = (int)samp;
        }

        trigger.fcal_gtp     = fcal_gtp[samp];
        trigger.fcal_gtp_en  = fcal_gtp[samp]  / FCAL_ADC_PER_MEV;
        trigger.bcal_gtp     = bcal_energy;
        trigger.bcal_gtp_en  = bcal_energy / BCAL_ADC_PER_MEV;

        ++ntriggers_found;
        trigger_condition_met = true;
        break; // Stop after first trigger condition met for this config
      }
    }
  }

  if(max_bcal_samp < BCAL_PEAK_WINDOW_MIN || max_bcal_samp > BCAL_PEAK_WINDOW_MAX) {
    for(const auto& trig_conf : triggers_enabled) {
      if(trig_conf.type == 2) {                       // BCAL-only
        trigger.trig_mask &= ~(1u << trig_conf.bit);  // no BCAL triggers outside of this window
      }
    }
  }

  return;
}

void DL1MCTrigger_factory_CPP::FillCalibTable(tof_digi_constants_t &table, vector<double> &raw_table, const DTOFGeometry &tofGeom) {
    char str[256];
    int channel = 0;
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
        throw JException(str);
    }
}

const double DL1MCTrigger_factory_CPP::GetConstant(const tof_digi_constants_t &the_table, const int in_plane, const int in_bar, const int in_end) const {

    char str[256];

    if( (in_plane != 0) && (in_plane != 1) ) {
        sprintf(str, "Bad module # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_plane, TOF_NUM_PLANES);
        throw JException(str);
    }
    if( (in_bar < 1) || (in_bar > TOF_NUM_BARS)) {
        sprintf(str, "Bad layer # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 1-%d", in_bar, TOF_NUM_BARS);
        throw JException(str);
    }
    if( (in_end != 0) && (in_end != 1) ) {
        sprintf(str, "Bad end # requested in DTOFHit_factory::GetConstant()! requested=%d , should be 0-1", in_end);
        throw JException(str);
    }

    return (in_end==0) ? the_table[in_plane][in_bar-1].first : the_table[in_plane][in_bar-1].second;
}

const int DL1MCTrigger_factory_CPP::GetTOFGroup(int bar, int end) {
  if(bar >= 1 && bar <= 19)   return 1;
  if(bar == 20)               return 2;
  if(bar == 21)               return 3;
  if(bar >= 22 && bar <= 23)  return (end == 0 ? 4 : 5);
  if(bar >= 24 && bar <= 25)  return (end == 0 ? 6 : 7);
  if(bar == 26)               return 8;
  if(bar == 27)               return 9;
  if(bar >= 28 && bar <= 46)  return 10;
  throw JException("GetTOFGroup Error");
}

uint32_t DL1MCTrigger_factory_CPP::BuildTOFGroupBits(const vector<TOF_FE_COINCIDENCE> & hits, int plane, size_t sample, int dt_max_samples) {
  uint32_t bits = 0;
  for(const auto& h : hits) {
    if(h.plane != plane) continue;
    if(abs(h.time - int(sample))>dt_max_samples) continue;
    bits |= (1u << (GetTOFGroup(h.bar, h.end) - 1));
  }
  return bits;
}

vector<DL1MCTrigger_factory_CPP::TOF_FE_COINCIDENCE> DL1MCTrigger_factory_CPP::find_coincidences(vector<TOF_SIGNAL>& hits, int window) {

  vector<TOF_FE_COINCIDENCE> coincidences;
  const size_t n = hits.size();

  for(size_t i = 0; i < n; ++i) {
    const TOF_SIGNAL& a = hits[i];
    if(a.bar>=22 && a.bar<=25)  {
      TOF_FE_COINCIDENCE c(&a,&a);
      coincidences.push_back(c);
      continue;
    }

    for(size_t j = i + 1; j < n; ++j) {
      const TOF_SIGNAL& b = hits[j];
      if(b.bar>=22 && b.bar<=25)  continue;

      if(a.plane != b.plane) continue;
      if(a.bar   != b.bar)   continue;
      if(a.end   == b.end)   continue;
      if(abs(a.time-b.time) > window) continue;

      TOF_FE_COINCIDENCE c(&a,&b);
      coincidences.push_back(c);
    }
  }
  return coincidences;
}

void DL1MCTrigger_factory_CPP::PrintTriggers() {

  cout << "\n\n ------------  Trigger Settings --------------- \n\n";

  cout << "----------- FCAL -----------\n\n";
  cout << "FCAL_CELL_THR  = " << setw(10) << FCAL_CELL_THR << endl;
  cout << "FCAL_NSA       = " << setw(10) << FCAL_NSA << endl;
  cout << "FCAL_NSB       = " << setw(10) << FCAL_NSB << endl;
  cout << "FCAL_WINDOW    = " << setw(10) << FCAL_WINDOW << endl;
  cout << "FCAL_BASELINE  = " << setw(10) << FCAL_BASELINE << endl;
  cout << "FCAL NSAMPLES  = " << setw(10) << NSAMPLES_INTEGRAL_FCAL << endl;

  cout << "\n----------- BCAL -----------\n\n";
  cout << "BCAL_CELL_THR  = " << setw(10) << BCAL_CELL_THR << endl;
  cout << "BCAL_NSA       = " << setw(10) << BCAL_NSA << endl;
  cout << "BCAL_NSB       = " << setw(10) << BCAL_NSB << endl;
  cout << "BCAL_WINDOW    = " << setw(10) << BCAL_WINDOW << endl;
  cout << "BCAL_BASELINE  = " << setw(10) << BCAL_BASELINE << endl;
  cout << "BCAL NSAMPLES  = " << setw(10) << NSAMPLES_INTEGRAL_BCAL << endl;

  cout << "\n----------- TOF  -----------\n\n";
  cout << "TOF_CELL_THR   = " << setw(10) << TOF_CELL_THR << endl;
  cout << "TOF_NSA        = " << setw(10) << TOF_NSA << endl;
  cout << "TOF_NSB        = " << setw(10) << TOF_NSB << endl;
  cout << "TOF_WINDOW     = " << setw(10) << TOF_WINDOW << endl;
  cout << "TOF_BASELINE   = " << setw(10) << TOF_BASELINE << endl;
  cout << "TOF NSAMPLES   = " << setw(10) << NSAMPLES_INTEGRAL_TOF << endl;

  cout << "\n\n";

  if (!triggers_enabled.empty()) {
    cout << left
         << setw(10) << "BIT"
         << setw(10) << "K FCAL"
         << setw(10) << "K BCAL"
         << setw(10) << "EN THR"
         << setw(10) << "NHIT"
         << setw(10) << "LANE"
         << setw(14) << "FCAL_EMIN"
         << setw(14) << "FCAL_EMAX"
         << setw(14) << "BCAL_EMIN"
         << setw(14) << "BCAL_EMAX"
         << "TYPE"
         << endl;
  }

  const vector<string> cttype = {"FCAL+BCAL","TOF","BCAL ONLY"};

  for(const auto& trig : triggers_enabled) {
    string detector = "BFCAL";  // All known types are BFCAL
    if (trig.type < 0x1 || trig.type > 0x5) {
      detector = "NONE";
      cout << " Unknown detector ===== " << trig.type << endl;
    }

    cout << left
         << setw(10) << trig.bit
         << setw(10) << trig.gtp.fcal
         << setw(10) << trig.gtp.bcal
         << setw(10) << trig.gtp.en_thr
         << setw(10) << ( (trig.bit==1) ? 4 : 0 )
         << setw(10) << trig.bit
         << setw(14) << trig.gtp.fcal_min
         << setw(14) << trig.gtp.fcal_max
         << setw(14) << trig.gtp.bcal_min
         << setw(14) << trig.gtp.bcal_max
         << ( (trig.bit>=0 && trig.bit<=2) ? cttype[trig.bit] : "Unknown" )
         << endl;
  }

  cout << "\n" << endl;
}

void DL1MCTrigger_factory_CPP::Read_RCDB(const shared_ptr<const JEvent>& event, int print_level) {

  vector<const DTranslationTable*> ttab;
  event->Get(ttab);
  int32_t runnumber = event->GetRunNumber();
  vector<string> SectionNames = {
        "TRIGGER", "GLOBAL", "FCAL", "BCAL", "TOF", "ST", "TAGH",
        "TAGM", "PS", "PSC", "TPOL", "CDC", "FDC"};

  const char* env_conn = getenv("RCDB_CONNECTION");
  string RCDB_CONNECTION = env_conn ? env_conn : "mysql://rcdb@hallddb.jlab.org/rcdb";
  rcdb::Connection connection(RCDB_CONNECTION);

  auto rtvsCnd = connection.GetCondition(runnumber, "rtvs");
  if(!rtvsCnd)  throw JException("Read_RCDB: rtvs is not set");

  auto json = rtvsCnd->ToJsonDocument();
  string fileName = json["%(config)"].GetString();
  auto file = connection.GetFile(runnumber, fileName);
  if(!file) throw JException("Read_RCDB: missed json file");

  string fileContent = file->GetContent();
  auto result = rcdb::ConfigParser::Parse(fileContent, SectionNames);

// FCAL
  const auto& FCALSection = result.Sections["FCAL"].NameValues;

  if(print_level > 1) {
    cout << "Keys in FCAL section:\n";
    for (const auto& kv : FCALSection) cout << "  " << kv.first << " = " << kv.second << "\n";
  }

  auto it = FCALSection.find("FADC250_TRIG_THR");
  if(it != FCALSection.end() && !it->second.empty())  FCAL_CELL_THR = max(0, stoi(it->second));

  it = FCALSection.find("FADC250_TRIG_NSB");
  if(it != FCALSection.end() && !it->second.empty())  FCAL_NSB = stoi(it->second);

  it = FCALSection.find("FADC250_TRIG_NSA");
  if(it != FCALSection.end() && !it->second.empty())  FCAL_NSA = stoi(it->second);

  it = FCALSection.find("FADC250_TRIG_BL");
  if(it != FCALSection.end() && !it->second.empty())  FCAL_BASELINE = stoi(it->second);

  int fcal_readout_nsb = -1, fcal_readout_nsa = -1;
  it = FCALSection.find("FADC250_NSB");
  if(it != FCALSection.end() && !it->second.empty())  fcal_readout_nsb = stoi(it->second);

  it = FCALSection.find("FADC250_NSA");
  if(it != FCALSection.end() && !it->second.empty())  fcal_readout_nsa = stoi(it->second);
  NSAMPLES_INTEGRAL_FCAL = fcal_readout_nsb + fcal_readout_nsa;
  if(fcal_readout_nsb<0 || fcal_readout_nsa<0) throw JException("undefined FCAL FADC250_NSA/FADC250_NSB");

// BCAL
  const auto& BCALSection = result.Sections["BCAL"].NameValues;

  if(print_level > 1) {
    cout << "Keys in BCAL section:\n";
    for (const auto& kv : BCALSection) cout << "  " << kv.first << " = " << kv.second << "\n";
  }

  it = BCALSection.find("FADC250_TRIG_THR");
  if(it != BCALSection.end() && !it->second.empty())  BCAL_CELL_THR = max(0, stoi(it->second));

  it = BCALSection.find("FADC250_TRIG_NSB");
  if(it != BCALSection.end() && !it->second.empty())  BCAL_NSB = stoi(it->second);

  it = BCALSection.find("FADC250_TRIG_NSA");
  if(it != BCALSection.end() && !it->second.empty())  BCAL_NSA = stoi(it->second);

  it = BCALSection.find("FADC250_TRIG_BL");
  if(it != BCALSection.end() && !it->second.empty())  BCAL_BASELINE = stoi(it->second);

  int bcal_readout_nsb = -1, bcal_readout_nsa = -1;
  it = BCALSection.find("FADC250_NSB");
  if(it != BCALSection.end() && !it->second.empty())  bcal_readout_nsb = stoi(it->second);

  it = BCALSection.find("FADC250_NSA");
  if(it != BCALSection.end() && !it->second.empty())  bcal_readout_nsa = stoi(it->second);
  NSAMPLES_INTEGRAL_BCAL = bcal_readout_nsb + bcal_readout_nsa;
  if(bcal_readout_nsb<0 || bcal_readout_nsa<0) throw JException("undefined BCAL FADC250_NSA/FADC250_NSB");

// TOF
  const auto& TOFSection = result.Sections["TOF"].NameValues;

  if(print_level > 1) {
    cout << "Keys in TOF section:\n";
    for (const auto& kv : TOFSection) cout << "  " << kv.first << " = " << kv.second << "\n";
  }

  it = TOFSection.find("FADC250_TRIG_THR");
  if(it != TOFSection.end() && !it->second.empty())  TOF_CELL_TRIG_THR = max(0, stoi(it->second));

  it = TOFSection.find("FADC250_TRIG_NSB");
  if(it != TOFSection.end() && !it->second.empty())  TOF_NSB = stoi(it->second);

  it = TOFSection.find("FADC250_TRIG_NSA");
  if(it != TOFSection.end() && !it->second.empty())  TOF_NSA = stoi(it->second);

  it = TOFSection.find("FADC250_NSB");
  if(it != TOFSection.end() && !it->second.empty())  TOF_READ_NSB = stoi(it->second);

  it = TOFSection.find("FADC250_NSA");
  if(it != TOFSection.end() && !it->second.empty())  TOF_READ_NSA = stoi(it->second);

  it = TOFSection.find("FADC250_NSAT");
  if(it != TOFSection.end() && !it->second.empty())  TOF_NSAT = stoi(it->second);

  it = TOFSection.find("FADC250_READ_THR");
  if(it != TOFSection.end() && !it->second.empty())  TOF_CELL_THR = stoi(it->second);

  it = TOFSection.find("FADC250_TRIG_BL");
  if(it != TOFSection.end() && !it->second.empty())  TOF_BASELINE = stoi(it->second);

  int  tof_readout_nsb = -1,  tof_readout_nsa = -1;
  it = TOFSection.find("FADC250_NSB");
  if(it != TOFSection.end() && !it->second.empty())  tof_readout_nsb = stoi(it->second);

  it = TOFSection.find("FADC250_NSA");
  if(it != TOFSection.end() && !it->second.empty())  tof_readout_nsa = stoi(it->second);
  NSAMPLES_INTEGRAL_TOF = tof_readout_nsb + tof_readout_nsa;
  if(tof_readout_nsb<0 || tof_readout_nsa<0) throw JException("undefined  TOF FADC250_NSA/FADC250_NSB");

  it = TOFSection.find("FADC250_W_WIDTH");
  if(it != TOFSection.end() && !it->second.empty())  TOF_W_WIDTH = stoi(it->second);

// Trigger types and equations
  vector<vector<string>> triggerTypes;
  for(const auto& row : result.Sections["TRIGGER"].Rows) {
    if(row.empty()) continue;
    if(row[0] == "TRIG_TYPE") {
      if(row.size() >= 9) {
        triggerTypes.push_back(row);
      } else {
        if(print_level>-1) cout << "Read_RCDB: skipping TRIG_TYPE line: not enough parameters N = " << row.size() << endl;
      }
    }
    if(row[0] == "TRIG_EQ" && row.size() >= 5) {
      if(stoi(row[4]) == 1) { // enabled
        if(row[1] == "FCAL")    {FCAL_WINDOW = stoi(row[3]); continue;}
        if(row[1] == "BCAL_E")  {BCAL_WINDOW = stoi(row[3]); continue;}
        if(row[1] == "TOF")     { TOF_WINDOW = stoi(row[3]); continue;}
      }
    }
  }

  triggers_enabled.clear();
  for(int bit = 0; bit<32; ++bit) {
    for(const auto& trigRow : triggerTypes) {
      if(trigRow.size()<9) continue;
      if(stoi(trigRow[8]) != bit) continue; // check if the trigger lane is enabled

      const string& trigType = trigRow[1];
      if(trigType != "BFCAL" && trigType != "TOF") {
        if(print_level>-1) cout << "Read_RCDB: skipping Trigger " << trigType << endl;
        continue;
      }

      trigger_conf trigger_tmp{};
      trigger_tmp.type  = 0;
      trigger_tmp.bit   = bit;

      int fcal = !trigRow[4].empty() ? stoi(trigRow[4]) : 0;
      int bcal = !trigRow[5].empty() ? stoi(trigRow[5]) : 0;

      if(!fcal && !bcal && trigType != "TOF") {
        if(print_level>-1) cout << "Read_RCDB: Incorrect parameters for BFCAL trigger" << endl;
        continue;
      }

      if(trigType != "TOF") {
        if(fcal)  trigger_tmp.type |= 0x1;
        if(bcal)  trigger_tmp.type |= 0x2;
        trigger_tmp.gtp.fcal = fcal;
        trigger_tmp.gtp.bcal = bcal;
      } else {
        trigger_tmp.type = 5;
      }

      if(!trigRow[6].empty()) trigger_tmp.gtp.en_thr = stoi(trigRow[6]);
      if(trigRow.size()>9  && !trigRow[9].empty()  && fcal) trigger_tmp.gtp.fcal_min = stoi(trigRow[9]);
      if(trigRow.size()>10 && !trigRow[10].empty() && fcal) trigger_tmp.gtp.fcal_max = stoi(trigRow[10]);
      if(trigRow.size()>11 && !trigRow[11].empty() && bcal) trigger_tmp.gtp.bcal_min = stoi(trigRow[11]);
      if(trigRow.size()>12 && !trigRow[12].empty() && bcal) trigger_tmp.gtp.bcal_max = stoi(trigRow[12]);

      triggers_enabled.push_back(trigger_tmp);
    }
  }

  for(const string det : {"FCAL", "BCAL"}) {

    const auto& section = result.Sections[det].NameValues;

    string userDir;
    it = section.find("FADC250_USER_DIR");
    if (it != section.end()) userDir = it->second;
    else { if(print_level>-1) cout << "\n\nMissing FADC250_USER_DIR in " << det << " section\n" << endl; continue; }

    string userVer;
    it = section.find("FADC250_USER_VER");
    if (it != section.end()) userVer = it->second;
    else { if(print_level>-1) cout << "\n\nMissing FADC250_USER_VER in " << det << " section\n" << endl; continue; }

    for(int crate = 1; crate <= 12; ++crate) {

      if(det == "BCAL" && (crate == 3 || crate == 6 || crate == 9 || crate == 12))  continue; // Skip these crates for BCAL

      string userFileName = userDir + "/" + (det == "FCAL" ? "rocfcal" : "rocbcal") + to_string(crate) + "_" + userVer + ".cnf";

      if(print_level>1)
        cout << "crate " << crate << " userFileName = " << userFileName << endl;

      auto userFile = connection.GetFile(runnumber, userFileName);
      if(!userFile) continue;

      auto userParseResult = rcdb::ConfigParser::ParseWithSlots(userFile->GetContent(), "FADC250_SLOTS");

      for(unsigned int slot = 3; slot<=21; ++slot) {
        auto userValues = userParseResult.SectionsBySlotNumber[slot].NameVectors["FADC250_TRG_MASK"];
        if(userValues.empty()) continue;

        for(unsigned int ch = 0; ch<userValues.size(); ++ch) {
          if(userValues[ch].empty()) continue;
          if(stoi(userValues[ch])<=0) continue;

          uint32_t roc_id = (det == "FCAL" ? 10 : 30) + crate;
          DTranslationTable::csc_t daq_index = {roc_id, slot, ch};
          DTranslationTable::DChannelInfo channel_info;

          try {channel_info = ttab[0]->GetDetectorIndex(daq_index);} catch (...) {
            if(print_level>-1)
              cout << "Exception: " << det << " channel is not in the translation table. "
                   << "Crate = " << crate << " Slot = " << slot << " Channel = " << ch << endl;
            continue;
          }

          if(det == "FCAL") {
            fcal_mod tmp;
            tmp.roc = crate; tmp.slot = slot; tmp.ch = ch;
            tmp.row = channel_info.fcal.row; tmp.col = channel_info.fcal.col;
            fcal_trig_mask.push_back(tmp);
            if(print_level>1)  cout << "MASKED FCAL CHANNEL " << tmp.row << " " << tmp.col << endl;

          }
          if(det == "BCAL") {
            bcal_mod tmp;
            tmp.roc = crate; tmp.slot = slot; tmp.ch = ch;
            tmp.module = channel_info.bcal.module; tmp.layer = channel_info.bcal.layer;
            tmp.sector = channel_info.bcal.sector; tmp.end = channel_info.bcal.end;
            bcal_trig_mask.push_back(tmp);

            if(print_level>1)  cout << "MASKED BCAL CHANNEL "
                                    << "Module = " << tmp.module << ", Layer = " << tmp.layer
                                    << ", Sector = " << tmp.sector << ", End = " << tmp.end << endl;
          }
        }
      }
    }
  }

  if(print_level>0) {
    cout << "NUMBER OF MASKED FCAL CHANNELS = " << fcal_trig_mask.size()
              << ", BCAL CHANNELS = " << bcal_trig_mask.size() << endl;
    PrintTriggers();
  }

}
