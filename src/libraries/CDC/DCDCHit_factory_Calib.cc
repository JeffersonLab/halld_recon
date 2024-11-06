// $Id$
//
//    File: DCDCHit_factory_Calib.cc
// Created: Fri Mar  9 16:57:04 EST 2018
// Creator: B. Zihlmann, derived/copyed from DCDCHit_factory.cc
//


#include <CDC/DCDCHit_factory_Calib.h>

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>
#include <DANA/DGeometryManager.h>
#include <HDGEOMETRY/DGeometry.h>

//#define ENABLE_UPSAMPLING

//------------------
// Init
//------------------
void DCDCHit_factory_Calib::Init()
{
  CDC_HIT_THRESHOLD = 0;
  auto app = GetApplication();
  app->SetDefaultParameter("CDC:CDC_HIT_THRESHOLD", CDC_HIT_THRESHOLD,
                              "Remove CDC Hits with peak amplitudes smaller than CDC_HIT_THRESHOLD");

  // After a saturated pulse, small afterpulses can occur on other channels in the same preamp
  // Single-peak afterpulses occur after 3-7 samples
  // If ECHO_OPT=1, likely afterpulses are removed

  ECHO_OPT = 1;
  gPARMS->SetDefaultParameter("CDC:ECHO_OPT", ECHO_OPT,
                              "0:do not suppress afterpulses, 1:suppress afterpulses");
  
  ECHO_MAX_A = 350;
  gPARMS->SetDefaultParameter("CDC:ECHO_MAX_A", ECHO_MAX_A,
                              "Max height (adc units 0-4095) for afterpulses, if ECHO_OPT=1");

  ECHO_MAX_T = 7;
  gPARMS->SetDefaultParameter("CDC:ECHO_MAX_T", ECHO_MAX_T,
                              "End of time range (number of samples) to search for afterpulses");
  
  // default values
  Nrings = 0;
  a_scale = 0.;
  t_scale = 0.;
  t_base = 0.;
  
  // Set default number of number of detector channels
  maxChannels = 3522;
  
  /// set the base conversion scales
  a_scale = 4.0E3/1.0E2; 
  amp_a_scale = a_scale*28.8;
  t_scale = 8.0/10.0;    // 8 ns/count and integer time is in 1/10th of sample
  t_base  = 0.;       // ns
}

//------------------
// BeginRun
//------------------
void DCDCHit_factory_Calib::BeginRun(const std::shared_ptr<const JEvent>& event)
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
  
  // calculate the number of straws in each ring
  CalcNstraws(event, runnumber, Nstraws);
  Nrings = Nstraws.size();
  
  vector<double> raw_gains;
  vector<double> raw_pedestals;
  vector<double> raw_time_offsets;
  
  if(print_messages) jout << "In DCDCHit_factory, loading constants..." << std::endl;
  
  // load scale factors
  map<string,double> scale_factors;
  if (calibration->Get("/CDC/digi_scales", scale_factors))
    jout << "Error loading /CDC/digi_scales !" << endl;
  if (scale_factors.find("CDC_ADC_ASCALE") != scale_factors.end())
    a_scale = scale_factors["CDC_ADC_ASCALE"];
  else
    jerr << "Unable to get CDC_ADC_ASCALE from /CDC/digi_scales !" << endl;
  amp_a_scale=a_scale*28.8;
  
#ifdef ENABLE_UPSAMPLING
  //t_scale=1.;
#else
  if (scale_factors.find("CDC_ADC_TSCALE") != scale_factors.end())
    t_scale = scale_factors["CDC_ADC_TSCALE"];
  else
    jerr << "Unable to get CDC_ADC_TSCALE from /CDC/digi_scales !" << endl;
#endif
  
  // load base time offset
  map<string,double> base_time_offset;
  if (calibration->Get("/CDC/base_time_offset",base_time_offset))
    jout << "Error loading /CDC/base_time_offset !" << endl;
  if (base_time_offset.find("CDC_BASE_TIME_OFFSET") != base_time_offset.end())
    t_base = base_time_offset["CDC_BASE_TIME_OFFSET"];
  else
    jerr << "Unable to get CDC_BASE_TIME_OFFSET from /CDC/base_time_offset !" << endl;
  
  // load constant tables
  if (calibration->Get("/CDC/wire_gains", raw_gains))
    jout << "Error loading /CDC/wire_gains !" << endl;
  if (calibration->Get("/CDC/pedestals", raw_pedestals))
    jout << "Error loading /CDC/pedestals !" << endl;
  if (calibration->Get("/CDC/timing_offsets", raw_time_offsets))
    jout << "Error loading /CDC/timing_offsets !" << endl;
  
  // fill the tables
  FillCalibTable(gains, raw_gains, Nstraws);
  FillCalibTable(pedestals, raw_pedestals, Nstraws);
  FillCalibTable(time_offsets, raw_time_offsets, Nstraws);
  
  // Verify that the right number of rings was read for each set of constants
  char str[256];
  if (gains.size() != Nrings) {
    sprintf(str, "Bad # of rings for CDC gain from CCDB! CCDB=%zu , should be %d", gains.size(), Nrings);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  if (pedestals.size() != Nrings) {
    sprintf(str, "Bad # of rings for CDC pedestal from CCDB! CCDB=%zu , should be %d", pedestals.size(), Nrings);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  if (time_offsets.size() != Nrings) {
    sprintf(str, "Bad # of rings for CDC time_offset from CCDB!"
	    " CCDB=%zu , should be %d", time_offsets.size(), Nrings);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  
  // Verify the right number of straws was read for each ring for each set of constants
  for (unsigned int i=0; i < Nrings; i++) {
    if (gains[i].size() != Nstraws[i]) {
      sprintf(str, "Bad # of straws for CDC gain from CCDB!"
	      " CCDB=%zu , should be %d for ring %d",
	      gains[i].size(), Nstraws[i], i+1);
      std::cerr << str << std::endl;
      throw JException(str);
    }
    if (pedestals[i].size() != Nstraws[i]) {
      sprintf(str, "Bad # of straws for CDC pedestal from CCDB!"
	      " CCDB=%zu , should be %d for ring %d",
	      pedestals[i].size(), Nstraws[i], i+1);
      std::cerr << str << std::endl;
      throw JException(str);
    }
    if (time_offsets[i].size() != Nstraws[i]) {
      sprintf(str, "Bad # of straws for CDC time_offset from CCDB!"
	      " CCDB=%zu , should be %d for ring %d",
	      time_offsets[i].size(), Nstraws[i], i+1);
      std::cerr << str << std::endl;
      throw JException(str);
    }
  }

  return;
}

//------------------
// Process
//------------------
void DCDCHit_factory_Calib::Process(const std::shared_ptr<const JEvent>& event)
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
  
  vector<const DCDCDigiHit*> digihits;
  event->Get(digihits);
  char str[256];


  // flag the small nuisance hits that follow a saturated hit on the same board.

  vector <unsigned int> RogueHits;
  RogueHits.clear();

  if (ECHO_OPT > 0) FindRogueHits(eventLoop,RogueHits);

  
  for (unsigned int i=0; i < digihits.size(); i++) {
    const DCDCDigiHit *digihit = digihits[i];
    
    //if ( (digihit->QF & 0x1) != 0 ) continue; // Cut bad timing quality factor hits... (should check effect on efficiency)
    
    bool skip = 0;
    if (RogueHits.size()>0) {
      if (i==RogueHits[0]) {
	skip = 1;
        RogueHits.erase(RogueHits.begin());
      }
    }

    if (skip) continue;

    const int &ring  = digihit->ring;
    const int &straw = digihit->straw;
    
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

    // Grab the pedestal from the digihit since this should be consistent between the old and new formats
    int raw_ped           = digihit->pedestal;
    int maxamp            = digihit->pulse_peak;
    int nsamples_integral = 0; // actual number computed below using config info
    
    // There are a few values from the new data type that are critical for the interpretation of the data
    uint16_t IBIT = 0; // 2^{IBIT} Scale factor for integral
    uint16_t ABIT = 0; // 2^{ABIT} Scale factor for amplitude
    uint16_t PBIT = 0; // 2^{PBIT} Scale factor for pedestal
    uint16_t NW   = 0;
    
    // This is the place to make quality cuts on the data. 
    const Df125PulsePedestal* PPobj = NULL;
    digihit->GetSingle(PPobj);
    if( PPobj != NULL ) { 
      // Use the old format - mostly handle error conditions
      // This code will at some point become deprecated in the future...
      // This applies to the firmware for data taken until the fall of 2015.
      // Mode 8: Raw data and processed data (except pulse integral).
      // Mode 7: Processed data only.
      
      // This error state is only present in mode 8
      if (digihit->pulse_time==0.) continue;
      
      // There is a slight difference between Mode 7 and 8 data
      // The following condition signals an error state in the flash algorithm
      // Do not make hits out of these
      if (PPobj != NULL){
	if (PPobj->pedestal == 0 || PPobj->pulse_peak == 0) continue;
	if (PPobj->pulse_number == 1) continue; // Unintentionally had 2 pulses found in fall 2014 data (0-1 counting issue)
      }
      
      const Df125PulseIntegral* PIobj = NULL;
      digihit->GetSingle(PIobj);
      if (PPobj == NULL || PIobj == NULL) continue; // We don't want hits where ANY of the associated information is missing
      
      // this amplitude is not set in the translation table for this old data format, so make a (reasonable?) guess
      maxamp = digihit->pulse_integral / 28.8;
    } else {

      // Use the modern (2017+) data versions
      // Configuration data needed to interpret the hits is stored in the data stream
      vector<const Df125Config*> configs;
      digihit->Get(configs);
      if( configs.empty() ){
	static int Nwarnings = 0;
	if(Nwarnings<10){
	  _DBG_ << "NO Df125Config object associated with Df125CDCPulse object!" << endl;
	  Nwarnings++;
	  if(Nwarnings==10) _DBG_ << " --- LAST WARNING!! ---" << endl;
	}
      }else{
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

    }

    
    // Complete the pedestal subtraction here since we should know the correct number of samples.
    int scaled_ped = raw_ped << PBIT;
    
    if (maxamp > 0) maxamp = maxamp << ABIT;
    //if (maxamp <= scaled_ped) continue;
    
    maxamp = maxamp - scaled_ped;
    
    if (maxamp<CDC_HIT_THRESHOLD) {
      continue;
    }
    
    // Apply calibration constants here
    double t_raw = double(digihit->pulse_time);
    
    // Scale factor to account for gain variation
    double gain=gains[ring-1][straw-1];
       
    // Charge and amplitude 
    double q = a_scale *gain * double((digihit->pulse_integral<<IBIT)
				      - scaled_ped*nsamples_integral);
    double amp = amp_a_scale*gain*double(maxamp);
    
    double t = t_scale * t_raw - time_offsets[ring-1][straw-1] + t_base;
    
    DCDCHit *hit = new DCDCHit;
    hit->ring  = ring;
    hit->straw = straw;
    
    // Values for d, itrack, ptype only apply to MC data
    // note that ring/straw counting starts at 1
    hit->q = q;
    hit->amp = amp;
    hit->t = t;
    hit->d = 0.0;
    hit->QF = digihit->QF;
    hit->itrack = -1;
    hit->ptype = 0;
    
    hit->AddAssociatedObject(digihit);
    
    Insert(hit);
  }
}


//------------------
// EndRun
//------------------
void DCDCHit_factory_Calib::EndRun()
{
}

//------------------
// Finish
//------------------
void DCDCHit_factory_Calib::Finish()
{
}

//------------------
// CalcNstraws
//------------------
void DCDCHit_factory_Calib::CalcNstraws(const std::shared_ptr<const JEvent>& event, int32_t runnumber, vector<unsigned int> &Nstraws)
{
  DGeometry *dgeom;
  vector<vector<DCDCWire *> >cdcwires;
  
  // Get pointer to DGeometry object
  auto app = event->GetJApplication();
  auto geo_manager = app->GetService<DGeometryManager>();
  dgeom = geo_manager->GetDGeometry(runnumber);

  // Get the CDC wire table from the XML
  dgeom->GetCDCWires(cdcwires);
  
  // Fill array with the number of straws for each layer
  // Also keep track of the total number of straws, i.e., the total number of detector channels
  maxChannels = 0;
  Nstraws.clear();
  for (unsigned int i=0; i<cdcwires.size(); i++) {
    Nstraws.push_back( cdcwires[i].size() );
    maxChannels += cdcwires[i].size();
  }
  
  // clear up all of the wire information
  for (unsigned int i=0; i<cdcwires.size(); i++) {
    for (unsigned int j=0; j<cdcwires[i].size(); j++) {
      delete cdcwires[i][j];
    }
  }    
  cdcwires.clear();
}


//------------------
// FillCalibTable
//------------------
void DCDCHit_factory_Calib::FillCalibTable(vector< vector<double> > &table, vector<double> &raw_table, 
					   vector<unsigned int> &Nstraws)
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


//------------------------------------
// GetConstant
//   Allow a few different interfaces
//------------------------------------
const double DCDCHit_factory_Calib::GetConstant(const cdc_digi_constants_t &the_table,
						const int in_ring, const int in_straw) const {
  
  char str[256];
  
  if ( (in_ring <= 0) || (static_cast<unsigned int>(in_ring) > Nrings)) {
    sprintf(str, "Bad ring # requested in DCDCHit_factory::GetConstant()!"
	    " requested=%d , should be %ud", in_ring, Nrings);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  if ( (in_straw <= 0) || 
       (static_cast<unsigned int>(in_straw) > Nstraws[in_ring]))
    {
      sprintf(str, "Bad straw # requested in DCDCHit_factory::GetConstant()!"
	      " requested=%d , should be %ud", in_straw, Nstraws[in_ring]);
      std::cerr << str << std::endl;
      throw JException(str);
    }
  
  return the_table[in_ring-1][in_straw-1];
}

const double DCDCHit_factory_Calib::GetConstant(const cdc_digi_constants_t &the_table,
						const DCDCDigiHit *in_digihit) const {
  
  char str[256];
  
  if ( (in_digihit->ring <= 0) || 
       (static_cast<unsigned int>(in_digihit->ring) > Nrings))
    {
      sprintf(str, "Bad ring # requested in DCDCHit_factory::GetConstant()!"
	      " requested=%d , should be %ud", in_digihit->ring, Nrings);
      std::cerr << str << std::endl;
      throw JException(str);
    }
  if ( (in_digihit->straw <= 0) || 
       (static_cast<unsigned int>(in_digihit->straw) > Nstraws[in_digihit->ring]))
    {
      sprintf(str, "Bad straw # requested in DCDCHit_factory::GetConstant()!"
	      " requested=%d , should be %ud",
	      in_digihit->straw, Nstraws[in_digihit->ring]);
      std::cerr << str << std::endl;
      throw JException(str);
    }
  
  return the_table[in_digihit->ring-1][in_digihit->straw-1];
}

const double DCDCHit_factory_Calib::GetConstant(const cdc_digi_constants_t &the_table,
						const DCDCHit *in_hit) const {
  
  char str[256];
  
  if ( (in_hit->ring <= 0) || (static_cast<unsigned int>(in_hit->ring) > Nrings)) {
    sprintf(str, "Bad ring # requested in DCDCHit_factory::GetConstant()!"
	    " requested=%d , should be %ud", in_hit->ring, Nrings);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  if ( (in_hit->straw <= 0) || 
       (static_cast<unsigned int>(in_hit->straw) > Nstraws[in_hit->ring])) {
    sprintf(str, "Bad straw # requested in DCDCHit_factory::GetConstant()!"
	    " requested=%d , should be %ud",
	    in_hit->straw, Nstraws[in_hit->ring]);
    std::cerr << str << std::endl;
    throw JException(str);
  }
  
  return the_table[in_hit->ring-1][in_hit->straw-1];
}

//------------------
// Identify rogue hits
//------------------
void DCDCHit_factory_Calib::FindRogueHits(jana::JEventLoop *loop, vector<unsigned int> &RogueHits)
{

  /* // Beni's trick for getting the DAQ channel info for simulated data 
     // Keeping it here in case this code is moved into Hit_factory.cc 
  // loop over hits and find roc/slot/con numbers
  for (unsigned int k=0 ;k<hits.size(); k++){
    const DCDCHit *hit = hits[k];
    vector <const Df125CDCPulse*> pulse;
    hit->Get(pulse);
    
    if(pulse.size()==0) {
      // for hits without lower-level hit info, e.g. HDDM data, we have to use the translation table
      // to figure out which DAQ channels his hit corresponds to
      try {
	DTranslationTable::DChannelInfo channel_info;
	channel_info.det_sys = DTranslationTable::CDC;
	channel_info.cdc.ring = hit->ring;
	channel_info.cdc.straw = hit->straw;
	DTranslationTable::csc_t daq_index = ttab[0]->GetDAQIndex(channel_info);
	
	hit_info.rocid = daq_index.rocid;
	hit_info.slot = daq_index.slot;
	hit_info.connector = daq_index.channel / 24;
      } catch(...) { 
	cout << "Cannot find Translation Table data for hit on ring " << hit->ring
	     << " straw " << hit->straw << ", skipping this info ..." << endl;
	continue;
      }  
    }
    
   */
  
  RogueHits.clear();

  vector<const DCDCDigiHit*> digihits;
  loop->Get(digihits);

  if (digihits.size() == 0) return;

  uint16_t ABIT = 0; // 2^{ABIT} Scale factor for amplitude
  uint16_t PBIT = 0; // 2^{PBIT} Scale factor for pedestal

  const Df125Config *config = NULL;
  digihits[0]->GetSingle(config);

  if(config) { 
      ABIT = config->ABIT;
      PBIT = config->PBIT; 
  } else {
      ABIT = 3;
      PBIT = 0;
  }

  // store list of saturated hit times and their hvb number

  vector<unsigned int> sat_boards;  // code for hvb w saturated hits
  vector<vector<unsigned int>> sat_times;  // saturated hit times, a vector of these for each board
  
  for (unsigned int i=0; i < (unsigned int)digihits.size(); i++) {

    const DCDCDigiHit *digihit = digihits[i];
  
      const Df125CDCPulse *cp = NULL;
      digihit->GetSingle(cp);
      if (!cp) continue ; 
  
      uint32_t rocid = cp->rocid;
      uint32_t slot = cp->slot;
      uint32_t channel = cp->channel;
      uint32_t amp = cp->first_max_amp<<ABIT;
      
      unsigned int preamp = (unsigned int)(channel/24);
      unsigned int rought = (unsigned int)(cp->le_time/10);
            
      unsigned int board = (unsigned int)rocid*100000 + (unsigned int)slot*100 + preamp;  
      
      //  511<<3 = 4088, so check overflows too, and ensure that the overflows are from the first pulse
      if ( amp >= 4088 && cp->overflow_count>0 ) {    

        // check to see if this board was already registered 

        bool found = 0;
        unsigned int x = 0;

	while (!found && x < sat_boards.size()) {
	    if (board == sat_boards[x]) found = 1;
            x++;
        }
	  
        if (found) {   // add the time to the list saved earlier

	   sat_times[x-1].push_back(rought);
	  
	} else {       // new board.  register its number and start a new list of times
	  
	   sat_boards.push_back(board);  
           sat_times.push_back({rought});

	 }  
      }      
  } 

    
  if (sat_times.size() == 0) return;

  
  // check for small afterpulses

  for (unsigned int i=0; i < (unsigned int)digihits.size(); i++) {
    
      const DCDCDigiHit *digihit = digihits[i];
  
      const Df125CDCPulse *cp = NULL;
      digihit->GetSingle(cp);
      if (!cp) continue ; 
  
      uint32_t rocid = cp->rocid;
      uint32_t slot = cp->slot;
      uint32_t channel = cp->channel;
  
      unsigned int preamp = (unsigned int)(channel/24);
      unsigned int rought = (unsigned int)(cp->le_time/10);
  
      unsigned int dt;  // time difference between saturated & later pulses
        
      unsigned int board = (unsigned int)rocid*100000 + (unsigned int)slot*100 + preamp;  
      
      // find out if there's a saturated hit on the same HVB
  
      bool found = 0;
      unsigned int x = 0;

      while (!found && x < sat_boards.size()) {
          if (board == sat_boards[x]) found = 1;
          x++;
      }
      
      if (!found) continue;
  
      x = x-1;
      
      // fill RogueHits if this is a problem pulse
      
      unsigned int net_amp = (unsigned int)(cp->first_max_amp<<ABIT) - (unsigned int)(cp->pedestal<<PBIT);
  
      if (net_amp < ECHO_MAX_A) {
  
          // look at times of saturated pulses to see if any is a candidate for causing this hit as an afterpulse
  
          found = 0;
  
          for (unsigned int j=0; j<(unsigned int)sat_times[x].size(); j++) {
  
              if (rought <= sat_times[x][j] ) continue; // saturated pulse was too late 
  
              dt = rought - sat_times[x][j];   // time delay between saturated pulse and this one
  
    	      if (dt >=2 && dt <= ECHO_MAX_T) found = 1;    // afterpulses start at dt=2
  	
              if (found) break;
  
          }
  
          if (found) RogueHits.push_back(i);
  
      }
      
  }

}
