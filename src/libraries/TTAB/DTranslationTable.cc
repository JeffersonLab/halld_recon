// $Id$
//
//    File: DTranslationTable.cc
// Created: Thu Jun 27 16:07:11 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#include "DTranslationTable.h"

#include <expat.h>
#include <sstream>

#include <DAQ/DModuleType.h>
#include <PAIR_SPECTROMETER/DPSGeometry.h>

#include <JANA/Calibrations/JCalibrationManager.h>

using namespace std;


// Use one translation table for all threads
pthread_mutex_t& DTranslationTable::Get_TT_Mutex(void) const
{
	static pthread_mutex_t tt_mutex = PTHREAD_MUTEX_INITIALIZER;
	return tt_mutex;
}

bool& DTranslationTable::Get_TT_Initialized(void) const
{
	static bool tt_initialized = false;
	return tt_initialized;
}

map<DTranslationTable::csc_t, DTranslationTable::DChannelInfo>& DTranslationTable::Get_TT(void) const
{
	static map<DTranslationTable::csc_t, DTranslationTable::DChannelInfo> TT;
	return TT;
}

map<uint32_t, uint32_t>& DTranslationTable::Get_ROCID_Map(void) const
{
	static map<uint32_t, uint32_t> rocid_map;     // (see ReadOptionalROCidTranslation() for details)
	return rocid_map;
}

map<uint32_t, uint32_t>& DTranslationTable::Get_ROCID_Inv_Map(void) const
{
	static map<uint32_t, uint32_t> rocid_inv_map; // (see ReadOptionalROCidTranslation() for details)
	return rocid_inv_map;
}

map<DTranslationTable::Detector_t, set<uint32_t> >& DTranslationTable::Get_ROCID_By_System(void)
{
	static map<DTranslationTable::Detector_t, set<uint32_t> > rocid_by_system;
	return rocid_by_system;
}

int& DTranslationTable::Get_ROCID_By_System_Mismatch_Behaviour(void)
{
    // This is a flag set via the EVIO:SYSTEMS_TO_PARSE_FORCE variable. It is used
    // to determine how mismatches between the CCDB and the hard-coded rocid map
    // are handled. Note that this is really only used when EVIO:SYSTEMS_TO_PARSE
    // is set. Values are:
    // 0  -  Treat as error
    // 1  -  Use CCDB
    // 2  -  Use hard-coded
    static int mismatch_behaviour = 0;
    return mismatch_behaviour;
}

//...................................
// Less than operator for csc_t data types. This is used by
// the map<csc_t, XX> to order the entires by key
bool operator<(const DTranslationTable::csc_t &a, const DTranslationTable::csc_t &b) {
   if (a.rocid < b.rocid) return true;
   if (a.rocid > b.rocid) return false;
   if (a.slot < b.slot) return true;
   if (a.slot > b.slot) return false;
   if (a.channel < b.channel) return true;
   return false;
}

//...................................
// sort functions
bool SortBCALDigiHit(const DBCALDigiHit *a, const DBCALDigiHit *b) {
   if (a->module == b->module) {
      if (a->layer == b->layer) {
         if (a->sector == b->sector) {
            if (a->end == b->end) {
               return a->pulse_time < b->pulse_time;
            }else{ return a->end < b->end; }
         }else{ return a->sector < b->sector; }
      }else{ return a->layer< b->layer; }
   }else { return a->module < b->module; }
}

//---------------------------------
// DTranslationTable    (Constructor)
//---------------------------------
DTranslationTable::DTranslationTable(JApplication* app, JEvent* event)
{
   // Default is to just read translation table from CCDB. If this fails,
   // then an attempt will be made to read from a file on the local disk.
   // The filename can be specified to be anything, but if the user specifies
   // this, then we assume that they want to use it and skip using the CCDB.
   // They may also specify that they want to skip checking the CCDB via
   // the "TT:NO_CCDB" parameter. This would only be useful if they want to
   // force the use of a local file named "tt.xml".
   NO_CCDB = false;
   XML_FILENAME = "tt.xml";
   VERBOSE = 0;
   SYSTEMS_TO_PARSE = "";
   CALL_STACK = false;
   app->SetDefaultParameter("TT:NO_CCDB", NO_CCDB,
           "Don't try getting translation table from CCDB and just look"
           " for file. Only useful if you want to force reading tt.xml."
           " This is automatically set if you specify a different"
           " filename via the TT:XML_FILENAME parameter.");
   JParameter *p = app->SetDefaultParameter("TT:XML_FILENAME", XML_FILENAME,
           "Fallback filename of translation table XML file."
           " If set to non-default, CCDB will not be checked.");
   if (!p->IsDefault())
     NO_CCDB = true;
   app->SetDefaultParameter("TT:VERBOSE", VERBOSE,
           "Verbosity level for Applying Translation Table."
           " 0=no messages, 10=all messages.");
   
   ROCID_MAP_FILENAME = "rocid.map";
   app->SetDefaultParameter("TT:ROCID_MAP_FILENAME", ROCID_MAP_FILENAME,
           "Optional rocid to rocid conversion map for use with files"
           " generated with the non-standard rocid's");

	app->SetDefaultParameter("TT:SYSTEMS_TO_PARSE", SYSTEMS_TO_PARSE,"This is deprecated. Please use EVIO:SYSTEMS_TO_PARSE instead.");

	app->SetDefaultParameter("TT:CALL_STACK", CALL_STACK,
			"Set this to one to try and force correct recording of the"
			"JANA call stack. You will want this if using the janadot"
			"plugin, but otherwise, it will just give a slight performance"
			"hit.");
	if(SYSTEMS_TO_PARSE != ""){
		jerr << "You have set the TT:SYSTEMS_TO_PARSE config. parameter." << endl;
		jerr << "This is now deprecated. Please use EVIO:SYSTEMS_TO_PARSE" << endl;
		jerr << "instead. Quitting now to make sure you see this message." << endl;
		exit(-1);
	}

	// Here we create a bunch of config. parameters to allow us to overwrite
	// the nsamples_integral and nsamples_pedestal fields of each type of
	// fADC digihit class. This is done using some special macros in
	// DTranslationTable.h
	InitNsamplesOverride(app);

	// Initialize dedicated JStreamLog used for debugging messages
	ttout.SetTag("--- TT ---: ");
	ttout.SetTimestampFlag();
	ttout.SetThreadstampFlag();

	// Look for and read in an optional rocid <-> rocid translation table
	ReadOptionalROCidTranslation();

	// Read in Translation table. This will create DChannelInfo objects
	// and store them in the "TT" map, indexed by csc_t objects
	ReadTranslationTable(app->GetService<JCalibrationManager>()->GetJCalibration(event->GetRunNumber()));
}

//---------------------------------
// ~DTranslationTable    (Destructor)
//---------------------------------
DTranslationTable::~DTranslationTable()
{

}

//---------------------------------
// ReadOptionalROCidTranslation
//---------------------------------
void DTranslationTable::ReadOptionalROCidTranslation(void)
{
   // Some data may be taken with the ROC ID value set
   // incorrectly in CODA. For CODA 3.0 data, there is
   // actually no way to set it so it can be different
   // for every CODA configuration. A simple work-around
   // for this is to use a map file to list the translation
   // from the crate numbers used in the evio file to those
   // stored in the TT. Check here if a local file exists
   // with the name specified by the TT:ROCID_MAP_FILENAME
   // config parameter (default is "rocid.map"). If so,
   // read it in. The format is just 2 values per line.
   // The first is the rocid in the evio file, and the
   // second, what the rocid is in the TT. Note that the
   // value of the crate copied into the data objects 
   // will be what is in the EVIO file.
   ifstream ifs(ROCID_MAP_FILENAME.c_str());
   if (!ifs.is_open()) return;
   
   std::cout << "Opened ROC id translation map: " << ROCID_MAP_FILENAME << std::endl;
   while (ifs.good()) {
      char line[256];
      ifs.getline(line, 256);
      if (ifs.gcount() < 1) break;
      if (line[0] == '#') continue;

      stringstream ss(line);
      uint32_t from=10000, to=10000;
      ss >> from >> to;  // from=evio  to=TT
      if ( to == 10000 ) {
         if ( from != 10000) {
            std::cout << "unable to convert line:" << std::endl;
            std::cout << "  " << line;
         }
      }else{
         Get_ROCID_Map()[from] = to;
         Get_ROCID_Inv_Map()[to] = from;
      }
   }
   ifs.close();
   
   if (Get_ROCID_Map().size() == Get_ROCID_Inv_Map().size()) {
      std::cout << "   Read " << Get_ROCID_Map().size() << " entries" << std::endl;
      map<uint32_t,uint32_t>::iterator iter;
      for (iter=Get_ROCID_Map().begin(); iter != Get_ROCID_Map().end(); iter++) {
         std::cout << "   rocid " << iter->first << " -> rocid "
                    << iter->second << std::endl;
      }
   }else{
      std::cout << "Entries not unique! This can happen if there are"
                << std::endl;
      std::cout << "more than one entry with the same value (either" 
                << std::endl;
      std::cout << "two keys or two vals the same.)" 
                << std::endl;
      std::cout << "Please fix the file \"" << ROCID_MAP_FILENAME << "\"." 
                << std::endl;
      exit(-1);
   }
}

//---------------------------------
// GetSystemsToParse
//---------------------------------
std::set<uint32_t> DTranslationTable::GetSystemsToParse(string systems, int systems_to_parse_force)
{
	/// This takes a string of comma separated system names and
	/// identifies a list of Detector_t values from this (using
	/// strings returned by DetectorName() ). It then tries to
	/// copy the value into the DAQ plugin so they can be used
	/// to restrict which banks to parse.
	std::set<uint32_t> rocids_to_parse;

	// Copy value on how to handle mismatch bewtween CCDB and hard-coded to internal variable
    Get_ROCID_By_System_Mismatch_Behaviour() = systems_to_parse_force;

	if(systems == "") return rocids_to_parse; // nothing to do for empty strings
	jout << "Setting systems to parse to: " << systems << std::endl;

    // Make map of system type id by name
	map<string, Detector_t> name_to_id;
	for(uint32_t dettype=UNKNOWN_DETECTOR; dettype<NUM_DETECTOR_TYPES; dettype++){
		name_to_id[DetectorName((Detector_t)dettype)] = (Detector_t)dettype;
	}

	// There is a chicken-egg problem of reading the ROCID assignments
	// from the CCDB which requires a run number. The run number is
	// not actually available though until parsing of the first event.
	// If the current map of ROCID_By_System is empty, we hold our nose
	// and fill it with the known map. Note that this map will be
	// overwritten later when the XML from the CCDB is parsed. This
	// potentially could lead to inconsistencies. The primary use for
	// this is mostly expcted for parsing only certain systems which
	// is a specialized operation.
	auto &rocid_map = Get_ROCID_By_System();
	if(rocid_map.empty()){
		rocid_map[name_to_id[     "UNKNOWN"]] = {14, 78};
		rocid_map[name_to_id[        "BCAL"]] = {31, 32, 33, 34, 35 ,36, 37, 38, 39, 40, 41, 42};
		rocid_map[name_to_id[         "CDC"]] = {25, 26, 27, 28};
		rocid_map[name_to_id[        "FCAL"]] = {11, 12, 13, 14 ,15, 16, 17, 18, 19, 20, 21, 22};
		rocid_map[name_to_id[        "ECAL"]] = {111, 112, 113, 114 ,115, 116, 117};
		rocid_map[name_to_id["FDC_CATHODES"]] = {52, 53, 55, 56, 57, 58, 59, 60, 61, 62};
		rocid_map[name_to_id[   "FDC_WIRES"]] = {51, 54, 63, 64};
		rocid_map[name_to_id[          "PS"]] = {83, 84};
		rocid_map[name_to_id[         "PSC"]] = {84, 95};
		rocid_map[name_to_id[          "RF"]] = {51, 73, 75, 78, 94, 95};
		rocid_map[name_to_id[          "SC"]] = {94, 95};
		rocid_map[name_to_id[        "TAGH"]] = {73, 75};
		rocid_map[name_to_id[        "TAGM"]] = {71, 75};
		rocid_map[name_to_id[         "TOF"]] = {77, 78};
		rocid_map[name_to_id[        "TPOL"]] = {84};
		rocid_map[name_to_id[         "TAC"]] = {14, 78};
		rocid_map[name_to_id[        "CCAL"]] = {90};
		rocid_map[name_to_id[        "CCAL_REF"]] = {90};
		rocid_map[name_to_id[        "DIRC"]] = {92};
		rocid_map[name_to_id[         "TRD"]] = {76};
		rocid_map[name_to_id[       "FMWPC"]] = {88};
		rocid_map[name_to_id[        "CTOF"]] = {77, 78};
		
	}

	// Parse string of system names
	std::istringstream ss(systems);
	std::string token;
	while(std::getline(ss, token, ',')) {

		// Get initial list of rocids based on token
		set<uint32_t> rocids = rocid_map[name_to_id[token]];

		// Let "FDC" be an alias for both cathode strips and wires
		if(token == "FDC"){
			set<uint32_t> rocids1 = rocid_map[name_to_id["FDC_CATHODES"]];
			set<uint32_t> rocids2 = rocid_map[name_to_id["FDC_WIRES"]];
			rocids.insert(rocids1.begin(), rocids1.end());
			rocids.insert(rocids2.begin(), rocids2.end());
		}

		// More likely than not, someone specifying "PS" will also want "PSC"
		if(token == "PS"){
			set<uint32_t> rocids1 = rocid_map[name_to_id["PSC"]];
			rocids.insert(rocids1.begin(), rocids1.end());
		}

		set<uint32_t>::iterator it;
		for(it=rocids.begin(); it!=rocids.end(); it++){

			// Add this rocid to the DAQ parsing list
			uint32_t rocid = *it;
			rocids_to_parse.insert(rocid);
			jout << "   Added rocid " << rocid << " for system " << token << " to parse list" << std::endl;
		}
	}
	return rocids_to_parse;
}

//---------------------------------
// ApplyTranslationTable
//---------------------------------
void DTranslationTable::ApplyTranslationTable(const std::shared_ptr<const JEvent> &event) const
{
   /// This will get all of the low level objects and
   /// generate detector hit objects from them, placing
   /// them in the appropriate DANA factories.

   if (VERBOSE > 2) ttout << "Entering ApplyTranslationTable:" << std::endl;
   
   // Clear our internal vectors of pointers from previous event
   ClearVectors();
   
   // If the JANA call stack is being recorded, then temporarily disable it
   // so calls we make to loop->Get() here are ignored. The reason we do this
   // is because this routine is called while already in a loop->Get() call
   // so JANA will treat all other loop->Get() calls we make as being dependencies
   // of the loop->Get() call that we are already in. (Confusing eh?) 
   bool record_call_stack = event->GetJCallGraphRecorder()->IsEnabled();
   if (record_call_stack) event->GetJCallGraphRecorder()->SetEnabled(false);

   // Df250PulseIntegral (will apply Df250PulseTime via associated objects)
   vector<const Df250PulseIntegral*> pulseintegrals250;
   event->Get(pulseintegrals250);
   if (VERBOSE > 2) ttout << "  Number Df250PulseIntegral objects: "  << pulseintegrals250.size() << std::endl;
   for (uint32_t i=0; i<pulseintegrals250.size(); i++) {
      const Df250PulseIntegral *pi = pulseintegrals250[i];
      
      // Apply optional rocid translation
      uint32_t rocid = pi->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;
      
      if (VERBOSE > 4)
         ttout << "    Looking for rocid:" << rocid << " slot:" << pi->slot
               << " chan:" << pi->channel << std::endl;
      
      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, pi->slot, pi->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
         if (VERBOSE > 6)
            ttout << "     - Didn't find it" << std::endl;
         continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6)
         ttout << "     - Found entry for: " << DetectorName(chaninfo.det_sys)
               << std::endl;
      
      // Check for a pulse time (this should have been added in JEventSource_EVIO.cc)
      const Df250PulseTime *pt = NULL;
      const Df250PulsePedestal *pp = NULL;
	  pi->GetSingle(pt);
	  pi->GetSingle(pp);

      // Avoid f250 Error with extra PulseIntegral word
      if( pt == NULL || pp == NULL) continue;

      // Create the appropriate hit type based on detector type
      switch (chaninfo.det_sys) {
         case BCAL:       MakeBCALDigiHit(chaninfo.bcal, pi, pt, pp);              break;
         case FCAL:       MakeFCALDigiHit(chaninfo.fcal, pi, pt, pp);              break;
         case ECAL:       MakeECALDigiHit(chaninfo.ecal, pi, pt, pp);              break;
         case CCAL:       MakeCCALDigiHit(chaninfo.ccal, pi, pt, pp);              break;
         case CCAL_REF:   MakeCCALRefDigiHit(chaninfo.ccal_ref, pi, pt, pp);       break;
         case SC:         MakeSCDigiHit(  chaninfo.sc, pi, pt, pp);                break;
         case TOF:        MakeTOFDigiHit( chaninfo.tof, pi, pt, pp);               break;
         case TAGM:       MakeTAGMDigiHit(chaninfo.tagm, pi, pt, pp);              break;
         case TAGH:       MakeTAGHDigiHit(chaninfo.tagh, pi, pt, pp);              break;
         case PS:         MakePSDigiHit(chaninfo.ps, pi, pt, pp);                  break;
         case PSC:        MakePSCDigiHit(chaninfo.psc, pi, pt, pp);                break;
         case RF:         MakeRFDigiTime(chaninfo.rf, pt);                         break;
         case TAC:        MakeTACDigiHit(chaninfo.tac, pi, pt, pp);  	   	   break;
         case CTOF:       MakeCTOFDigiHit(chaninfo.ctof, pi, pt, pp);              break;

         default:
            if (VERBOSE > 4) ttout << "       - Don't know how to make DigiHit objects for this detector type!" << std::endl;
            break;
      }
   }

   // Df250PulseData
   vector<const Df250PulseData*> pulsedatas250;
   event->Get(pulsedatas250);
   if (VERBOSE > 2) ttout << "  Number Df250PulseData objects: "  << pulsedatas250.size() << std::endl;
   for(auto pd : pulsedatas250){
      
      // Apply optional rocid translation
      uint32_t rocid = pd->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;
      
      if (VERBOSE > 4) ttout << "    Looking for rocid:" << rocid << " slot:" << pd->slot << " chan:" << pd->channel << std::endl;
      
      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, pd->slot, pd->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
         if (VERBOSE > 6)  ttout << "     - Didn't find it" << std::endl;
         continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6) ttout << "     - Found entry for: " << DetectorName(chaninfo.det_sys) << std::endl;

      // Create the appropriate hit type based on detector type
      switch (chaninfo.det_sys) {
         case BCAL:       MakeBCALDigiHit( chaninfo.bcal, pd);             break;
         case FCAL:       MakeFCALDigiHit( chaninfo.fcal, pd);             break;
         case ECAL:       MakeECALDigiHit( chaninfo.ecal, pd);             break;
         case CCAL:       MakeCCALDigiHit( chaninfo.ccal, pd);             break;
         case CCAL_REF:   MakeCCALRefDigiHit( chaninfo.ccal_ref, pd);      break;	   
         case SC:         MakeSCDigiHit(   chaninfo.sc,   pd);             break;
         case TOF:        MakeTOFDigiHit(  chaninfo.tof,  pd);             break;
         case TAGM:       MakeTAGMDigiHit( chaninfo.tagm, pd);             break;
         case TAGH:       MakeTAGHDigiHit( chaninfo.tagh, pd);             break;
         case PS:         MakePSDigiHit(   chaninfo.ps,   pd);             break;
         case PSC:        MakePSCDigiHit(  chaninfo.psc,  pd);             break;
         case RF:         MakeRFDigiTime(  chaninfo.rf,   pd);             break;
         case TAC: 	  MakeTACDigiHit(  chaninfo.tac,  pd);  	   break;
         case CTOF:       MakeCTOFDigiHit( chaninfo.ctof, pd);             break;
        default:
            if (VERBOSE > 4) ttout << "       - Don't know how to make DigiHit objects for this detector type!" << std::endl;
            break;
      }
   }

   // Direct creation of DigiHits from Df250WindowRawData for TPOL (always raw mode readout) 
   vector<const Df250WindowRawData*> windowrawdata;
   event->Get(windowrawdata);
   if (VERBOSE > 2) ttout << "  Number Df250WindowRawData objects: " << windowrawdata.size() << std::endl;
   for (uint32_t i=0; i<windowrawdata.size(); i++) {
      const Df250WindowRawData *window = windowrawdata[i];

      // Apply optional rocid translation
      uint32_t rocid = window->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;

      if (VERBOSE > 4)
         ttout << "    Looking for rocid:" << rocid << " slot:" << window->slot
               << " chan:" << window->channel << std::endl;
      
      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, window->slot, window->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
          if (VERBOSE > 6)
             ttout << "     - Didn't find it" << std::endl;
          continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6)
	      ttout << "     - Found entry for: " << DetectorName(chaninfo.det_sys)
               << std::endl;
      
      // Create the appropriate hit type based on detector type
      if(chaninfo.det_sys == TPOLSECTOR) 
	      MakeTPOLSectorDigiHit(chaninfo.tpolsector, window);
   }

   // Df125PulseIntegral (will apply Df125PulseTime via associated objects)
   vector<const Df125PulseIntegral*> pulseintegrals125;
   event->Get(pulseintegrals125);
   if (VERBOSE > 2) ttout << "  Number Df125PulseIntegral objects: " << pulseintegrals125.size() << std::endl;
   for (uint32_t i=0; i<pulseintegrals125.size(); i++) {
      const Df125PulseIntegral *pi = pulseintegrals125[i];

      // Apply optional rocid translation
      uint32_t rocid = pi->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;
      
      if (VERBOSE > 4)
         ttout << "    Looking for rocid:" << rocid << " slot:" << pi->slot
               << " chan:" << pi->channel << std::endl;
   
      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, pi->slot, pi->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
          if (VERBOSE > 6)
             ttout << "     - Didn't find it" << std::endl;
          continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6)
         ttout << "     - Found entry for: " << DetectorName(chaninfo.det_sys) 
               << std::endl;

      // Check for a pulse time (this should have been added in JEventSource_EVIO.cc
      const Df125PulseTime *pt = NULL;
      const Df125PulsePedestal *pp = NULL;
	  pi->GetSingle(pt);
	  pi->GetSingle(pp);

      // Create the appropriate hit type based on detector type
      switch (chaninfo.det_sys) {
         case CDC:           MakeCDCDigiHit(chaninfo.cdc, pi, pt, pp);                 break;
         case FDC_CATHODES:  MakeFDCCathodeDigiHit(chaninfo.fdc_cathodes, pi, pt, pp); break;
         default: 
             if (VERBOSE > 4) ttout << "       - Don't know how to make DigiHit objects for this detector type!" << std::endl;
             break;
      }
   }

   // Df125CDCPulse
   vector<const Df125CDCPulse*> cdcpulses;
   event->Get(cdcpulses);
   if (VERBOSE > 2) ttout << "  Number Df125CDCPulse objects: " << cdcpulses.size() << std::endl;
   for (uint32_t i=0; i<cdcpulses.size(); i++) {
      const Df125CDCPulse *p = cdcpulses[i];

      // Apply optional rocid translation
      uint32_t rocid = p->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;
      
      if (VERBOSE > 4)
         ttout << "    Looking for rocid:" << rocid << " slot:" << p->slot
               << " chan:" << p->channel << std::endl;
   
      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, p->slot, p->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
          if (VERBOSE > 6)
             ttout << "     - Didn't find it" << std::endl;
          continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6)
         ttout << "     - Found entry for: " << DetectorName(chaninfo.det_sys) 
               << std::endl;

      // Create the appropriate hit type based on detector type
      switch (chaninfo.det_sys) {
         case CDC:  MakeCDCDigiHit(chaninfo.cdc, p); break;
         case FMWPC:  MakeFMWPCDigiHit(chaninfo.fmwpc, p); break;
		 //case TRD:  MakeTRDDigiHit(chaninfo.trd, p); break;
         default: 
             if (VERBOSE > 4) ttout << "       - Don't know how to make DigiHit objects for this detector type!" << std::endl;
             break;
      }
   }

   // Df125FDCPulse
   vector<const Df125FDCPulse*> fdcpulses;
   event->Get(fdcpulses);
   if (VERBOSE > 2) ttout << "  Number Df125FDCPulse objects: " << fdcpulses.size() << std::endl;
   for (uint32_t i=0; i<fdcpulses.size(); i++) {
      const Df125FDCPulse *p = fdcpulses[i];

      // Apply optional rocid translation
      uint32_t rocid = p->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;
      
      if (VERBOSE > 4)
         ttout << "    Looking for rocid:" << rocid << " slot:" << p->slot
               << " chan:" << p->channel << std::endl;
   
      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, p->slot, p->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
          if (VERBOSE > 6)
             ttout << "     - Didn't find it" << std::endl;
          continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6)
         ttout << "     - Found entry for: " << DetectorName(chaninfo.det_sys) 
               << std::endl;

      // Create the appropriate hit type based on detector type
      switch (chaninfo.det_sys) {
         case FDC_CATHODES:  MakeFDCCathodeDigiHit(chaninfo.fdc_cathodes, p); break;
         case CDC         :  MakeCDCDigiHit(chaninfo.cdc, p); break;
	 case TRD         :  MakeTRDDigiHit(chaninfo.trd, p); break;
         default: 
             if (VERBOSE > 4) ttout << "       - Don't know how to make DigiHit objects for this detector type!" << std::endl;
             break;
      }
   }

   // DF1TDCHit
   vector<const DF1TDCHit*> f1tdchits;
   event->Get(f1tdchits);
   if (VERBOSE > 2) ttout << "  Number DF1TDCHit objects: " << f1tdchits.size() << std::endl;
   for (uint32_t i=0; i<f1tdchits.size(); i++) {
      const DF1TDCHit *hit = f1tdchits[i];

      // Apply optional rocid translation
      uint32_t rocid = hit->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;

      if (VERBOSE > 4)
         ttout << "    Looking for rocid:" << rocid << " slot:" << hit->slot
               << " chan:" << hit->channel << std::endl;

      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, hit->slot, hit->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
          if (VERBOSE > 6)
             ttout << "     - Didn't find it" << std::endl;
          continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6) 
         ttout << "     - Found entry for: " 
               << DetectorName(chaninfo.det_sys) << std::endl;
      
      // Create the appropriate hit type based on detector type
      switch (chaninfo.det_sys) {
         case BCAL:        MakeBCALTDCDigiHit(chaninfo.bcal, hit);      break;
         case FDC_WIRES:   MakeFDCWireDigiHit(chaninfo.fdc_wires, hit); break;
         case RF:          MakeRFTDCDigiTime(chaninfo.rf, hit);         break;
         case SC:          MakeSCTDCDigiHit(chaninfo.sc, hit);          break;
         case TAGM:        MakeTAGMTDCDigiHit(chaninfo.tagm, hit);      break;
         case TAGH:        MakeTAGHTDCDigiHit(chaninfo.tagh, hit);      break;
         case PSC:         MakePSCTDCDigiHit(chaninfo.psc, hit);        break;
        default:
             if (VERBOSE > 4) ttout << "       - Don't know how to make DigiHit objects for this detector type!" << std::endl;
             break;
      }
   }

   // DCAEN1290TDCHit
   vector<const DCAEN1290TDCHit*> caen1290tdchits;
   event->Get(caen1290tdchits);
   if (VERBOSE > 2) ttout << "  Number DCAEN1290TDCHit objects: " << caen1290tdchits.size() << std::endl;
   for (uint32_t i=0; i<caen1290tdchits.size(); i++) {
      const DCAEN1290TDCHit *hit = caen1290tdchits[i];

      // Apply optional rocid translation
      uint32_t rocid = hit->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;

      if (VERBOSE > 4)
         ttout << "    Looking for rocid:" << rocid << " slot:" << hit->slot
               << " chan:" << hit->channel << std::endl;
      
      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, hit->slot, hit->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
          if (VERBOSE > 6)
             ttout << "     - Didn't find it" << std::endl;
          continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6)
         ttout << "     - Found entry for: " << DetectorName(chaninfo.det_sys)
               << std::endl;
      
      // Create the appropriate hit type based on detector type
      switch (chaninfo.det_sys) {
         case TOF:    MakeTOFTDCDigiHit(chaninfo.tof, hit); break;
         case RF:     MakeRFTDCDigiTime(chaninfo.rf, hit);  break;
         case TAC:    MakeTACTDCDigiHit(chaninfo.tac, hit); break;
         case CTOF:   MakeCTOFTDCDigiHit(chaninfo.ctof, hit); break;
         default:     
             if (VERBOSE > 4) ttout << "       - Don't know how to make DigiHit objects for this detector type!" << std::endl;
             break;
      }
   }

   // DDIRCTDCHit
   vector<const DDIRCTDCHit*> dirctdchits;
   event->Get(dirctdchits);
   if (VERBOSE > 2) ttout << "  Number DDIRCTDCHit objects: " << dirctdchits.size() << std::endl;
   for (uint32_t i=0; i<dirctdchits.size(); i++) {
      const DDIRCTDCHit *hit = dirctdchits[i];

      // Apply optional rocid translation
      uint32_t rocid = hit->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;

      if (VERBOSE > 4)
         ttout << "    Looking for rocid:" << rocid << " slot:" << hit->slot
               << " chan:" << hit->channel << std::endl;
      
      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, hit->slot, hit->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
          if (VERBOSE > 6)
             ttout << "     - Didn't find it" << std::endl;
          continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6)
	      ttout << "     - Found entry for: " << DetectorName(chaninfo.det_sys)
               << std::endl;
      
      // Create the appropriate hit type based on detector type
      switch (chaninfo.det_sys) {
         case DIRC:    MakeDIRCTDCDigiHit(chaninfo.dirc, hit); break;
         default:     
             if (VERBOSE > 4) ttout << "       - Don't know how to make DigiHit objects for this detector type!" << std::endl;
             break;
      }
   }

   // DGEMSRSWindowRawData
   vector<const DGEMSRSWindowRawData*> gemsrswindowrawdata;
   event->Get(gemsrswindowrawdata);
   if (VERBOSE > 2) ttout << "  Number DGEMSRSWindowRawData objects: " << gemsrswindowrawdata.size() << std::endl;
   for (uint32_t i=0; i<gemsrswindowrawdata.size(); i++) {
      const DGEMSRSWindowRawData *hit = gemsrswindowrawdata[i];

      // Apply optional rocid translation
      uint32_t rocid = hit->rocid;
      map<uint32_t, uint32_t>::iterator rocid_iter = Get_ROCID_Map().find(rocid);
      if (rocid_iter != Get_ROCID_Map().end()) rocid = rocid_iter->second;

      if (VERBOSE > 4)
         ttout << "    Looking for rocid:" << rocid << " slot:" << hit->slot
               << " chan:" << hit->channel << std::endl;
      
      // Create crate,slot,channel index and find entry in Translation table.
      // If none is found, then just quietly skip this hit.
      csc_t csc = {rocid, hit->slot, hit->channel};
      map<csc_t, DChannelInfo>::const_iterator iter = Get_TT().find(csc);
      if (iter == Get_TT().end()) {
          if (VERBOSE > 6)
             ttout << "     - Didn't find it" << std::endl;
          continue;
      }
      const DChannelInfo &chaninfo = iter->second;
      if (VERBOSE > 6)
	      ttout << "     - Found entry for: " << DetectorName(chaninfo.det_sys)
               << std::endl;
      
      // Create the appropriate hit type based on detector type
      switch (chaninfo.det_sys) {
         case TRD:    MakeGEMDigiWindowRawData(chaninfo.trd, hit); break;
         default:     
             if (VERBOSE > 4) ttout << "       - Don't know how to make DigiHit objects for this detector type!" << std::endl;
             break;
      }
   }

   // Optionally overwrite nsamples_integral and/or nsamples_pedestal if 
   // user specified via config. parameters.
   OverwriteNsamples();
   
   // Sort object order (this makes it easier to browse with hd_dump)
   sort(vDBCALDigiHit.begin(), vDBCALDigiHit.end(), SortBCALDigiHit);
   
   // Copy pointers to all objects produced to their appropriate
   // factories. This hands ownership of them over to the factories
   // so JANA will delete them.
   CopyToFactories(const_cast<JEvent*>(&(*event))); // TODO: NWB: Make this less hacky
   
   
	if (VERBOSE > 3) PrintVectorSizes();

   // Add to JANA's call stack some entries to make janadot draw something reasonable
   // Unfortunately, this is just us telling JANA the relationship as defined here.
   // It is not derived from the above code which would guarantee the declared relationsips
   // are correct. That would just be too complicated given how that code works.
   if (record_call_stack) {
      // re-enable call stack recording
      event->GetJCallGraphRecorder()->SetEnabled(true);

		if(CALL_STACK){
      	Addf250ObjectsToCallStack(*event, "DBCALDigiHit");
      	Addf250ObjectsToCallStack(*event, "DFCALDigiHit");
      	Addf250ObjectsToCallStack(*event, "DECALDigiHit");
      	Addf250ObjectsToCallStack(*event, "DCCALDigiHit");
      	Addf250ObjectsToCallStack(*event, "DCCALRefDigiHit");
      	Addf250ObjectsToCallStack(*event, "DSCDigiHit");
      	Addf250ObjectsToCallStack(*event, "DTOFDigiHit");
      	Addf250ObjectsToCallStack(*event, "DCTOFDigiHit");
      	Addf250ObjectsToCallStack(*event, "DTACDigiHit");
      	Addf125CDCObjectsToCallStack(*event, "DCDCDigiHit", cdcpulses.size()>0);
      	Addf125FDCObjectsToCallStack(*event, "DFDCCathodeDigiHit", fdcpulses.size()>0);
      	Addf125CDCObjectsToCallStack(*event, "DFMWPCDigiHit", cdcpulses.size()>0);
      	AddF1TDCObjectsToCallStack(*event, "DBCALTDCDigiHit");
      	AddF1TDCObjectsToCallStack(*event, "DFDCWireDigiHit");
      	AddF1TDCObjectsToCallStack(*event, "DRFDigiTime");
      	AddF1TDCObjectsToCallStack(*event, "DRFTDCDigiTime");
      	AddF1TDCObjectsToCallStack(*event, "DSCTDCDigiHit");
      	AddCAEN1290TDCObjectsToCallStack(*event, "DTOFTDCDigiHit");
      	AddCAEN1290TDCObjectsToCallStack(*event, "DTACTDCDigiHit");
      	AddCAEN1290TDCObjectsToCallStack(*event, "DFWMPCDigiHit");
      	AddCAEN1290TDCObjectsToCallStack(*event, "DCTOFTDCDigiHit");
		}
   }
}

//---------------------------------
// MakeBCALDigiHit
//---------------------------------
DBCALDigiHit* DTranslationTable::MakeBCALDigiHit(const BCALIndex_t &idx,
                                                 const Df250PulseData *pd) const
{
   if (VERBOSE > 4)
      ttout << "       - Making DBCALDigiHit for (mod,lay,sec,end)=("
            << idx.module << "," << idx.layer << "," << idx.sector 
            << "," << (DBCALGeometry::End)idx.end << std::endl;

   DBCALDigiHit *h = new DBCALDigiHit();
   CopyDf250Info(h, pd);

   h->module = idx.module;
   h->layer  = idx.layer;
   h->sector = idx.sector;
   h->end    = (DBCALGeometry::End)idx.end;

   vDBCALDigiHit.push_back(h);

   return h;
}

//---------------------------------
// MakeFCALDigiHit
//---------------------------------
DFCALDigiHit* DTranslationTable::MakeFCALDigiHit(const FCALIndex_t &idx,
                                                 const Df250PulseData *pd) const
{
   DFCALDigiHit *h = new DFCALDigiHit();
   CopyDf250Info(h, pd);

   h->row    = idx.row;
   h->column = idx.col;

   vDFCALDigiHit.push_back(h);
   
   return h;
}


//---------------------------------
// MakeECALDigiHit
//---------------------------------
DECALDigiHit* DTranslationTable::MakeECALDigiHit(const ECALIndex_t &idx,
                                                 const Df250PulseData *pd) const
{
   DECALDigiHit *h = new DECALDigiHit();
   CopyDf250Info(h, pd);

   // The ECAL coordinate system: (column,row) = (0,0) in the bottom right corner

   if(idx.col < 0)
     h->column = idx.col + 20;
   else if(idx.col > 0) 
     h->column = idx.col + 19;
   
   if(idx.row < 0)
     h->row = idx.row + 20;
   else if(idx.row > 0) 
     h->row = idx.row + 19;
   
   vDECALDigiHit.push_back(h);
   
   return h;
}



//---------------------------------
// MakeCCALDigiHit
//---------------------------------
DCCALDigiHit* DTranslationTable::MakeCCALDigiHit(const CCALIndex_t &idx,
                                                 const Df250PulseData *pd) const
{
   DCCALDigiHit *h = new DCCALDigiHit();
   CopyDf250Info(h, pd);

   // The CCAL coordinate system: (column,row) = (0,0) in the bottom right corner

   if(idx.col < 0)
     h->column = idx.col + 6;
   else if(idx.col > 0) 
     h->column = idx.col + 5;
   
   if(idx.row < 0)
     h->row = idx.row + 6;
   else if(idx.row > 0) 
     h->row = idx.row + 5;
   
   //   h->row    = idx.row;
   //   h->column = idx.col;
   
   vDCCALDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeCCALRefDigiHit
//---------------------------------
DCCALRefDigiHit* DTranslationTable::MakeCCALRefDigiHit(const CCALRefIndex_t &idx,
                                                 const Df250PulseData *pd) const
{
   DCCALRefDigiHit *h = new DCCALRefDigiHit();
   CopyDf250Info(h, pd);

   h->id     = idx.id;

   vDCCALRefDigiHit.push_back(h);
   
   return h;
}


//---------------------------------
// MakeTOFDigiHit
//---------------------------------
DTOFDigiHit* DTranslationTable::MakeTOFDigiHit(const TOFIndex_t &idx,
                                               const Df250PulseData *pd) const
{
   DTOFDigiHit *h = new DTOFDigiHit();
   CopyDf250Info(h, pd);

   h->plane = idx.plane;
   h->bar   = idx.bar;
   h->end   = idx.end;

   vDTOFDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeCTOFDigiHit
//---------------------------------
DCTOFDigiHit* DTranslationTable::MakeCTOFDigiHit(const CTOFIndex_t &idx,
                                               const Df250PulseData *pd) const
{
   DCTOFDigiHit *h = new DCTOFDigiHit();
   CopyDf250Info(h, pd);

   h->plane = idx.plane;
   h->bar   = idx.bar;
   h->end   = idx.end;

   vDCTOFDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeSCDigiHit
//---------------------------------
DSCDigiHit* DTranslationTable::MakeSCDigiHit(const SCIndex_t &idx, 
                                             const Df250PulseData *pd) const
{
   DSCDigiHit *h = new DSCDigiHit();
   CopyDf250Info(h, pd);

   h->sector = idx.sector;

   vDSCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTAGMDigiHit
//---------------------------------
DTAGMDigiHit* DTranslationTable::MakeTAGMDigiHit(const TAGMIndex_t &idx,
                                                 const Df250PulseData *pd) const
{
   DTAGMDigiHit *h = new DTAGMDigiHit();
   CopyDf250Info(h, pd);

   h->row = idx.row;
   h->column = idx.col;

   vDTAGMDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTAGHDigiHit
//---------------------------------
DTAGHDigiHit* DTranslationTable::MakeTAGHDigiHit(const TAGHIndex_t &idx,
                                                 const Df250PulseData *pd) const
{
   DTAGHDigiHit *h = new DTAGHDigiHit();
   CopyDf250Info(h, pd);

   h->counter_id = idx.id;

   vDTAGHDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakePSCDigiHit
//---------------------------------
DPSCDigiHit* DTranslationTable::MakePSCDigiHit(const PSCIndex_t &idx,
											   const Df250PulseData *pd) const
{
   DPSCDigiHit *h = new DPSCDigiHit();
   CopyDf250Info(h, pd);

   h->counter_id = idx.id;

   vDPSCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakePSDigiHit
//---------------------------------
DPSDigiHit* DTranslationTable::MakePSDigiHit(const PSIndex_t &idx,
                                             const Df250PulseData *pd) const
{
   DPSDigiHit *h = new DPSDigiHit();
   CopyDf250Info(h, pd);

   h->arm = (DPSGeometry::Arm)idx.side;
   h->column = idx.id;

   vDPSDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeBCALDigiHit
//---------------------------------
DBCALDigiHit* DTranslationTable::MakeBCALDigiHit(const BCALIndex_t &idx,
                                                 const Df250PulseIntegral *pi,
                                                 const Df250PulseTime *pt,
                                                 const Df250PulsePedestal *pp) const
{
   if (VERBOSE > 4)
      ttout << "       - Making DBCALDigiHit for (mod,lay,sec,end)=("
            << idx.module << "," << idx.layer << "," << idx.sector 
            << "," << (DBCALGeometry::End)idx.end << std::endl;

   DBCALDigiHit *h = new DBCALDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->pulse_peak = pp==NULL ? 0 : pp->pulse_peak; // Include pulse peak information in the digihit for BCAL

   h->module = idx.module;
   h->layer  = idx.layer;
   h->sector = idx.sector;
   h->end    = (DBCALGeometry::End)idx.end;

   vDBCALDigiHit.push_back(h);

   return h;
}

//---------------------------------
// MakeFCALDigiHit
//---------------------------------
DFCALDigiHit* DTranslationTable::MakeFCALDigiHit(const FCALIndex_t &idx,
                                                 const Df250PulseIntegral *pi,
                                                 const Df250PulseTime *pt,
                                                 const Df250PulsePedestal *pp) const
{
   DFCALDigiHit *h = new DFCALDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->row    = idx.row;
   h->column = idx.col;

   vDFCALDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeECALDigiHit
//---------------------------------
DECALDigiHit* DTranslationTable::MakeECALDigiHit(const ECALIndex_t &idx,
                                                 const Df250PulseIntegral *pi,
                                                 const Df250PulseTime *pt,
                                                 const Df250PulsePedestal *pp) const
{
   DECALDigiHit *h = new DECALDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   if(idx.col < 0)
     h->column = idx.col + 20;
   else if(idx.col > 0) 
     h->column = idx.col + 19;

   if(idx.row < 0)
     h->row = idx.row + 20;
   else if(idx.row > 0) 
     h->row = idx.row + 19;

   vDECALDigiHit.push_back(h);
   
   return h;
}



//---------------------------------
// MakeCCALDigiHit
//---------------------------------
DCCALDigiHit* DTranslationTable::MakeCCALDigiHit(const CCALIndex_t &idx,
                                                 const Df250PulseIntegral *pi,
                                                 const Df250PulseTime *pt,
                                                 const Df250PulsePedestal *pp) const
{
   DCCALDigiHit *h = new DCCALDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   if(idx.col < 0)
     h->column = idx.col + 6;
   else if(idx.col > 0) 
     h->column = idx.col + 5;

   if(idx.row < 0)
     h->row = idx.row + 6;
   else if(idx.row > 0) 
     h->row = idx.row + 5;

   //   h->row    = idx.row;
   //   h->column = idx.col;

   vDCCALDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeCCALRefDigiHit
//---------------------------------
DCCALRefDigiHit* DTranslationTable::MakeCCALRefDigiHit(const CCALRefIndex_t &idx,
                                                 const Df250PulseIntegral *pi,
                                                 const Df250PulseTime *pt,
                                                 const Df250PulsePedestal *pp) const
{
   DCCALRefDigiHit *h = new DCCALRefDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->id    = idx.id;

   vDCCALRefDigiHit.push_back(h);
   
   return h;
}


//---------------------------------
// MakeTOFDigiHit
//---------------------------------
DTOFDigiHit* DTranslationTable::MakeTOFDigiHit(const TOFIndex_t &idx,
                                               const Df250PulseIntegral *pi,
                                               const Df250PulseTime *pt,
                                               const Df250PulsePedestal *pp) const
{
   DTOFDigiHit *h = new DTOFDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->plane = idx.plane;
   h->bar   = idx.bar;
   h->end   = idx.end;

   vDTOFDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTOFDigiHit
//---------------------------------
DCTOFDigiHit* DTranslationTable::MakeCTOFDigiHit(const CTOFIndex_t &idx,
                                               const Df250PulseIntegral *pi,
                                               const Df250PulseTime *pt,
                                               const Df250PulsePedestal *pp) const
{
   DCTOFDigiHit *h = new DCTOFDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->plane = idx.plane;
   h->bar   = idx.bar;
   h->end   = idx.end;

   vDCTOFDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeSCDigiHit
//---------------------------------
DSCDigiHit* DTranslationTable::MakeSCDigiHit(const SCIndex_t &idx, 
                                             const Df250PulseIntegral *pi,
                                             const Df250PulseTime *pt,
                                             const Df250PulsePedestal *pp) const
{
   DSCDigiHit *h = new DSCDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->sector = idx.sector;

   vDSCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTAGMDigiHit
//---------------------------------
DTAGMDigiHit* DTranslationTable::MakeTAGMDigiHit(const TAGMIndex_t &idx,
                                                 const Df250PulseIntegral *pi,
                                                 const Df250PulseTime *pt,
                                                 const Df250PulsePedestal *pp) const
{
   DTAGMDigiHit *h = new DTAGMDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->row = idx.row;
   h->column = idx.col;

   vDTAGMDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTAGHDigiHit
//---------------------------------
DTAGHDigiHit* DTranslationTable::MakeTAGHDigiHit(const TAGHIndex_t &idx,
                                                 const Df250PulseIntegral *pi,
                                                 const Df250PulseTime *pt,
                                                 const Df250PulsePedestal *pp) const
{
   DTAGHDigiHit *h = new DTAGHDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->counter_id = idx.id;

   vDTAGHDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakePSCDigiHit
//---------------------------------
DPSCDigiHit* DTranslationTable::MakePSCDigiHit(const PSCIndex_t &idx,
					       const Df250PulseIntegral *pi,
					       const Df250PulseTime *pt,
					       const Df250PulsePedestal *pp) const
{
   DPSCDigiHit *h = new DPSCDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->counter_id = idx.id;

   vDPSCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakePSDigiHit
//---------------------------------
DPSDigiHit* DTranslationTable::MakePSDigiHit(const PSIndex_t &idx,
					     const Df250PulseIntegral *pi,
					     const Df250PulseTime *pt,
					     const Df250PulsePedestal *pp) const
{
   DPSDigiHit *h = new DPSDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->arm = (DPSGeometry::Arm)idx.side;
   h->column = idx.id;

   vDPSDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeCDCDigiHit
//---------------------------------
DCDCDigiHit* DTranslationTable::MakeCDCDigiHit(const CDCIndex_t &idx,
                                               const Df125PulseIntegral *pi,
                                               const Df125PulseTime *pt,
                                               const Df125PulsePedestal *pp) const
{
   DCDCDigiHit *h = new DCDCDigiHit();
   CopyDf125Info(h, pi, pt, pp);

   h->ring = idx.ring;
   h->straw = idx.straw;
	h->pulse_peak = 0;
   
   vDCDCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeCDCDigiHit
//---------------------------------
DCDCDigiHit* DTranslationTable::MakeCDCDigiHit(const CDCIndex_t &idx,
                                               const Df125CDCPulse *p) const
{
	DCDCDigiHit *h = new DCDCDigiHit();
	h->ring              = idx.ring;
	h->straw             = idx.straw;
	h->pulse_peak        = p->first_max_amp;
	h->pulse_integral    = p->integral;
	h->pulse_time        = p->le_time;
	h->pedestal          = p->pedestal;
	h->QF                = p->time_quality_bit + (p->overflow_count<<1);
	h->nsamples_integral = p->nsamples_integral;
	h->nsamples_pedestal = p->nsamples_pedestal;

	h->AddAssociatedObject(p);

	vDCDCDigiHit.push_back(h);
   
	return h;
}

//---------------------------------
// MakeCDCDigiHit
//---------------------------------
DCDCDigiHit* DTranslationTable::MakeCDCDigiHit(const CDCIndex_t &idx,
                                               const Df125FDCPulse *p) const
{
	DCDCDigiHit *h = new DCDCDigiHit();
	h->ring              = idx.ring;
	h->straw             = idx.straw;
	h->pulse_peak        = p->peak_amp;
	h->pulse_integral    = p->integral;
	h->pulse_time        = p->le_time;
	h->pedestal          = p->pedestal;
	h->QF                = p->time_quality_bit + (p->overflow_count<<1);
	h->nsamples_integral = p->nsamples_integral;
	h->nsamples_pedestal = p->nsamples_pedestal;

	h->AddAssociatedObject(p);

	vDCDCDigiHit.push_back(h);
   
	return h;
}

//---------------------------------
// MakeFDCCathodeDigiHit
//---------------------------------
DFDCCathodeDigiHit* DTranslationTable::MakeFDCCathodeDigiHit(
                                       const FDC_CathodesIndex_t &idx,
                                       const Df125PulseIntegral *pi,
                                       const Df125PulseTime *pt,
                                       const Df125PulsePedestal *pp) const
{
   DFDCCathodeDigiHit *h = new DFDCCathodeDigiHit();
   CopyDf125Info(h, pi, pt, pp);

   h->package    = idx.package;
   h->chamber    = idx.chamber;
   h->view       = idx.view;
   h->strip      = idx.strip;
   h->strip_type = idx.strip_type;

   vDFDCCathodeDigiHit.push_back(h);
   
   return h;
}


//---------------------------------
// MakeFDCCathodeDigiHit
//---------------------------------
DFDCCathodeDigiHit* DTranslationTable::MakeFDCCathodeDigiHit(
                                       const FDC_CathodesIndex_t &idx,
                                       const Df125FDCPulse *p) const
{
	DFDCCathodeDigiHit *h = new DFDCCathodeDigiHit();
	h->package           = idx.package;
	h->chamber           = idx.chamber;
	h->view              = idx.view;
	h->strip             = idx.strip;
	h->strip_type        = idx.strip_type;
	h->pulse_integral    = p->integral;
	h->pulse_time        = p->le_time;
	h->pedestal          = p->pedestal;
	h->QF                = p->time_quality_bit + (p->overflow_count<<1);
	h->nsamples_integral = p->nsamples_integral;
	h->nsamples_pedestal = p->nsamples_pedestal;

	h->AddAssociatedObject(p);

	vDFDCCathodeDigiHit.push_back(h);
   
	return h;
}

//---------------------------------
// MakeTRDDigiHit
//---------------------------------
DTRDDigiHit* DTranslationTable::MakeTRDDigiHit(
                                       const TRDIndex_t &idx,
                                       const Df125CDCPulse *p) const
{
	DTRDDigiHit *h = new DTRDDigiHit();
	h->plane             = idx.plane;
	h->strip             = idx.strip;
	h->pulse_peak        = p->first_max_amp;
	h->pulse_time        = p->le_time;
	h->pedestal          = p->pedestal;
	h->QF                = p->time_quality_bit + (p->overflow_count<<1);
	h->nsamples_integral = p->nsamples_integral;
	h->nsamples_pedestal = p->nsamples_pedestal;

	h->AddAssociatedObject(p);

	vDTRDDigiHit.push_back(h);
   
	return h;
}

//---------------------------------
// MakeTRDDigiHit
//---------------------------------
DTRDDigiHit* DTranslationTable::MakeTRDDigiHit(
                                       const TRDIndex_t &idx,
                                       const Df125FDCPulse *p) const
{
	DTRDDigiHit *h = new DTRDDigiHit();
	h->plane             = idx.plane;
	h->strip             = idx.strip;
	h->pulse_peak        = p->peak_amp;
	h->pulse_time        = p->le_time;
	h->pedestal          = p->pedestal;
	h->QF                = p->time_quality_bit + (p->overflow_count<<1);
	h->nsamples_integral = p->nsamples_integral;
	h->nsamples_pedestal = p->nsamples_pedestal;

	h->AddAssociatedObject(p);

	vDTRDDigiHit.push_back(h);
   
	return h;
}

//---------------------------------
// MakeFMWPCDigiHit
//---------------------------------
DFMWPCDigiHit* DTranslationTable::MakeFMWPCDigiHit(const FMWPCIndex_t &idx,
                                                 const Df125CDCPulse *p) const
{
	DFMWPCDigiHit *h = new DFMWPCDigiHit();
	h->layer             = idx.layer;
	h->wire              = idx.wire;
	h->pulse_peak        = p->first_max_amp;
	h->pulse_integral    = p->integral;
	h->pulse_time        = p->le_time;
	h->pedestal          = p->pedestal;
	h->QF                = p->time_quality_bit + (p->overflow_count<<1);
	h->nsamples_integral = p->nsamples_integral;
	h->nsamples_pedestal = p->nsamples_pedestal;

	h->AddAssociatedObject(p);

	vDFMWPCDigiHit.push_back(h);
   
	return h;
}


//---------------------------------
// MakeDigiWindowRawData
//---------------------------------
DGEMDigiWindowRawData* DTranslationTable::MakeGEMDigiWindowRawData(
                                       const TRDIndex_t &idx,
                                       const DGEMSRSWindowRawData *p) const
{
	DGEMDigiWindowRawData *h = new DGEMDigiWindowRawData();
	h->plane             = idx.plane;
	h->strip             = idx.strip;

	h->AddAssociatedObject(p);

	vDGEMDigiWindowRawData.push_back(h);
   
	return h;
}

//---------------------------------
// MakeBCALTDCDigiHit
//---------------------------------
DBCALTDCDigiHit* DTranslationTable::MakeBCALTDCDigiHit(
                                    const BCALIndex_t &idx,
                                    const DF1TDCHit *hit) const
{
   DBCALTDCDigiHit *h = new DBCALTDCDigiHit();
   CopyDF1TDCInfo(h, hit);

   h->module = idx.module;
   h->layer  = idx.layer;
   h->sector = idx.sector;
   h->end    = (DBCALGeometry::End)idx.end;

   vDBCALTDCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeFDCWireDigiHit
//---------------------------------
DFDCWireDigiHit* DTranslationTable::MakeFDCWireDigiHit(
                                    const FDC_WiresIndex_t &idx,
                                    const DF1TDCHit *hit) const
{
   DFDCWireDigiHit *h = new DFDCWireDigiHit();
   CopyDF1TDCInfo(h, hit);

   h->package = idx.package;
   h->chamber = idx.chamber;
   h->wire    = idx.wire;

   vDFDCWireDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeRFDigiTime
//---------------------------------
DRFTDCDigiTime*  DTranslationTable::MakeRFTDCDigiTime(
                                   const RFIndex_t &idx,
                                   const DF1TDCHit *hit) const
{
   DRFTDCDigiTime *h = new DRFTDCDigiTime();
   CopyDF1TDCInfo(h, hit);

   h->dSystem = idx.dSystem;
   h->dIsCAENTDCFlag = false;

   vDRFTDCDigiTime.push_back(h);
   
   return h;
}

//---------------------------------
// MakeRFDigiTime
//---------------------------------
DRFTDCDigiTime*  DTranslationTable::MakeRFTDCDigiTime(
                                   const RFIndex_t &idx,
                                   const DCAEN1290TDCHit *hit) const
{
   DRFTDCDigiTime *h = new DRFTDCDigiTime();
   CopyDCAEN1290TDCInfo(h, hit);

   h->dSystem = idx.dSystem;
   h->dIsCAENTDCFlag = true;

   vDRFTDCDigiTime.push_back(h);
   
   return h;
}

//---------------------------------
// MakeRFDigiTime
//---------------------------------
DRFDigiTime*  DTranslationTable::MakeRFDigiTime(
                                   const RFIndex_t &idx,
                                   const Df250PulseTime *hit) const
{
   DRFDigiTime *h = new DRFDigiTime();
   h->time = hit->time;

   h->dSystem = idx.dSystem;

   vDRFDigiTime.push_back(h);
   
   return h;
}

//---------------------------------
// MakeRFDigiTime
//---------------------------------
DRFDigiTime*  DTranslationTable::MakeRFDigiTime(
                                   const RFIndex_t &idx,
                                   const Df250PulseData *hit) const
{
   DRFDigiTime *h = new DRFDigiTime();
   h->time = (hit->course_time<<6) + hit->fine_time;

   h->dSystem = idx.dSystem;

   vDRFDigiTime.push_back(h);
   
   return h;
}

//---------------------------------
// MakeSCTDCDigiHit
//---------------------------------
DSCTDCDigiHit*  DTranslationTable::MakeSCTDCDigiHit(
                                   const SCIndex_t &idx,
                                   const DF1TDCHit *hit) const
{
   DSCTDCDigiHit *h = new DSCTDCDigiHit();
   CopyDF1TDCInfo(h, hit);

   h->sector = idx.sector;

   vDSCTDCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTAGMTDCDigiHit
//---------------------------------
DTAGMTDCDigiHit*  DTranslationTable::MakeTAGMTDCDigiHit(
                                     const TAGMIndex_t &idx,
                                     const DF1TDCHit *hit) const
{
   DTAGMTDCDigiHit *h = new DTAGMTDCDigiHit();
   CopyDF1TDCInfo(h, hit);

   h->row = idx.row;
   h->column = idx.col;

   vDTAGMTDCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTAGHTDCDigiHit
//---------------------------------
DTAGHTDCDigiHit*  DTranslationTable::MakeTAGHTDCDigiHit(
                                     const TAGHIndex_t &idx,
                                     const DF1TDCHit *hit) const
{
   DTAGHTDCDigiHit *h = new DTAGHTDCDigiHit();
   CopyDF1TDCInfo(h, hit);

   h->counter_id = idx.id;

   vDTAGHTDCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakePSCTDCDigiHit
//---------------------------------
DPSCTDCDigiHit*  DTranslationTable::MakePSCTDCDigiHit(
                                     const PSCIndex_t &idx,
                                     const DF1TDCHit *hit) const
{
   DPSCTDCDigiHit *h = new DPSCTDCDigiHit();
   CopyDF1TDCInfo(h, hit);

   h->counter_id = idx.id;

   vDPSCTDCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTOFTDCDigiHit
//---------------------------------
DTOFTDCDigiHit*  DTranslationTable::MakeTOFTDCDigiHit(
                                    const TOFIndex_t &idx,
                                    const DCAEN1290TDCHit *hit) const
{
   DTOFTDCDigiHit *h = new DTOFTDCDigiHit();
   CopyDCAEN1290TDCInfo(h, hit);

   h->plane = idx.plane;
   h->bar   = idx.bar;
   h->end   = idx.end;

   vDTOFTDCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTOFTDCDigiHit
//---------------------------------
DCTOFTDCDigiHit*  DTranslationTable::MakeCTOFTDCDigiHit(
                                    const CTOFIndex_t &idx,
                                    const DCAEN1290TDCHit *hit) const
{
   DCTOFTDCDigiHit *h = new DCTOFTDCDigiHit();
   CopyDCAEN1290TDCInfo(h, hit);

   h->plane = idx.plane;
   h->bar   = idx.bar;
   h->end   = idx.end;

   vDCTOFTDCDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTPOLSectorDigiHit
//---------------------------------
DTPOLSectorDigiHit* DTranslationTable::MakeTPOLSectorDigiHit(const TPOLSECTORIndex_t &idx,
							     const Df250PulseIntegral *pi,
							     const Df250PulseTime *pt,
							     const Df250PulsePedestal *pp) const
{
   DTPOLSectorDigiHit *h = new DTPOLSectorDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   h->sector = idx.sector;

   vDTPOLSectorDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTPOLSectorDigiHit
//---------------------------------
DTPOLSectorDigiHit* DTranslationTable::MakeTPOLSectorDigiHit(const TPOLSECTORIndex_t &idx,
							     const Df250PulseData *pd) const
{
   DTPOLSectorDigiHit *h = new DTPOLSectorDigiHit();
   CopyDf250Info(h, pd);

   h->sector = idx.sector;

   vDTPOLSectorDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTPOLSectorDigiHit
//---------------------------------
DTPOLSectorDigiHit* DTranslationTable::MakeTPOLSectorDigiHit(const TPOLSECTORIndex_t &idx,
							     const Df250WindowRawData *window) const
{
   DTPOLSectorDigiHit *h = new DTPOLSectorDigiHit();

   h->sector = idx.sector;
   h->AddAssociatedObject(window);

   vDTPOLSectorDigiHit.push_back(h);
   
   return h;
}

//---------------------------------
// MakeTACDigiHit
//---------------------------------
DTACDigiHit* DTranslationTable::MakeTACDigiHit(const TACIndex_t &idx,
							     const Df250PulseIntegral *pi,
							     const Df250PulseTime *pt,
							     const Df250PulsePedestal *pp) const
{
   DTACDigiHit *h = new DTACDigiHit();
   CopyDf250Info(h, pi, pt, pp);

   vDTACDigiHit.push_back(h);

   return h;
}

//---------------------------------
// MakeTACDigiHit
//---------------------------------
DTACDigiHit* DTranslationTable::MakeTACDigiHit(const TACIndex_t &idx,
							     const Df250PulseData *pd) const
{
	   DTACDigiHit *h = new DTACDigiHit();
	   CopyDf250Info(h, pd);

	   vDTACDigiHit.push_back(h);

   return h;
}

//---------------------------------
// MakeTACTDCDigiHit
//---------------------------------
DTACTDCDigiHit*  DTranslationTable::MakeTACTDCDigiHit(
                                    const TACIndex_t &idx,
                                    const DCAEN1290TDCHit *hit) const
{
   DTACTDCDigiHit *h = new DTACTDCDigiHit();
   CopyDCAEN1290TDCInfo(h, hit);

   vDTACTDCDigiHit.push_back(h);

   return h;
}

//---------------------------------
// MakeDIRCTDCDigiHit
//---------------------------------
DDIRCTDCDigiHit*  DTranslationTable::MakeDIRCTDCDigiHit(
                                    const DIRCIndex_t &idx,
                                    const DDIRCTDCHit *hit) const
{
   DDIRCTDCDigiHit *h = new DDIRCTDCDigiHit();
   CopyDIRCTDCInfo(h, hit);

   h->channel = idx.pixel;

   vDDIRCTDCDigiHit.push_back(h);

   return h;
}

//---------------------------------
// GetDetectorIndex
//---------------------------------
const DTranslationTable::DChannelInfo 
     &DTranslationTable::GetDetectorIndex(const csc_t &in_daq_index) const
{
    map<DTranslationTable::csc_t, DTranslationTable::DChannelInfo>::const_iterator detector_index_itr = Get_TT().find(in_daq_index);
    if (detector_index_itr == Get_TT().end()) {
       stringstream ss_err;
       ss_err << "Could not find detector channel in Translaton Table: "
              << "rocid = " << in_daq_index.rocid
              << "slot = " << in_daq_index.slot
              << "channel = " << in_daq_index.channel;
       throw JException(ss_err.str());
    } 

    return detector_index_itr->second;
}

//---------------------------------
// GetDAQIndex
//---------------------------------
const DTranslationTable::csc_t 
     &DTranslationTable::GetDAQIndex(const DChannelInfo &in_channel) const
{
    map<DTranslationTable::csc_t, DTranslationTable::DChannelInfo>::const_iterator tt_itr = Get_TT().begin();

    // search through the whole Table to find the key that corresponds to our detector channel
    // this is not terribly efficient - linear in the size of the table
    bool found = false;
    for (; tt_itr != Get_TT().end(); tt_itr++) {
       const DTranslationTable::DChannelInfo &det_channel = tt_itr->second;
       if ( det_channel.det_sys == in_channel.det_sys ) {
          switch ( in_channel.det_sys ) {
          case DTranslationTable::BCAL:
             if ( det_channel.bcal == in_channel.bcal ) 
                found = true;
             break;
          case DTranslationTable::CDC:
             if ( det_channel.cdc == in_channel.cdc ) 
                found = true;
             break;
          case DTranslationTable::FCAL:
             if ( det_channel.fcal == in_channel.fcal ) 
                found = true;
             break;
          case DTranslationTable::ECAL:
             if ( det_channel.ecal == in_channel.ecal ) 
                found = true;
             break;
          case DTranslationTable::CCAL:
             if ( det_channel.ccal == in_channel.ccal ) 
                found = true;
             break;
          case DTranslationTable::CCAL_REF:
             if ( det_channel.ccal_ref == in_channel.ccal_ref ) 
                found = true;
             break;
          case DTranslationTable::FDC_CATHODES:
             if ( det_channel.fdc_cathodes == in_channel.fdc_cathodes ) 
                found = true;
             break;
          case DTranslationTable::FDC_WIRES:
             if ( det_channel.fdc_wires == in_channel.fdc_wires ) 
                found = true;
             break;
          case DTranslationTable::PS:
             if ( det_channel.ps == in_channel.ps ) 
                found = true;
            break;
          case DTranslationTable::PSC:
             if ( det_channel.psc == in_channel.psc )
                found = true;
             break;
          case DTranslationTable::RF:
             if ( det_channel.rf == in_channel.rf )
                found = true;
             break;
          case DTranslationTable::SC:
             if ( det_channel.sc == in_channel.sc )
                found = true;
             break;
          case DTranslationTable::TAGH:
             if ( det_channel.tagh == in_channel.tagh )
                found = true;
             break;
          case DTranslationTable::TAGM:
             if ( det_channel.tagm == in_channel.tagm )
                found = true;
             break;
          case DTranslationTable::TOF:
             if ( det_channel.tof == in_channel.tof )
                found = true;
             break;
          case DTranslationTable::CTOF:
             if ( det_channel.ctof == in_channel.ctof )
                found = true;
             break;
          case DTranslationTable::TPOLSECTOR:
             if ( det_channel.tpolsector == in_channel.tpolsector )
                found = true;
             break;
          case DTranslationTable::TAC:
             if ( det_channel.tac == in_channel.tac )
                found = true;
             break;
	  case DTranslationTable::DIRC:
             if ( det_channel.dirc == in_channel.dirc )
                found = true;
	     break;
	  case DTranslationTable::TRD:
             if ( det_channel.trd == in_channel.trd )
                found = true;
             break;
	  case DTranslationTable::FMWPC:
             if ( det_channel.fmwpc == in_channel.fmwpc )
                found = true;
             break;

          default:
             jerr << "DTranslationTable::GetDAQIndex(): "
                  << "Invalid detector type = " << in_channel.det_sys 
                  << std::endl;
       }
   }

   if (found)
       break;
    }
    
    if (tt_itr == Get_TT().end()) {
       stringstream ss_err;
       ss_err << "Could not find DAQ channel in Translaton Table:  "
              << Channel2Str(in_channel) << std::endl;
       throw JException(ss_err.str());
    }

    return tt_itr->first;
}

//----------------
// Channel2Str
//----------------
string DTranslationTable::Channel2Str(const DChannelInfo &in_channel) const
{
    stringstream ss;
    
    switch ( in_channel.det_sys ) {
    case DTranslationTable::BCAL:
       ss << "module = " << in_channel.bcal.module << " layer = " << in_channel.bcal.layer 
          << " sector = " << in_channel.bcal.sector << " end = " << in_channel.bcal.end;
       break;
    case DTranslationTable::CDC:
       ss << "ring = " << in_channel.cdc.ring << " straw = " << in_channel.cdc.straw;
       break;
    case DTranslationTable::FCAL:
       ss << "row = " << in_channel.fcal.row << " column = " << in_channel.fcal.col;
       break;
    case DTranslationTable::ECAL:
       ss << "row = " << in_channel.ecal.row << " column = " << in_channel.ecal.col;
       break;
    case DTranslationTable::CCAL:
       ss << "row = " << in_channel.ccal.row << " column = " << in_channel.ccal.col;
       break;
    case DTranslationTable::CCAL_REF:
       ss << "id = " << in_channel.ccal_ref.id << " id = " << in_channel.ccal_ref.id;
       break;
    case DTranslationTable::FDC_CATHODES:
       ss << "package = " << in_channel.fdc_cathodes.package
          << " chamber = " << in_channel.fdc_cathodes.chamber
          << " view = " << in_channel.fdc_cathodes.view
          << " strip = " << in_channel.fdc_cathodes.strip 
          << " strip type = " << in_channel.fdc_cathodes.strip_type;
       break;
    case DTranslationTable::FDC_WIRES:
       ss << "package = " << in_channel.fdc_wires.package
          << " chamber = " << in_channel.fdc_wires.chamber
          << " wire = " << in_channel.fdc_wires.wire;
       break;
    case DTranslationTable::PS:
       ss << "side = " << in_channel.ps.side << " id = " << in_channel.ps.id;
       break;
    case DTranslationTable::PSC:
       ss << "id = " << in_channel.psc.id;
       break;
    case DTranslationTable::RF:
       ss << "system = " << SystemName(in_channel.rf.dSystem);
       break;
    case DTranslationTable::SC:
       ss << "sector = " << in_channel.sc.sector;
       break;
    case DTranslationTable::TAGH:
       ss << "id = " << in_channel.tagh.id;
       break;
    case DTranslationTable::TAGM:
       ss << "row = " << in_channel.tagm.row << " column = " << in_channel.tagm.col;
       break;
    case DTranslationTable::TOF:
       ss << "plane = " << in_channel.tof.plane << " bar = " << in_channel.tof.bar
          << " end = " << in_channel.tof.end;
       break;
    case DTranslationTable::CTOF:
       ss << "plane = " << in_channel.ctof.plane << " bar = " << in_channel.ctof.bar
          << " end = " << in_channel.ctof.end;
       break;
    case DTranslationTable::TPOLSECTOR:
       ss << "sector = " << in_channel.tpolsector.sector;
       break;
    case DTranslationTable::TAC:
       ss << " ";
       break;
    case DTranslationTable::DIRC:
       ss << "pixel = " << in_channel.dirc.pixel;
       break;
    case DTranslationTable::TRD:
       ss << "plane = " << in_channel.trd.plane;
       ss << "strip = " << in_channel.trd.strip;
       break;
    case DTranslationTable::FMWPC:
       ss << "layer = " << in_channel.fmwpc.layer;
       ss << "wire = " << in_channel.fmwpc.wire;
       break;

    default:
       ss << "Unknown detector type" << std::endl;
    }   

    return ss.str();
}

//----------------
// Addf250ObjectsToCallStack
//----------------
void DTranslationTable::Addf250ObjectsToCallStack(const JEvent& event, string caller) const
{
	AddToCallStack(event, caller, "Df250Config");
	AddToCallStack(event, caller, "Df250PulseIntegral");
	AddToCallStack(event, caller, "Df250PulsePedestal");
	AddToCallStack(event, caller, "Df250PulseTime");
}

//----------------
// Addf125CDCObjectsToCallStack
//----------------
void DTranslationTable::Addf125CDCObjectsToCallStack(const JEvent& event, string caller, bool addpulseobjs) const
{
	AddToCallStack(event, caller, "Df125Config");
	if(addpulseobjs){
		// new style
		AddToCallStack(event, caller, "Df125CDCPulse");
	}else{
		// old style
		AddToCallStack(event, caller, "Df125PulseIntegral");
		AddToCallStack(event, caller, "Df125PulsePedestal");
		AddToCallStack(event, caller, "Df125PulseTime");
	}
}

//----------------
// Addf125FDCObjectsToCallStack
//----------------
void DTranslationTable::Addf125FDCObjectsToCallStack(const JEvent& event, string caller, bool addpulseobjs) const
{
	AddToCallStack(event, caller, "Df125Config");
	if(addpulseobjs){
		// new style
		AddToCallStack(event, caller, "Df125FDCPulse");
	}else{
		// old style
		AddToCallStack(event, caller, "Df125PulseIntegral");
		AddToCallStack(event, caller, "Df125PulsePedestal");
		AddToCallStack(event, caller, "Df125PulseTime");
	}
}

//----------------
// AddF1TDCObjectsToCallStack
//----------------
void DTranslationTable::AddF1TDCObjectsToCallStack(const JEvent& event, string caller) const
{
	AddToCallStack(event, caller, "DF1TDCConfig");
	AddToCallStack(event, caller, "DF1TDCHit");
}

//----------------
// AddCAEN1290TDCObjectsToCallStack
//----------------
void DTranslationTable::AddCAEN1290TDCObjectsToCallStack(const JEvent& event, string caller) const
{
	AddToCallStack(event, caller, "DCAEN1290TDCConfig");
	AddToCallStack(event, caller, "DCAEN1290TDCHit");
}

//----------------
// AddToCallStack
//----------------
void DTranslationTable::AddToCallStack(const JEvent& event,
                                       string caller, string callee) const
{
   /// This is used to give information to JANA regarding the relationship and
   /// origin of some of these data objects. This is really just needed so that
   /// the janadot program can be used to produce the correct callgraph. Because
   /// of how this plugin works, JANA can't record the correct call stack (at
   /// least not easily!) Therefore, we have to give it a little help here.

   JCallGraphRecorder::JCallGraphNode cs;
   //cs.start_time = cs.end_time = {0};
   cs.caller_name = caller;
   cs.callee_name = callee;
   cs.data_source = JCallGraphRecorder::DATA_FROM_CACHE;
   event.GetJCallGraphRecorder()->AddToCallGraph(cs);
   cs.callee_name = cs.caller_name;
   cs.caller_name = "<ignore>";
   cs.data_source = JCallGraphRecorder::DATA_FROM_FACTORY;
   event.GetJCallGraphRecorder()->AddToCallGraph(cs);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//  The following routines access the translation table
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static DTranslationTable::Detector_t DetectorStr2DetID(string &type);
static void StartElement(void *userData, const char *xmlname, const char **atts);
static void EndElement(void *userData, const char *xmlname);


//---------------------------------
// ReadTranslationTable
//---------------------------------
void DTranslationTable::ReadTranslationTable(JCalibration *jcalib)
{
   // It seems expat is not thread safe so we lock a mutex here and
   // read in the translation table just once
   pthread_mutex_lock(&Get_TT_Mutex());
   if (Get_TT_Initialized()) {
      pthread_mutex_unlock(&Get_TT_Mutex());
      return;
   }

   // String to hold entire XML translation table
   string tt_xml; 

   // Try getting it from CCDB first
   if (jcalib && !NO_CCDB) {
      map<string,string> tt;
      string namepath = "Translation/DAQ2detector";
      jout << "Reading translation table from calib DB: " << namepath << " ..." << std::endl;
      jcalib->GetCalib(namepath, tt);
      if (tt.size() != 1) {
         jerr << " Error: Unexpected translation table format!" << std::endl;
         jerr << "        tt.size()=" << tt.size() << " (expected 1)" << std::endl;
      }else{
         // Copy table into tt string
         map<string,string>::iterator iter = tt.begin();
         tt_xml = iter->second;
      }
   }
   
   // If getting from CCDB fails, try just reading in local file
   if (tt_xml.size() == 0) {
      if (!NO_CCDB) jout << "Unable to get translation table from CCDB." << std::endl;
      jout << "Will try reading TT from local file: " << XML_FILENAME << std::endl;

      // Open file
      ifstream ifs(XML_FILENAME.c_str());
      if (! ifs.is_open()) {
         jerr << " Error: Cannot open file! Translation table unavailable." << std::endl;
         pthread_mutex_unlock(&Get_TT_Mutex());
         return;
      }
      
      // read lines into stringstream object 
      stringstream ss;
      while (ifs.good()) {
         char line[4096];
         ifs.getline(line, 4096);
         ss << line;
      }

      // Close file
      ifs.close();
      
      // Copy from stringstream to tt
      tt_xml = ss.str();
   }
   
	// If a ROCID by system map exists it probably means the user
	// specified particular systems to parse and the default map
	// has to be installed above in SetSystemsToParse. Make a copy
	// and clear the map so that it can be filled while parsing.
	// After, we can compare the two and warn the user if there's
	// a descrepancy.
	auto save_rocid_map = Get_ROCID_By_System();
	Get_ROCID_By_System().clear();

   // create parser and specify element handlers
   XML_Parser xmlParser = XML_ParserCreate(NULL);
   if (xmlParser == NULL) {
      jerr << "readTranslationTable...unable to create parser" << std::endl;
      exit(EXIT_FAILURE);
   }
   XML_SetElementHandler(xmlParser,StartElement,EndElement);
   XML_SetUserData(xmlParser, &Get_TT());

   // Parse XML string
   int status=XML_Parse(xmlParser, tt_xml.c_str(), tt_xml.size(), 1); // "1" indicates this is the final piece of XML
   if (status == 0) {
      jerr << "  ?readTranslationTable...parseXMLFile parse error for " << XML_FILENAME << std::endl;
      jerr << XML_ErrorString(XML_GetErrorCode(xmlParser)) << std::endl;
   }
   
	// Check if there was a rocid map prior to parsing and
	// if so, compare the 2.
	if( !save_rocid_map.empty() ){
		if( save_rocid_map != Get_ROCID_By_System() ){
			jerr << " The rocid by system map read from the translation table in" << endl;
			jerr << " the CCDB differs from the default. This may happen if you" << endl;
			jerr << " specified EVIO:SYSTEMS_TO_PARSE and the hardcoded table is" << endl;
			jerr << " out of date. This may be handled in different ways depending" << endl;
			jerr << " on how EVIO:SYSTEMS_TO_PARSE_FORCE is set:" << endl;
			jerr << "    0 = Treat mismatch as error " << endl;
			jerr << "    1 = Use CCDB map in case of mismatch" << endl;
			jerr << "    2 = Use hardcoded map in case of mismatch" << endl;
			jerr << " n.b because the hardcoded map may actually need to be used before" << endl;
			jerr << " the CCDB map is read in choosing option 1 may not be absolutely" << endl;
			jerr << " true for all events." << endl;
			jerr << " The value of EVIO:SYSTEMS_TO_PARSE_FORCE is currently: " << Get_ROCID_By_System_Mismatch_Behaviour() << endl;
			jerr << " Here are the (mismatched) maps:" << endl;
			for(auto it : Get_ROCID_By_System()){
				cerr << " " << DetectorName((Detector_t)it.first) << ": CCDB num. rocids=" << Get_ROCID_By_System()[it.first].size() << "  num. hardcoded rocids=" << save_rocid_map[it.first].size() << endl;
				cerr << "       CCDB={";
				for(auto a : Get_ROCID_By_System()[it.first]) cerr << a <<", ";
				cerr << "}  hardcoded={";
				for(auto a : save_rocid_map[it.first]) cerr << a <<", ";
				cerr << "}" << endl;
			}
			for(auto it : save_rocid_map){
				if(Get_ROCID_By_System().find(it.first) != Get_ROCID_By_System().end()) continue;
				cerr << " " << DetectorName((Detector_t)it.first) << ": CCDB num. rocids=" << Get_ROCID_By_System()[it.first].size() << "  hardcoded num. rocids=" << save_rocid_map[it.first].size() << endl;
				cerr << "       CCDB={";
				for(auto a : Get_ROCID_By_System()[it.first]) cerr << a <<", ";
				cerr << "}  hardcoded={";
				for(auto a : save_rocid_map[it.first]) cerr << a <<", ";
				cerr << "}" << endl;
			}
			switch( Get_ROCID_By_System_Mismatch_Behaviour() ){
				case 0:
					exit(-1);  // treat as error
				case 1:
					// Use CCDB map (already in Get_ROCID_By_System())
					break;
				case 2:
					// Use hard coded map
					Get_ROCID_By_System() = save_rocid_map;
					break;
				default:
					jerr << "Bad value for Get_ROCID_By_System_Mismatch_Behaviour() (aka EVIO:SYSTEMS_TO_PARSE_FORCE)" << endl;
					exit(-1);
			}
		}
	}

   jout << Get_TT().size() << " channels defined in translation table" << std::endl;
   XML_ParserFree(xmlParser);

   pthread_mutex_unlock(&Get_TT_Mutex());
   Get_TT_Initialized() = true;
}

//---------------------------------
// DetectorStr2DetID
//---------------------------------
DTranslationTable::Detector_t DetectorStr2DetID(string &type)
{
   if ( type == "fdc_cathodes" ) {
      return DTranslationTable::FDC_CATHODES;
   } else if ( type == "fdc_wires" ) {
      return DTranslationTable::FDC_WIRES;   
   } else if ( type == "bcal" ) {
      return DTranslationTable::BCAL;
   } else if ( type == "cdc" ) {
      return DTranslationTable::CDC;   
   } else if ( type == "fcal" ) {
      return DTranslationTable::FCAL;
   } else if ( type == "ecal" ) {
     return DTranslationTable::ECAL;
   } else if ( type == "ccal" ) {
      return DTranslationTable::CCAL;
   } else if ( type == "ccal_ref" ) {
     return DTranslationTable::CCAL_REF;
   } else if ( type == "ps" ) {
      return DTranslationTable::PS;
   } else if ( type == "psc" ) {
      return DTranslationTable::PSC;
   } else if ( type == "rf" ) {
      return DTranslationTable::RF;
   } else if ( type == "st" ) {
           // The start counter is labelled by "ST" in the translation table
           // but we stick with the "SC" label in this plugin for consistency
           // with the rest of the reconstruction software
      return DTranslationTable::SC;
   } else if ( type == "tagh" ) {
      return DTranslationTable::TAGH;
   } else if ( type == "tagm" ) {
      return DTranslationTable::TAGM;
   } else if ( type == "tof" ) {
      return DTranslationTable::TOF;
   } else if ( type == "ctof" ) {
      return DTranslationTable::CTOF;
   } else if ( type == "tpol" ) {
      return DTranslationTable::TPOLSECTOR;
   } else if ( type == "tac" ) {
	      return DTranslationTable::TAC;
   } else if ( type == "dirc" ) {
	   return DTranslationTable::DIRC;
   } else if ( type == "trd" ) {
	   return DTranslationTable::TRD;
   } else if ( type == "fmwpc" ) {
	   return DTranslationTable::FMWPC;
   } else
   {
      return DTranslationTable::UNKNOWN_DETECTOR;
   }
}

//---------------------------------
// StartElement
//---------------------------------
void StartElement(void *userData, const char *xmlname, const char **atts)
{
   static int crate=0, slot=0;
   
   static string type,Type;
   int mc2codaType= 0;
   int channel = 0;
   string Detector, locSystem;
   int end=0;
   int row=0,column=0,module=0,sector=0,layer=0;
   int ring=0,straw=0,plane=0,bar=0;
   int package=0,chamber=0,view=0,strip=0,wire=0;
   int id=0, strip_type=0;
   int side=0;
   int pixel=0;

   // This complicated line just recasts the userData pointer into
   // a reference to the "TT" member of the DTranslationTable object
   // that called us.
   map<DTranslationTable::csc_t, DTranslationTable::DChannelInfo> &TT = *((map<DTranslationTable::csc_t, DTranslationTable::DChannelInfo>*)userData);
   
   // store crate summary info, fill both maps
   if (strcasecmp(xmlname,"halld_online_translation_table") == 0) {
      // do nothing
      
   } else if (strcasecmp(xmlname,"crate") == 0) {
      for (int i=0; atts[i]; i+=2) {
         if (strcasecmp(atts[i],"number") == 0) {
            crate = atoi(atts[i+1]);
            break;
         }
      }
      
   } else if (strcasecmp(xmlname,"slot") == 0) {
      for (int i=0; atts[i]; i+=2) {
         if (strcasecmp(atts[i],"number") == 0) {
            slot = atoi(atts[i+1]);
         } else if (strcasecmp(atts[i],"type") == 0) {
            Type = string(atts[i+1]);
            type = string(atts[i+1]);
            std::transform(type.begin(), type.end(), type.begin(), (int(*)(int)) tolower);
         }
      }
      
      // The detID value set here shows up in the header of the Data Block Bank
      // of the output file. It should be set to one if this crate has JLab
      // made modules that output in the standard format (see document:
      // "VME Data Format Standards for JLAB Modules"). These would include
      // f250ADC, f125ADC, F1TDC, .... Slots containing other types of modules
      // (e.g. CAEN1290) should have their own unique detID. We use detID of
      // zero for non-digitizing modules like CPUs nd TIDs even though potentially,
      // one could read data from these.
//      mc2codaType = ModuleStr2ModID(type);      
      
   } else if (strcasecmp(xmlname,"channel") == 0) {
      
      for (int i=0; atts[i]; i+=2) {
         string tag(atts[i+0]);
         string sval(atts[i+1]);
         int ival = atoi(atts[i+1]);

         if (tag == "number")
            channel = ival;
         else if (tag == "detector")
            Detector = sval;
         else if (tag == "row")
            row = ival;
         else if (tag == "column")
            column = ival;
         else if (tag == "col")
            column = ival;
         else if (tag == "module")
            module = ival;
         else if (tag == "sector")
            sector = ival;
         else if (tag == "layer")
            layer = ival;
         else if (tag == "chan");
//            chan = ival;
         else if (tag == "ring")
            ring = ival;
         else if (tag == "straw")
            straw = ival;
         else if (tag == "gPlane");
//            gPlane = ival;
         else if (tag == "element");
//            element = ival;
         else if (tag == "plane")
            plane = ival;
         else if (tag == "bar")
            bar = ival;
         else if (tag == "package")
            package = ival;
         else if (tag == "chamber")
            chamber = ival;
         else if (tag == "view") {
            if (sval == "U")
               view=1;
            else if (sval == "D")
               view=3;
         }
         else if (tag == "strip")
            strip = ival;
         else if (tag == "wire")
            wire = ival;
         else if (tag == "side") { 
            if (sval == "A") {
               side = DPSGeometry::kNorth;
            }
            else if (sval == "B") {
               side = DPSGeometry::kSouth;
            }
	 }
	 else if (tag == "pixel") {
		 pixel = ival;
	 }
         else if (tag == "id")
            id = ival;
         else if (tag == "end") {
            if (sval == "U") {
               end = DBCALGeometry::kUpstream;
               view=1;
            }
            else if (sval == "D") {
               end = DBCALGeometry::kDownstream;
               view=3;
            }
            else if (sval == "N")
               end = 0; // TOF or CTOF north
            else if (sval == "S")
               end = 1; // TOF or CTOF south
            else if (sval == "UP")
               end = 0; // TOF or CTOF up
            else if (sval == "DW") 
               end = 1; // TOF or CTOF down
         }
         else if (tag == "strip_type") {
            if (sval == "full")
               strip_type = 1;
            else if (sval == "A")
               strip_type = 2;
            else if (sval == "B")
               strip_type = 3;
         }
         else if (tag == "system")
        	 locSystem = sval;
      }
      
      // ignore certain module types
      if (type == "disc")
         return;
      if (type == "ctp")
         return;
      if (type == "sd")
         return;
      if (type == "a1535sn")
         return;

      
//      // Data integrity check
//      if (crate < 0 || crate >= MAXDCRATE) {
//         jerr << " Crate value of " << crate 
//              << " is not in range 0 <= crate < " << MAXDCRATE << std::endl;
//         exit(-1);
//      }
//      
//      if (slot < 0 || slot >= MAXDSLOT) {
//         jerr << " Slot value of " << slot 
//              << " is not in range 0 <= slot < " << MAXDSLOT << std::endl;
//         exit(-1);
//      }
//      
//      if (channel < 0 || channel >= MAXDCHANNEL) {
//         jerr << " Crate value of " << channel 
//              << " is not in range 0 <= channel < " << MAXDCHANNEL << std::endl;
//         exit(-1);
//      }
      
      // fill maps
      
      DTranslationTable::csc_t csc = {(uint32_t)crate,(uint32_t)slot,(uint32_t)channel};
      string detector = Detector;
      std::transform(detector.begin(), detector.end(), detector.begin(), (int(*)(int)) tolower);
      
      //string s="unknown::";

      // Common indexes
      DTranslationTable::DChannelInfo &ci = TT[csc];
      ci.CSC = csc;
      ci.module_type = (DModuleType::type_id_t)mc2codaType;
      ci.det_sys = DetectorStr2DetID(detector);
      DTranslationTable::Get_ROCID_By_System()[ci.det_sys].insert(crate);

      // detector-specific indexes
      switch (ci.det_sys) {
         case DTranslationTable::BCAL:
            ci.bcal.module = module;
            ci.bcal.layer = layer;
            ci.bcal.sector = sector;
            ci.bcal.end = end;
            break;
         case DTranslationTable::CDC:
            ci.cdc.ring = ring;
            ci.cdc.straw = straw;
            break;
         case DTranslationTable::FCAL:
            ci.fcal.row = row;
            ci.fcal.col = column;
            break;
         case DTranslationTable::ECAL:
            ci.ecal.row = row;
            ci.ecal.col = column;
            break;
         case DTranslationTable::CCAL:
            ci.ccal.row = row;
            ci.ccal.col = column;
            break;
         case DTranslationTable::CCAL_REF:
            ci.ccal_ref.id = id;
            break;
         case DTranslationTable::FDC_CATHODES:
            ci.fdc_cathodes.package = package;
            ci.fdc_cathodes.chamber = chamber;
            ci.fdc_cathodes.view = view;
            ci.fdc_cathodes.strip = strip;
            ci.fdc_cathodes.strip_type = strip_type;
            break;
         case DTranslationTable::FDC_WIRES:
            ci.fdc_wires.package = package;
            ci.fdc_wires.chamber = chamber;
            ci.fdc_wires.wire = wire;
            break;
         case DTranslationTable::RF:
            ci.rf.dSystem = NameToSystem(locSystem.c_str());
            break;
         case DTranslationTable::SC:
            ci.sc.sector = sector;
            break;
         case DTranslationTable::TAGH:
            ci.tagh.id = id;
            break;
         case DTranslationTable::TAGM:
            ci.tagm.col = column;
            ci.tagm.row = row;
            break;
         case DTranslationTable::TOF:
            ci.tof.plane = plane;
            ci.tof.bar = bar;
            ci.tof.end = end;
            break;
         case DTranslationTable::CTOF:
            ci.ctof.plane = layer;
            ci.ctof.bar = bar;
            ci.ctof.end = end;
            break;
         case DTranslationTable::PS:
	        ci.ps.side = side;
	        ci.ps.id = id;
	        break;
         case DTranslationTable::PSC:
	        ci.psc.id = id;
            break;
         case DTranslationTable::TPOLSECTOR:
            ci.tpolsector.sector = sector;
            break;
         case DTranslationTable::TAC:
//        	 ci.tac;
            break;
         case DTranslationTable::DIRC:
	      ci.dirc.pixel = pixel;
	      break;
         case DTranslationTable::TRD:
	      ci.trd.plane = plane;
	      ci.trd.strip = strip;
	      break;
         case DTranslationTable::FMWPC:
	      ci.fmwpc.layer = layer;
	      ci.fmwpc.wire = wire;
	      break;
        case DTranslationTable::UNKNOWN_DETECTOR:
		 default:
            break;
      }

   } else {
      jerr << std::endl << std::endl
           << "?startElement...unknown xml tag " << xmlname
           << std::endl << std::endl;
   }
   
}


//--------------------------------------------------------------------------


void EndElement(void *userData, const char *xmlname) {
   // nothing to do yet...
}


//--------------------------------------------------------------------------
