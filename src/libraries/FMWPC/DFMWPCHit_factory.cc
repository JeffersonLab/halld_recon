// $Id$
//
//    File: DFMWPCHit_factory.cc
//


#include <iostream>
#include <iomanip>
using namespace std;


#include <FMWPC/DFMWPCHit_factory.h>
#include <DAQ/Df125PulseIntegral.h>
#include <DAQ/Df125Config.h>
#include <DAQ/Df125CDCPulse.h>

using namespace jana;

//#define ENABLE_UPSAMPLING

//------------------
// init
//------------------
jerror_t DFMWPCHit_factory::init(void)
{
  
  hit_threshold = 0.;

  t_raw_min = -10000.;
  t_raw_max = 10000.;

  // default values
  a_scale = 0.;
  t_scale = 0.;
  t_base = 0.;
  
  // Set default number of number of detector channels
  maxChannels = 6*144;
  // hardcode some geometry constants - should pull from DGeometry later on
  Nlayers = 6;
  Nwires = {144, 144, 144, 144, 144, 144};

  
  /// set the base conversion scales
  a_scale = 4.0E3/1.0E2; 
  amp_a_scale = a_scale*28.8;
  t_scale = 8.0/10.0;    // 8 ns/count and integer time is in 1/10th of sample
  t_base  = 0.;       // ns
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DFMWPCHit_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
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
  
  // Read in calibration constants

  if(print_messages) jout << "In DFMWPCHit_factory, loading constants..." << std::endl;

  if (eventLoop->GetCalib("/FMWPC/hit_threshold", hit_threshold)){
    hit_threshold = 0.;
    jout << "Error loading /FMWPC/hit_threshold ! set default value to 0." << endl;
  } else {
    jout << "FMWPC Hit Threshold: " << hit_threshold << endl;
  }

  gPARMS->SetDefaultParameter("FMWPC:FMWPC_HIT_THRESHOLD", hit_threshold,
                              "Remove FMWPC Hits with peak amplitudes smaller than FMWPC_HIT_THRESHOLD");

  vector<double> fmwpc_timing_cuts;

  if (eventLoop->GetCalib("/FMWPC/timing_cut", fmwpc_timing_cuts)){
    t_raw_min = -60.;
    t_raw_max = 900.;
    jout << "Error loading /FMWPC/timing_cut ! set default values -60. and 900." << endl;
  } else {
    t_raw_min = fmwpc_timing_cuts[0];
    t_raw_max = fmwpc_timing_cuts[1];
    jout << "FMWPC Timing Cuts: " << t_raw_min << " ... " << t_raw_max << endl;
  }

  gPARMS->SetDefaultParameter("FMWPCHit:t_raw_min", t_raw_min,"Minimum acceptable FMWPC hit time");
  gPARMS->SetDefaultParameter("FMWPCHit:t_raw_max", t_raw_max, "Maximum acceptable FMWPC hit time");

  
  // load scale factors
  map<string,double> scale_factors;
  if (eventLoop->GetCalib("/FMWPC/digi_scales", scale_factors))
    jout << "Error loading /FMWPC/digi_scales !" << endl;
  if (scale_factors.find("FMWPC_ADC_ASCALE") != scale_factors.end())
    a_scale = scale_factors["FMWPC_ADC_ASCALE"];
  else
    jerr << "Unable to get FMWPC_ADC_ASCALE from /FMWPC/digi_scales !" << endl;
  amp_a_scale=a_scale*28.8;
  
#ifdef ENABLE_UPSAMPLING
  //t_scale=1.;
#else
  if (scale_factors.find("FMWPC_ADC_TSCALE") != scale_factors.end())
    t_scale = scale_factors["FMWPC_ADC_TSCALE"];
  else
    jerr << "Unable to get FMWPC_ADC_TSCALE from /FMWPC/digi_scales !" << endl;
#endif
  
  // load base time offset
  map<string,double> base_time_offset;
  if (eventLoop->GetCalib("/FMWPC/base_time_offset",base_time_offset))
    jout << "Error loading /FMWPC/base_time_offset !" << endl;
  if (base_time_offset.find("FMWPC_BASE_TIME_OFFSET") != base_time_offset.end())
    t_base = base_time_offset["FMWPC_BASE_TIME_OFFSET"];
  else
    jerr << "Unable to get FMWPC_BASE_TIME_OFFSET from /FMWPC/base_time_offset !" << endl;
  
  // load constant tables
  if (eventLoop->GetCalib("/FMWPC/wire_gains", gains))
    jout << "Error loading /FMWPC/wire_gains !" << endl;
  if (eventLoop->GetCalib("/FMWPC/pedestals", pedestals))
    jout << "Error loading /FMWPC/pedestals !" << endl;
  if (eventLoop->GetCalib("/FMWPC/timing_offsets", time_offsets))
    jout << "Error loading /FMWPC/timing_offsets !" << endl;
  

  // Verify that the right number of chambers was read for each set of constants
  char str[256];
  if (gains.size() != Nlayers) {
    sprintf(str, "Bad # of layers for FMWPC gain from CCDB! CCDB=%zu , should be %d", gains.size(), Nlayers);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  if (pedestals.size() != Nlayers) {
    sprintf(str, "Bad # of rings for FMWPC pedestal from CCDB! CCDB=%zu , should be %d", pedestals.size(), Nlayers);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  if (time_offsets.size() != Nlayers) {
    sprintf(str, "Bad # of rings for FMWPC time offset from CCDB!"
	    " CCDB=%zu , should be %d", time_offsets.size(), Nlayers);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  
  // Verify the right number of wires was read for each layer for each set of constants
  for (unsigned int i=0; i < Nlayers; i++) {
    if (gains[i].size() != Nwires[i]) {
      sprintf(str, "Bad # of wires for FMWPC gain from CCDB!"
	      " CCDB=%zu , should be %d for ring %d",
	      gains[i].size(), Nwires[i], i+1);
      std::cerr << str << std::endl;
      throw JException(str);
    }
    if (pedestals[i].size() != Nwires[i]) {
      sprintf(str, "Bad # of wires for FMWPC pedestal from CCDB!"
	      " CCDB=%zu , should be %d for ring %d",
	      pedestals[i].size(), Nwires[i], i+1);
      std::cerr << str << std::endl;
      throw JException(str);
    }
    if (time_offsets[i].size() != Nwires[i]) {
      sprintf(str, "Bad # of wires for FMWPC time offset from CCDB!"
	      " CCDB=%zu , should be %d for ring %d",
	      time_offsets[i].size(), Nwires[i], i+1);
      std::cerr << str << std::endl;
      throw JException(str);
    }
  }

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DFMWPCHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  /// Generate DCDCHit object for each DCDCDigiHit object.
  /// This is where the first set of calibration constants
  /// is applied to convert from digitzed units into natural
  /// units.
  ///
  /// Note that this code does NOT get called for simulated
  /// data in HDDM format. The HDDM event source will copy
  /// the precalibrated values directly into the _data vector.
  
  /// In order to use the new Flash125 data types and maintain compatibility with the old code, what is below is a bit of a mess

  vector<const DFMWPCDigiHit*> digihits;
  loop->Get(digihits);

  char str[256];
  for (unsigned int i=0; i < digihits.size(); i++) {
    const DFMWPCDigiHit *digihit = digihits[i];

    //if ( (digihit->QF & 0x1) != 0 ) continue; // Cut bad timing quality factor hits... (should check effect on efficiency)
    //if ( digihit->QF != 0 ) continue; // Cut bad timing quality factor hits... (should check effect on efficiency)
    
    const int &layer  = digihit->layer;
    const int &wire = digihit->wire;

    // Make sure layer and wire are in valid range
    if ( (layer < 1) || (layer > (int)Nlayers)) {
      sprintf(str, "DFMWPCDigiHit layer out of range!"
	      " layer=%d (should be 1-%d)", layer, Nlayers);
      throw JException(str);
    }
    if ( (wire < 1) || (wire > (int)Nwires[layer-1])) {
      sprintf(str, "DFMWPCDigiHit wire out of range!"
	      " wire=%d for layer=%d (should be 1-%d)",
	      wire, layer, Nwires[layer-1]);
      throw JException(str);
    }

    // Grab the pedestal from the digihit since this should be consistent between the old and new formats
    int raw_ped           = digihit->pedestal;
    int maxamp            = digihit->pulse_peak;
    // int nsamples_integral = 0; // actual number computed below using config info
    
    // There are a few values from the new data type that are critical for the interpretation of the data
    // uint16_t IBIT = 0; // 2^{IBIT} Scale factor for integral
    // uint16_t ABIT = 0; // 2^{ABIT} Scale factor for amplitude
    // uint16_t PBIT = 0; // 2^{PBIT} Scale factor for pedestal
    // uint16_t NW   = 0;
    
    /* COMMENTED OUT TO WRITE UNCALIBRATED HITS TO REST
	// Configuration data needed to interpret the hits is stored in the data stream
	vector<const Df125Config*> configs;
	digihit->Get(configs);
	if( configs.empty() ) {
		static int Nwarnings = 0;
		if(Nwarnings<10) {
			_DBG_ << "NO Df125Config object associated with Df125CDCPulse object!" << endl;
			Nwarnings++;
			if(Nwarnings==10) _DBG_ << " --- LAST WARNING!! ---" << endl;
		}
	} else {
		// Set some constants to defaults until they appear correctly in the config words in the future
		const Df125Config *config = configs[0];
		// IBIT = config->IBIT == 0xffff ? 4 : config->IBIT;
		ABIT = config->ABIT == 0xffff ? 3 : config->ABIT;
		PBIT = config->PBIT == 0xffff ? 0 : config->PBIT;
		// NW   = config->NW   == 0xffff ? 200 : config->NW;
	}

	// nsamples_integral = (NW - (digihit->pulse_time / 10));      
    
    // Complete the pedestal subtraction here since we should know the correct number of samples.
    int scaled_ped = raw_ped << PBIT;
    
    if (maxamp > 0) maxamp = maxamp << ABIT;
    if (maxamp <= scaled_ped) continue;
    
    maxamp = maxamp - scaled_ped;
    */
    
    if (maxamp<hit_threshold) {
      continue;
    }

    // Apply calibration constants here
    double t_raw = double(digihit->pulse_time);
    if (t_raw <= t_raw_min || t_raw >= t_raw_max)
      continue;
    
    // Scale factor to account for gain variation
    unsigned int layer_i=layer-1;
    unsigned int wire_i=wire-1;
    double gain=gains[layer_i][wire_i];
 
    // Charge and amplitude 
    // if ((digihit->pulse_integral<<IBIT) < scaled_ped*nsamples_integral)
    //   continue;
    // double q = a_scale *gain * double((digihit->pulse_integral<<IBIT)
    // 				      - scaled_ped*nsamples_integral);
    /* LEAVE AS IS
    double q = amp_a_scale*gain*double(maxamp);
    double amp = amp_a_scale*gain*double(maxamp);
    */
    double q = gain*double(digihit->pulse_integral);
    double amp = gain*double(maxamp);
    double ped = double(raw_ped);
    
    double t = t_scale * t_raw - time_offsets[layer_i][wire_i] + t_base;
    
    DFMWPCHit *hit = new DFMWPCHit;
    hit->layer  = layer;
    hit->wire = wire;
    
    // Values for d, itrack, ptype only apply to MC data
    // note that wire counting starts at 1
    hit->q = q;
    hit->amp = amp;
    hit->t = t;
    // hit->d = 0.0;
    hit->QF = digihit->QF;
    hit->ped = ped;
    // hit->itrack = -1;
    // hit->ptype = 0;

    hit->AddAssociatedObject(digihit);

    _data.push_back(hit);
    
  }
  
  return NOERROR;
}


//------------------
// erun
//------------------
jerror_t DFMWPCHit_factory::erun(void)
{
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DFMWPCHit_factory::fini(void)
{
  return NOERROR;
}

//------------------------------------
// GetConstant
//   Allow a few different interfaces
//------------------------------------
const double DFMWPCHit_factory::GetConstant(const fmwpc_digi_constants_t &the_table,
						const int in_layer, const int in_wire) const {
  
  char str[256];
  
  if ( (in_layer <= 0) || (static_cast<unsigned int>(in_layer) > Nlayers)) {
    sprintf(str, "Bad layer # requested in DFMWPCHit_factory::GetConstant()!"
	    " requested=%d , should be %ud", in_layer, Nlayers);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  if ( (in_wire <= 0) || 
       (static_cast<unsigned int>(in_wire) > Nwires[in_layer]))
    {
      sprintf(str, "Bad wire # requested in DCDCHit_factory::GetConstant()!"
	      " requested=%d , should be %ud", in_wire, Nwires[in_layer]);
      std::cerr << str << std::endl;
      throw JException(str);
    }
  
  return the_table[in_layer][in_wire]; // depends on numbering
}

const double DFMWPCHit_factory::GetConstant(const fmwpc_digi_constants_t &the_table,
						const DFMWPCDigiHit *in_digihit) const {
  
  char str[256];
  
  if ( (in_digihit->layer <= 0) || 
       (static_cast<unsigned int>(in_digihit->layer) > Nlayers))
    {
      sprintf(str, "Bad layer # requested in DFMWPCHit_factory::GetConstant()!"
	      " requested=%d , should be %ud", in_digihit->layer, Nlayers);
      std::cerr << str << std::endl;
      throw JException(str);
    }
  if ( (in_digihit->wire <= 0) || 
       (static_cast<unsigned int>(in_digihit->wire) > Nwires[in_digihit->layer]))
    {
      sprintf(str, "Bad wire # requested in DFMWPCHit_factory::GetConstant()!"
	      " requested=%d , should be %ud",
	      in_digihit->wire, Nwires[in_digihit->layer]);
      std::cerr << str << std::endl;
      throw JException(str);
    }
  
  return the_table[in_digihit->layer][in_digihit->wire];  // depends on numbering
}

const double DFMWPCHit_factory::GetConstant(const fmwpc_digi_constants_t &the_table,
						const DFMWPCHit *in_hit) const {
  
  char str[256];
  
  if ( (in_hit->layer <= 0) || (static_cast<unsigned int>(in_hit->layer) > Nlayers)) {
    sprintf(str, "Bad layer # requested in DFMWPCHit_factory::GetConstant()!"
	    " requested=%d , should be %ud", in_hit->layer, Nlayers);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  if ( (in_hit->wire <= 0) || 
       (static_cast<unsigned int>(in_hit->wire) > Nwires[in_hit->layer]) ) {
    sprintf(str, "Bad wire # requested in DFMWPCHit_factory::GetConstant()!"
	    " requested=%d , should be %ud",
	    in_hit->wire, Nwires[in_hit->layer]);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  
  return the_table[in_hit->layer][in_hit->wire];  // depends on numbering
}

