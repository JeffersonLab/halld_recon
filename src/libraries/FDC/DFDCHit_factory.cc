// $Id$
//
//    File: DFDCHit_factory.cc
// Created: Wed Aug  7 11:55:02 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include <FDC/DFDCGeometry.h>
#include <FDC/DFDCCathodeDigiHit.h>
#include <FDC/DFDCWireDigiHit.h>
#include "DFDCHit_factory.h"
#include <DAQ/Df125PulseIntegral.h>
#include <DAQ/Df125PulsePedestal.h>
#include <DAQ/Df125Config.h>
#include <DAQ/Df125FDCPulse.h>
#include <DANA/DEvent.h>



//------------------
// Init
//------------------
void DFDCHit_factory::Init()
{
  auto app = GetApplication();
  USE_FDC=true; 
  app->SetDefaultParameter("FDC:ENABLE",USE_FDC);
  
   /// set the base conversion scales
   a_scale      = 2.4E4/1.3E5;  // cathodes
   t_scale      = 8.0/10.0;     // 8 ns/count and integer time is in 1/10th of sample
   t_base       = 0.;           // ns
   fadc_t_base  = 0.;           // ns
}

//------------------
// BeginRun
//------------------
void DFDCHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  if (USE_FDC==false) return; // RESOURCE_UNAVAILABLE;

   auto runnumber = event->GetRunNumber();
   JCalibration* calibration = DEvent::GetJCalibration(event);

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

   // reset constants tables
   a_gains.clear();
   a_pedestals.clear();
   timing_offsets.clear();

   // now load them all
   if(print_messages) jout << "In DFDCHit_factory, loading constants..." << endl;

   // load scale factors
   map<string,double> scale_factors;
   if(calibration->Get("/FDC/digi_scales", scale_factors))
      jout << "Error loading /FDC/digi_scales !" << endl;
   if( scale_factors.find("FDC_ADC_ASCALE") != scale_factors.end() ) {
      a_scale = scale_factors["FDC_ADC_ASCALE"];
   } else {
      jerr << "Unable to get FDC_ADC_ASCALE from /FDC/digi_scales !" << endl;
   }
   if( scale_factors.find("FDC_ADC_TSCALE") != scale_factors.end() ) {
      t_scale = scale_factors["FDC_ADC_TSCALE"];
   } else {
      jerr << "Unable to get FDC_ADC_TSCALE from /FDC/digi_scales !" << endl;
   }

   // load base time offset
   map<string,double> base_time_offset;
   if (calibration->Get("/FDC/base_time_offset",base_time_offset))
      jout << "Error loading /FDC/base_time_offset !" << endl;
   if (base_time_offset.find("FDC_BASE_TIME_OFFSET") != base_time_offset.end())
      fadc_t_base = base_time_offset["FDC_BASE_TIME_OFFSET"];
   else
      jerr << "Unable to get FDC_BASE_TIME_OFFSET from /FDC/base_time_offset !" << endl;
   if (base_time_offset.find("FDC_TDC_BASE_TIME_OFFSET") != base_time_offset.end())
      t_base = base_time_offset["FDC_TDC_BASE_TIME_OFFSET"];
   else
      jerr << "Unable to get FDC_TDC_BASE_TIME_OFFSET from /FDC/base_time_offset !" << endl;


   // each FDC package has the same set of constants
   LoadPackageCalibTables(event,"/FDC/package1");
   LoadPackageCalibTables(event,"/FDC/package2");
   LoadPackageCalibTables(event,"/FDC/package3");
   LoadPackageCalibTables(event,"/FDC/package4");

   // Verify that the right number of layers were loaded
   char str[256];
   if(a_gains.size() != FDC_NUM_PLANES) {
      sprintf(str, "Bad # of planes for FDC gains from CCDB! CCDB=%zu , should be %d",
            a_gains.size(), FDC_NUM_PLANES);
      cerr << str << endl;
      throw JException(str);
   }
   if(a_pedestals.size() != FDC_NUM_PLANES) {
      sprintf(str, "Bad # of planes for FDC pedestals from CCDB! CCDB=%zu , should be %d",
            a_pedestals.size(), FDC_NUM_PLANES);
      cerr << str << endl;
      throw JException(str);
   }
   if(timing_offsets.size() != FDC_NUM_PLANES) {
      sprintf(str, "Bad # of planes for FDC timing offsets from CCDB! CCDB=%zu , should be %d",
            timing_offsets.size(), FDC_NUM_PLANES);
      cerr << str << endl;
      throw JException(str);
   }
}

//------------------
// Process
//------------------
void DFDCHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  if (USE_FDC==false) return; // RESOURCE_UNAVAILABLE;

   /// Generate DFDCHit object for each DFDCCathodeDigiHit and
   /// each DFDCWireDigiHit object.
   /// This is where the first set of calibration constants
   /// is applied to convert from digitzed units into natural
   /// units.
   ///
   /// Note that this code does NOT get called for simulated
   /// data in HDDM format. The HDDM event source will copy
   /// the precalibrated values directly into the _data vector.
   char str[256];

   const DTTabUtilities* locTTabUtilities = NULL;
   event->GetSingle(locTTabUtilities);

   // Make hits out of all DFDCCathodeDigiHit hits
   vector<const DFDCCathodeDigiHit*> cathodedigihits;
   event->Get(cathodedigihits);
   for(unsigned int i=0; i<cathodedigihits.size(); i++){
      const DFDCCathodeDigiHit *digihit = cathodedigihits[i];

      // The translation table has:
      // ---------------------------------------------------
      // package : 1-4
      // chamber : 1-6
      // view    : 1(="U") or 3(="D")
      // strip   : 1-192
      //
      //
      // The FDCHit class has 6 indexes which are derived
      // from these and contain some redundancy. They are:
      // ---------------------------------------------------
      // layer   : 1(V), 2(X), or 3(U)
      // module  : 1 through 8, 1 module = 3 detection layers
      // element : wire or strip number
      // plane   : for cathodes only: u(3) or v(1) plane, u@+45,v@-45
      // gPlane  : 1 through 72
      // gLayer  : 1 through 24

      int layer=digihit->view;
      int gLayer=digihit->chamber + 6*(digihit->package - 1);
      int gPlane=layer + 3*(gLayer - 1);
      // Make sure gPlane and stripare in valid range
      if((gPlane < 1) || (gPlane > FDC_NUM_PLANES)) {
         sprintf(str, "DFDCDigiHit plane out of range! gPlane=%d (should be 1-%d)", gPlane, FDC_NUM_PLANES);
         throw JException(str);
      }
      if( (digihit->strip < 1) || (digihit->strip > STRIPS_PER_PLANE)) {
         sprintf(str, "DFDCDigiHit straw out of range! strip=%d for plane=%d (should be 1-%d)", digihit->strip, gPlane, STRIPS_PER_PLANE);
         throw JException(str);
      }

      int plane_index=gPlane-1;
      int strip_index=digihit->strip-1;

      // Apply calibration constants here
      double T = (double)digihit->pulse_time;
      //if (T<=0.) continue;

      // Default pedestal from CCDB
      //double pedestal = a_pedestals[plane_index][strip_index];

      // Grab the pedestal from the digihit since this should be consistent between the old and new formats
      int raw_ped = digihit->pedestal;
      //        int nsamples_integral;

      // There are a few values from the new data type that are critical for the interpretation of the data
      //uint16_t IBIT = 0; // 2^{IBIT} Scale factor for integral
      uint16_t ABIT = 0; // 2^{ABIT} Scale factor for amplitude
      uint16_t PBIT = 0; // 2^{PBIT} Scale factor for pedestal
      uint16_t NW   = 0;
      uint16_t IE   = 0;

      // This is the place to make quality cuts on the data.
      // Try to get the new data type, if that fails, try to get the old...
      int pulse_peak = 0;
      const Df125FDCPulse *FDCPulseObj = NULL;
      digihit->GetSingle(FDCPulseObj);
      if (FDCPulseObj != NULL){
         // Cut on quality factor?
          vector<const Df125Config*> configs;
          digihit->Get(configs);

         // Set some constants to defaults until they appear correctly in the config words in the future
         // The defaults are taken from Run 4607
          if(!configs.empty()){
            const Df125Config *config = configs[0];
            //IBIT = config->IBIT == 0xffff ? 4 : config->IBIT;
            ABIT = config->ABIT == 0xffff ? 3 : config->ABIT;
            PBIT = config->PBIT == 0xffff ? 0 : config->PBIT;
            NW   = config->NW   == 0xffff ? 80 : config->NW;
            IE   = config->IE   == 0xffff ? 16 : config->IE;
         }else{
            static int Nwarnings = 0;
            if(Nwarnings<10){
               _DBG_ << "NO Df125Config object associated with Df125FDCPulse object!" << endl;
               Nwarnings++;
               if(Nwarnings==10) _DBG_ << " --- LAST WARNING!! ---" << endl;
            }
         }
         if ((NW - (digihit->pulse_time / 10)) < IE){
            //                nsamples_integral = (NW - (digihit->pulse_time / 10));
         }
         else{
            //                nsamples_integral = IE;
         }

         pulse_peak = FDCPulseObj->peak_amp << ABIT;
      }
      else{
         // There is a slight difference between Mode 7 and 8 data
         // The following condition signals an error state in the flash algorithm
         // Do not make hits out of these
         const Df125PulsePedestal* PPobj = NULL;
         digihit->GetSingle(PPobj);
         if (PPobj != NULL){
            if (PPobj->pedestal == 0 || PPobj->pulse_peak == 0) continue;
            if (PPobj->pulse_number == 1) continue; // Unintentionally had 2 pulses found in fall data (0-1 counting issue)
            pulse_peak = PPobj->pulse_peak;
         }

         const Df125PulseIntegral* PIobj = NULL;
         digihit->GetSingle(PIobj);
         if ( PPobj == NULL || PIobj == NULL) continue; // We don't want hits where ANY of the associated information is missing
      }

      // Complete the pedestal subtracion here since we should know the correct number of samples.
      int scaled_ped = raw_ped << PBIT;
      //pedestal = double(scaled_ped * nsamples_integral);

      //double integral = double(digihit->pulse_integral << IBIT);
      // Comment this line out temporarily until config words are behaving nicely
      //if (A-pedestal<0.) continue;

      // ------The integral is no longer reported for FDC hits --- SJT 3/4/16
      //double q = a_scale * a_gains[plane_index][strip_index] * (integral-pedestal);
      double t = t_scale * T - timing_offsets[plane_index][strip_index]+fadc_t_base;

      DFDCHit *hit = new DFDCHit;
      hit->layer   = digihit->view;
      hit->gLayer  = gLayer;
      hit->gPlane  = gPlane;
      hit->module  = 1 + (gLayer-1)/3;
      hit->element = digihit->strip;
      hit->plane   = digihit->view; // "plane" is apparently the same as "layer"
      hit->r       = DFDCGeometry::getWireR(hit);
      hit->d       = 0.0; // MC data only
      hit->type    = digihit->strip_type; // n.b. DEventSourceHDDM hardwires this as "1" for cathodes!
      hit->itrack  = -1; // MC data only
      hit->ptype   = 0;// MC data only
      //hit->q = q;
      hit->t = t;
      unsigned int stripOffset=0;
      if (digihit->strip_type == 3) stripOffset=9*12;
      hit->pulse_height=a_gains[plane_index][strip_index+stripOffset]
         *double(pulse_peak - scaled_ped);
      hit->pulse_height_raw = double(pulse_peak - scaled_ped);
      hit->q=a_scale*hit->pulse_height;

      //cerr << "FDC hitL  plane = " << hit->gPlane << "  element = " << hit->element << endl;

      hit->AddAssociatedObject(digihit);

      Insert(hit);
   }

   // Make hits out of all DFDCWireDigiHit hits
   vector<const DFDCWireDigiHit*> wiredigihits;
   event->Get(wiredigihits);
   for(unsigned int i=0; i<wiredigihits.size(); i++){
      const DFDCWireDigiHit *digihit = wiredigihits[i];

      // The translation table has:
      // ---------------------------------------------------
      // package : 1-4
      // chamber : 1-6
      // wire    : 1-96
      //
      //
      // The FDCHit class has 6 indexes which are derived
      // from these and contain some redundancy. They are:
      // ---------------------------------------------------
      // plane   : 1(V), 2(X), or 3(U)
      // layer   : 1 (phi=0), 2 (phi=+60), 3 (phi=-60)
      // module  : 1 through 8, 1 module = 3 detection layers
      // element : wire or strip number
      // plane   : for cathodes only: u(3) or v(1) plane, u@+45,v@-45
      // gPlane  : 1 through 72
      // gLayer  : 1 through 24

      DFDCHit *hit = new DFDCHit;
      hit->gLayer  = digihit->chamber + 6*(digihit->package - 1);
      hit->module  = 1 + (hit->gLayer-1)/3;
      hit->layer   = hit->gLayer - (hit->module-1)*3;
      hit->plane   = 2;                  // wire is always in "X" layer
      hit->gPlane  = hit->plane + 3*(hit->gLayer - 1);
      hit->element = digihit->wire;
      hit->r       = DFDCGeometry::getWireR(hit);
      hit->d       = 0.0;                // MC data only
      hit->type    = DFDCHit::AnodeWire; // n.b. DEventSourceHDDM hardwires this as "0" for anodes!
      hit->itrack  = -1;                 // MC data only
      hit->ptype   = 0;                  // MC data only

      // Make sure gPlane and wire are in valid range
      if( (hit->gPlane < 1) || (hit->gPlane > FDC_NUM_PLANES)) {
         sprintf(str, "DFDCDigiHit plane out of range! gPlane=%d (should be 1-%d)", hit->gPlane, FDC_NUM_PLANES);
         throw JException(str);
      }
      if( (digihit->wire < 1) || (digihit->wire > WIRES_PER_PLANE)) {
         sprintf(str, "DFDCDigiHit straw out of range! wire=%d for plane=%d (should be 1-%d)", digihit->wire, hit->gPlane, WIRES_PER_PLANE);
         throw JException(str);
      }

      // Apply calibration constants here
      double T = locTTabUtilities->Convert_DigiTimeToNs_F1TDC(digihit) - timing_offsets[hit->gPlane-1][hit->element-1] + t_base;
      hit->q = 0.0; // no charge measured for wires in FDC
      hit->pulse_height=0.0;
      hit->t = T;

      hit->AddAssociatedObject(digihit);

      Insert(hit);
   }
}

//------------------
// EndRun
//------------------
void DFDCHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DFDCHit_factory::Finish()
{
}


//------------------
// LoadPackageCalibTables
//------------------
void DFDCHit_factory::LoadPackageCalibTables(const std::shared_ptr<const JEvent>& event, string ccdb_prefix)
{
   vector< vector<double> >  new_gains, new_pedestals, new_strip_t0s, new_wire_t0s;
   char str[256];

   JCalibration* calibration = DEvent::GetJCalibration(event);

   if(calibration->Get(ccdb_prefix+"/strip_gains_v2", new_gains))
      cout << "Error loading "+ccdb_prefix+"/strip_gains_v2 !" << endl;
   if(calibration->Get(ccdb_prefix+"/strip_pedestals", new_pedestals))
      cout << "Error loading "+ccdb_prefix+"/strip_pedestals !" << endl;
   if(calibration->Get(ccdb_prefix+"/strip_timing_offsets", new_strip_t0s))
      cout << "Error loading "+ccdb_prefix+"/strip_timing_offsets!" << endl;
   if(calibration->Get(ccdb_prefix+"/wire_timing_offsets", new_wire_t0s))
      cout << "Error loading "+ccdb_prefix+"/wire_timing_offsets!" << endl;

   for(int nchamber=0; nchamber<6; nchamber++) {

      // check the size of table rows
      // The number of cathode strips was improperly implemented in the CCDB with only 192 channels
      // There are actually 216 cathode strips since 24 are split to different readout around the beam.
      if(new_gains[2*nchamber].size() != STRIPS_PER_PLANE+24) {
         sprintf(str, "Bad # of strips for FDC gain from CCDB! CCDB=%zu , should be %d", new_gains[2*nchamber].size(), STRIPS_PER_PLANE+24);
         cerr << str << endl;
         throw JException(str);
      }
      if(new_gains[2*nchamber+1].size() != STRIPS_PER_PLANE+24) {
         sprintf(str, "Bad # of strips for FDC gain from CCDB! CCDB=%zu , should be %d", new_gains[2*nchamber+1].size(), STRIPS_PER_PLANE+24);
         cerr << str << endl;
         throw JException(str);
      }
      if(new_pedestals[2*nchamber].size() != STRIPS_PER_PLANE) {
         sprintf(str, "Bad # of strips for FDC pedestals from CCDB! CCDB=%zu , should be %d", new_pedestals[2*nchamber].size(), STRIPS_PER_PLANE);
         cerr << str << endl;
         throw JException(str);
      }
      if(new_pedestals[2*nchamber+1].size() != STRIPS_PER_PLANE) {
         sprintf(str, "Bad # of strips for FDC pedestals from CCDB! CCDB=%zu , should be %d", new_pedestals[2*nchamber+1].size(), STRIPS_PER_PLANE);
         cerr << str << endl;
         throw JException(str);
      }
      if(new_strip_t0s[2*nchamber].size() != STRIPS_PER_PLANE) {
         sprintf(str, "Bad # of strips for FDC timing offsets from CCDB! CCDB=%zu , should be %d", new_strip_t0s[2*nchamber].size(), STRIPS_PER_PLANE);
         cerr << str << endl;
         throw JException(str);
      }
      if(new_strip_t0s[2*nchamber+1].size() != STRIPS_PER_PLANE) {
         sprintf(str, "Bad # of strips for FDC timing offsets from CCDB! CCDB=%zu , should be %d", new_strip_t0s[2*nchamber+1].size(), STRIPS_PER_PLANE);
         cerr << str << endl;
         throw JException(str);
      }
      if(new_wire_t0s[nchamber].size() != WIRES_PER_PLANE) {
         sprintf(str, "Bad # of wires for FDC timing offsets from CCDB! CCDB=%zu , should be %d", new_wire_t0s[2*nchamber].size(), WIRES_PER_PLANE);
         cerr << str << endl;
         throw JException(str);
      }


      // load ADC gains (only for cathode strips)
      a_gains.push_back( new_gains[2*nchamber] );
      a_gains.push_back( vector<double>() );
      a_gains.push_back( new_gains[2*nchamber+1] );

      // load ADC pedestals (only for cathode strips)
      a_pedestals.push_back( new_pedestals[2*nchamber] );
      a_pedestals.push_back( vector<double>() );
      a_pedestals.push_back( new_pedestals[2*nchamber+1] );

      // load t0's for strips and wires
      timing_offsets.push_back( new_strip_t0s[2*nchamber] );
      timing_offsets.push_back( new_wire_t0s[nchamber] );
      timing_offsets.push_back( new_strip_t0s[2*nchamber+1] );

   }
}

//------------------------------------
// GetConstant
//   Allow a few different interfaces
//------------------------------------
const double DFDCHit_factory::GetConstant(const fdc_digi_constants_t &the_table,
      const int in_gPlane, const int in_element) const {

   char str[256];

   if( (in_gPlane <= 0) || (static_cast<unsigned int>(in_gPlane) > FDC_NUM_PLANES)) {
      sprintf(str, "Bad gPlane # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", in_gPlane, FDC_NUM_PLANES);
      cerr << str << endl;
      throw JException(str);
   }
   // strip and wire planes have different numbers of elements
   if( (in_element <= 0) || (static_cast<unsigned int>(in_element) > the_table[in_gPlane].size())) {
      sprintf(str, "Bad element # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %zu", in_element, the_table[in_gPlane].size());
      cerr << str << endl;
      throw JException(str);
   }

   return the_table[in_gPlane-1][in_element-1];
}

const double DFDCHit_factory::GetConstant(const fdc_digi_constants_t &the_table,
      const DFDCCathodeDigiHit *in_digihit) const {

   char str[256];

   int gLayer = in_digihit->chamber + 6*(in_digihit->package - 1);
   int gPlane = in_digihit->view + 3*(gLayer - 1);

   if( (gPlane <= 0) || (static_cast<unsigned int>(gPlane) > FDC_NUM_PLANES)) {
      sprintf(str, "Bad gPlane # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", gPlane, FDC_NUM_PLANES);
      cerr << str << endl;
      throw JException(str);
   }
   // strip and wire planes have different numbers of elements
   if( (in_digihit->strip <= 0) || (static_cast<unsigned int>(in_digihit->strip) > STRIPS_PER_PLANE)) {
      sprintf(str, "Bad strip # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", in_digihit->strip, STRIPS_PER_PLANE);
      cerr << str << endl;
      throw JException(str);
   }

   return the_table[gPlane-1][in_digihit->strip-1];
}

const double DFDCHit_factory::GetConstant(const fdc_digi_constants_t &the_table,
      const DFDCWireDigiHit *in_digihit) const {

   char str[256];

   int gLayer = in_digihit->chamber + 6*(in_digihit->package - 1);
   int gPlane = 2 + 3*(gLayer - 1);

   if( (gPlane <= 0) || (static_cast<unsigned int>(gPlane) > FDC_NUM_PLANES)) {
      sprintf(str, "Bad gPlane # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", gPlane, FDC_NUM_PLANES);
      cerr << str << endl;
      throw JException(str);
   }
   // strip and wire planes have different numbers of elements
   if( (in_digihit->wire <= 0) || (static_cast<unsigned int>(in_digihit->wire) > WIRES_PER_PLANE)) {
      sprintf(str, "Bad wire # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", in_digihit->wire, WIRES_PER_PLANE);
      cerr << str << endl;
      throw JException(str);
   }

   return the_table[gPlane-1][in_digihit->wire-1];
}

const double DFDCHit_factory::GetConstant(const fdc_digi_constants_t &the_table,
      const DFDCHit *in_hit) const {

   char str[256];

   if( (in_hit->gPlane <= 0) || (static_cast<unsigned int>(in_hit->gPlane) > FDC_NUM_PLANES)) {
      sprintf(str, "Bad gPlane # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", in_hit->gPlane, FDC_NUM_PLANES);
      cerr << str << endl;
      throw JException(str);
   }
   // strip and wire planes have different numbers of elements
   if( (in_hit->element <= 0) || (static_cast<unsigned int>(in_hit->element) > the_table[in_hit->gPlane].size())) {
      sprintf(str, "Bad element # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %zu", in_hit->element, the_table[in_hit->gPlane].size());
      cerr << str << endl;
      throw JException(str);
   }

   return the_table[in_hit->gPlane-1][in_hit->element-1];
}
/*
   const double DFDCHit_factory::GetConstant(const fdc_digi_constants_t &the_table,
   const DTranslationTable *ttab,
   const int in_rocid, const int in_slot, const int in_channel) const {

   char str[256];

   DTranslationTable::csc_t daq_index = { in_rocid, in_slot, in_channel };
   DTranslationTable::DChannelInfo channel_info = ttab->GetDetectorIndex(daq_index);

   if( channel_info.det_sys == DTranslationTable::FDC_CATHODES ) {
// FDC Cathodes
int gLayer = channel_info.fdc_cathodes.chamber + 6*(channel_info.fdc_cathodes.package - 1);
int gPlane = channel_info.fdc_cathodes.view + 3*(gLayer - 1);

if( (gPlane <= 0) || (gPlane > FDC_NUM_PLANES)) {
sprintf(str, "Bad gPlane # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", gPlane, FDC_NUM_PLANES);
cerr << str << endl;
throw JException(str);
}
// strip and wire planes have different numbers of elements
if( (channel_info.fdc_cathodes.strip <= 0)
|| (channel_info.fdc_cathodes.strip > STRIPS_PER_PLANE)) {
sprintf(str, "Bad strip # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", channel_info.fdc_cathodes.strip, STRIPS_PER_PLANE);
cerr << str << endl;
throw JException(str);
}

return the_table[gPlane-1][channel_info.fdc_cathodes.strip-1];
} else if( channel_info.det_sys == DTranslationTable::FDC_WIRES ) {
// FDC Wirees
int gLayer = channel_info.fdc_wires.chamber + 6*(channel_info.fdc_wires.package - 1);
int gPlane = 2 + 3*(gLayer - 1);  // wire planes are always layer 2

if( (gPlane <= 0) || (gPlane > FDC_NUM_PLANES)) {
sprintf(str, "Bad gPlane # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", gPlane, FDC_NUM_PLANES);
cerr << str << endl;
throw JException(str);
}
// strip and wire planes have different numbers of elements
if( (channel_info.fdc_wires.wire <= 0)
|| (channel_info.fdc_wires.wire > WIRES_PER_PLANE)) {
sprintf(str, "Bad strip # requested in DFDCHit_factory::GetConstant()! requested=%d , should be %ud", channel_info.fdc_wires.wire, WIRES_PER_PLANE);
cerr << str << endl;
throw JException(str);
}

return the_table[gPlane-1][channel_info.fdc_wires.wire-1];
} else {
sprintf(str, "Got bad detector type in DFDCHit_factory::GetConstant()! requested=%d", channel_info.module_type);
cerr << str << endl;
throw JException(str);

return -1.;  // should never reach here!
}
}
*/
