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

static int FMWPC_HIT_THRESHOLD = 0;

//#define ENABLE_UPSAMPLING

//------------------
// init
//------------------
jerror_t DFMWPCHit_factory::init(void)
{
  
  gPARMS->SetDefaultParameter("FMWPC:FMWPC_HIT_THRESHOLD", FMWPC_HIT_THRESHOLD,
                              "Remove FMWPC Hits with peak amplitudes smaller than FMWPC_HIT_THRESHOLD");
  
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
  
  // load geometry from XML here!!
	  
  //vector<double> raw_gains;
  //vector<double> raw_pedestals;
  //vector<double> raw_time_offsets;
  
  if(print_messages) jout << "In DFMWPCHit_factory, loading constants..." << std::endl;
  
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
  
  // fill the tables
  //FillCalibTable(gains, raw_gains);
  //FillCalibTable(pedestals, raw_pedestals);
  //FillCalibTable(time_offsets, raw_time_offsets);


  /* -- DISABLE THESE FOR NOW
  // Verify that the right number of rings was read for each set of constants
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
  -- */

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
    
    const int &layer  = digihit->layer;
    const int &wire = digihit->wire;

    /* fill in sanity check when you decided the detector numbering
    // Make sure ring and straw are in valid range
    if ( (ring < 1) || (ring > (int)Nrings)) {
      sprintf(str, "DCDCDigiHit ring out of range!"
	      " ring=%d (should be 1-%d)", ring, Nrings);
      throw JException(str);
    }
    if ( (straw < 1) || (straw > (int)Nstraws[ring-1])) {
      sprintf(str, "DCDCDigiHit straw out of range!"
	      " straw=%d for ring=%d (should be 1-%d)",
	      straw, ring, Nstraws[ring-1]);
      throw JException(str);
    }
	*/

    // Grab the pedestal from the digihit since this should be consistent between the old and new formats
    int raw_ped           = digihit->pedestal;
    int maxamp            = digihit->pulse_peak;
    int nsamples_integral = 0; // actual number computed below using config info
    
    // There are a few values from the new data type that are critical for the interpretation of the data
    uint16_t IBIT = 0; // 2^{IBIT} Scale factor for integral
    uint16_t ABIT = 0; // 2^{ABIT} Scale factor for amplitude
    uint16_t PBIT = 0; // 2^{PBIT} Scale factor for pedestal
    uint16_t NW   = 0;
    
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
		IBIT = config->IBIT == 0xffff ? 4 : config->IBIT;
		ABIT = config->ABIT == 0xffff ? 3 : config->ABIT;
		PBIT = config->PBIT == 0xffff ? 0 : config->PBIT;
		NW   = config->NW   == 0xffff ? 180 : config->NW;
	}

	if(NW==0) NW=180; // some data was taken (<=run 4700) where NW was written as 0 to file

	// The integration window in the CDC should always extend past the end 
	//of the window
	// Only true after about run 4100
	nsamples_integral = (NW - (digihit->pulse_time / 10));      
    
    // Complete the pedestal subtraction here since we should know the correct number of samples.
    int scaled_ped = raw_ped << PBIT;
    
    if (maxamp > 0) maxamp = maxamp << ABIT;
    //if (maxamp <= scaled_ped) continue;
    
    maxamp = maxamp - scaled_ped;
    
    // if (maxamp<FMWPC_HIT_THRESHOLD) {
    //   continue;
    // }
    
    // Apply calibration constants here
    double t_raw = double(digihit->pulse_time);
    
    // Scale factor to account for gain variation
    //double gain=gains[layer][wire];   // REMOVE CHANNEL-SPECIFIC CORRECTIONS
    double gain=1.;
 
    // Charge and amplitude 
    //double q = a_scale *gain * double((digihit->pulse_integral<<IBIT)
    //               - scaled_ped*nsamples_integral);
    double q = digihit->pulse_integral;
    double amp = amp_a_scale*gain*double(maxamp);
    
    //double t = t_scale * t_raw - time_offsets[layer][wire] + t_base;
    double t = t_scale * t_raw /*- time_offsets[layer][wire]*/ + t_base;   // REMOVE CHANNEL-SPECIFIC CORRECTIONS
    
    DFMWPCHit *hit = new DFMWPCHit;
    hit->layer  = layer;
    hit->wire = wire;
    
    // Values for d, itrack, ptype only apply to MC data
    // note that wire counting starts at 1
    hit->q = q;
    hit->amp = amp;
    hit->t = t;
    // hit->d = 0.0;
    // hit->QF = digihit->QF;
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

/* -- shouldn't need this if you make the CCDB tables 2D tables
//------------------
// FillCalibTable
//------------------
void DFMWPCHit_factory::FillCalibTable(vector< vector<double> > &table, vector<double> &raw_table)
{
  int ring = 0;
  int straw = 0;
  
  // reset table before filling it
  table.clear();
  table.resize( Nstraws.size() );
  
  for (unsigned int channel=0; channel<raw_table.size(); channel++,straw++) {
    // make sure that we don't try to load info for channels that don't exist
    if (channel == maxChannels) break;
    
    // if we've hit the end of the ring, move on to the next
    if (straw == (int)Nstraws[ring]) {
      ring++;
      straw = 0;
    }
    
    table[ring].push_back( raw_table[channel] );
  }
  
}
*/

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
    sprintf(str, "Bad ring # requested in DFMWPCHit_factory::GetConstant()!"
	    " requested=%d , should be %ud", in_hit->layer, Nlayers);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  if ( (in_hit->wire <= 0) || 
       (static_cast<unsigned int>(in_hit->wire) > Nwires[in_hit->layer]) ) {
    sprintf(str, "Bad straw # requested in DFMWPCHit_factory::GetConstant()!"
	    " requested=%d , should be %ud",
	    in_hit->wire, Nwires[in_hit->layer]);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  
  return the_table[in_hit->layer][in_hit->wire];  // depends on numbering
}

