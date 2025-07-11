// $Id$
//
//    File: JEventSource_EVIOpp.cc
// Created: Tue Mar 29 08:14:42 EDT 2016
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <forward_list>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace std::chrono;


#include <DAQ/Df250EmulatorAlgorithm_v1.h>
#include <DAQ/Df250EmulatorAlgorithm_v2.h>
#include <DAQ/Df250EmulatorAlgorithm_v3.h>
#include <DAQ/Df125EmulatorAlgorithm_v2.h>


#include "JEventSource_EVIOpp.h"
#include "LinkAssociations.h"



// sort Df250PulseData objects by pulse number 
bool sortf250pulsenumbers(const Df250PulseData *a, const Df250PulseData *b) {
	return a->pulse_number < b->pulse_number;
}

//----------------
// Constructor
//----------------
JEventSource_EVIOpp::JEventSource_EVIOpp()
{
    SetTypeName(NAME_OF_THIS);
	SetCallbackStyle(CallbackStyle::ExpertMode); // 
	EnableGetObjects(true);  // Check the source first for existing objects; only invoke the factory to create them if they aren't found in the source.
	EnableFinishEvent(true); // Ensure ::FinishEvent gets called. By default, it is disabled (false).
}

//----------------
// Init
//----------------
void JEventSource_EVIOpp::Init() {
	// This is where we register parameters and services that our JEventSource needs
	auto app = GetApplication();

	// Initialize dedicated logger
	evioout.SetGroup("EVIO");
	evioout.ShowTimestamp(true);
	evioout.ShowThreadstamp(true);

	// Get configuration parameters
	VERBOSE = 0;
	VERBOSE_ET = 0;
	NTHREADS = 2;
	MAX_PARSED_EVENTS = 128;
	MAX_EVENT_RECYCLES = 1000;
	MAX_OBJECT_RECYCLES = 1000;
	LOOP_FOREVER = false;
	USER_RUN_NUMBER = 0;
	ET_STATION_NEVENTS = 10;
	ET_STATION_CREATE_BLOCKING = false;
	PRINT_STATS = true;
	SWAP = true;
	LINK = true;
	LINK_TRIGGERTIME = true;
	LINK_BORCONFIG = true;
	LINK_CONFIG = true;
	PARSE = true;
	PARSE_F250 = true;
	SKIP_F250_FORMAT_ERROR = false;
	PARSE_F125 = true;
	PARSE_F1TDC = true;
	PARSE_CAEN1290TDC = true;
	PARSE_CONFIG = true;
	PARSE_BOR = true;
	PARSE_EPICS = true;
	PARSE_EVENTTAG = true;
	PARSE_TRIGGER = true;
	PARSE_SSP = true;
	SKIP_SSP_FORMAT_ERROR = false;
	PARSE_GEMSRS = false;
	PARSE_HELICITY = true;
	NSAMPLES_GEMSRS = 9;
	APPLY_TRANSLATION_TABLE = true;
	IGNORE_EMPTY_BOR = false;
	F250_EMULATION_MODE = kEmulationAuto;
	F125_EMULATION_MODE = kEmulationNone;
	F250_EMULATION_VERSION = 3;
	RECORD_CALL_STACK = false;
	TREAT_TRUNCATED_AS_ERROR = false;
	SYSTEMS_TO_PARSE = "";
	SYSTEMS_TO_PARSE_FORCE = 0;
	BLOCKS_TO_SKIP = 0;
	PHYSICS_BLOCKS_TO_SKIP = 0;
	PHYSICS_BLOCKS_SKIPPED = 0;
	PHYSICS_BLOCKS_TO_KEEP = 0;
	PHYSICS_BLOCKS_KEPT = 0;

	app->SetDefaultParameter("EVIO:VERBOSE", VERBOSE, "Set verbosity level for processing and debugging statements while parsing. 0=no debugging messages. 10=all messages");
	app->SetDefaultParameter("ET:VERBOSE", VERBOSE_ET, "Set verbosity level for processing and debugging statements while reading from ET. 0=no debugging messages. 10=all messages");
	app->SetDefaultParameter("EVIO:NTHREADS", NTHREADS, "Set the number of worker threads to use for parsing the EVIO data");
	app->SetDefaultParameter("EVIO:MAX_PARSED_EVENTS", MAX_PARSED_EVENTS, "Set maximum number of events to allow in EVIO parsed events queue");
	app->SetDefaultParameter("EVIO:MAX_EVENT_RECYCLES", MAX_EVENT_RECYCLES, "Set maximum number of EVIO (i.e. block of) events  a worker thread should process before pruning excess DParsedEvent objects from its pool");
	app->SetDefaultParameter("EVIO:MAX_OBJECT_RECYCLES", MAX_OBJECT_RECYCLES, "Set maximum number of events a DParsedEvent should be used for before pruning excess hit objects from its pools");
	app->SetDefaultParameter("EVIO:LOOP_FOREVER", LOOP_FOREVER, "If reading from EVIO file, keep re-opening file and re-reading events forever (only useful for debugging) If reading from ET, this is ignored.");
	app->SetDefaultParameter("EVIO:RUN_NUMBER", USER_RUN_NUMBER, "User-supplied run number. Override run number from other sources with this.(will be ignored if set to zero)");
	app->SetDefaultParameter("EVIO:ET_STATION_NEVENTS", ET_STATION_NEVENTS, "Number of events to use if we have to create the ET station. Ignored if station already exists.");
	app->SetDefaultParameter("EVIO:ET_STATION_CREATE_BLOCKING", ET_STATION_CREATE_BLOCKING, "Set this to 0 to create station in non-blocking mode (default is to create it in blocking mode). Ignored if station already exists.");
	app->SetDefaultParameter("EVIO:PRINT_STATS", PRINT_STATS, "Print some additional stats from event source when it's finished processing events");

	app->SetDefaultParameter("EVIO:SWAP", SWAP, "Allow swapping automatic swapping. Turning this off should only be used for debugging.");
	app->SetDefaultParameter("EVIO:LINK", LINK, "Link associated objects. Turning this off should only be used for debugging.");
	app->SetDefaultParameter("EVIO:LINK_TRIGGERTIME", LINK_TRIGGERTIME, "Link D*TriggerTime associated objects. This is on by default and may be OK to turn off (but please check output if you do!)");
	app->SetDefaultParameter("EVIO:LINK_BORCONFIG", LINK_BORCONFIG, "Link BORConfig associated objects. This is on by default. If turned off, it will break emulation (and possibly other things in the future).");
	app->SetDefaultParameter("EVIO:LINK_CONFIG", LINK_CONFIG, "Link Config associated objects. This is on by default.");

	app->SetDefaultParameter("EVIO:PARSE", PARSE, "Set this to 0 to disable parsing of event buffers and generation of any objects (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_F250", PARSE_F250, "Set this to 0 to disable parsing of data from F250 ADC modules (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:SKIP_F250_FORMAT_ERROR", SKIP_F250_FORMAT_ERROR, "Set this to 1 to skip F250 format errors (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_F125", PARSE_F125, "Set this to 0 to disable parsing of data from F125 ADC modules (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_F1TDC", PARSE_F1TDC, "Set this to 0 to disable parsing of data from F1TDC modules (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_CAEN1290TDC", PARSE_CAEN1290TDC, "Set this to 0 to disable parsing of data from CAEN 1290 TDC modules (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_CONFIG", PARSE_CONFIG, "Set this to 0 to disable parsing of ROC configuration data in the data stream (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_BOR", PARSE_BOR, "Set this to 0 to disable parsing of BOR events from the data stream (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_EPICS", PARSE_EPICS, "Set this to 0 to disable parsing of EPICS events from the data stream (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_EVENTTAG", PARSE_EVENTTAG, "Set this to 0 to disable parsing of event tag data in the data stream (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_TRIGGER", PARSE_TRIGGER, "Set this to 0 to disable parsing of the built trigger bank from CODA (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_SSP", PARSE_SSP, "Set this to 0 to disable parsing of the SSP (DIRC data) bank from CODA (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:SKIP_SSP_FORMAT_ERROR", SKIP_SSP_FORMAT_ERROR, "Set this to 1 to skip SSP format errors (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_GEMSRS", PARSE_GEMSRS, "Set this to 0 to disable parsing of the SRS (GEM data) bank from CODA (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:PARSE_HELICITY", PARSE_HELICITY, "Set this to 0 to disable parsing of the helicity decoder bank from CODA (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:NSAMPLES_GEMSRS", NSAMPLES_GEMSRS, "Set this to number of readout samples for SRS (GEM data) bank from CODA (for benchmarking/debugging)");
	app->SetDefaultParameter("EVIO:APPLY_TRANSLATION_TABLE", APPLY_TRANSLATION_TABLE, "Apply the translation table to create DigiHits (you almost always want this on)");
	app->SetDefaultParameter("EVIO:IGNORE_EMPTY_BOR", IGNORE_EMPTY_BOR, "Set to non-zero to continue processing data even if an empty BOR event is encountered.");
	app->SetDefaultParameter("EVIO:TREAT_TRUNCATED_AS_ERROR", TREAT_TRUNCATED_AS_ERROR, "Set to non-zero to have a truncated EVIO file the JANA return code to non-zero indicating the program errored.");

	app->SetDefaultParameter("EVIO:F250_EMULATION_MODE", F250_EMULATION_MODE, "Set f250 emulation mode. 0=no emulation, 1=always, 2=auto. Default is 2 (auto).");
	app->SetDefaultParameter("EVIO:F125_EMULATION_MODE", F125_EMULATION_MODE, "Set f125 emulation mode. 0=no emulation, 1=always, 2=auto. Default is 2 (auto).");

	app->SetDefaultParameter("EVIO:SYSTEMS_TO_PARSE", SYSTEMS_TO_PARSE,
			"Comma separated list of systems to parse EVIO data for. "
			"Default is empty string which means to parse all. System "
			"names should be what is returned by DTranslationTable::DetectorName() .");
	app->SetDefaultParameter("EVIO:SYSTEMS_TO_PARSE_FORCE", SYSTEMS_TO_PARSE_FORCE,
			"How to handle mismatches between hard coded map and one read from CCDB "
			"when EVIO:SYSTEMS_TO_PARSE is set. 0=Treat as error, 1=Use CCDB, 2=Use hardcoded");

	app->SetDefaultParameter("EVIO:BLOCKS_TO_SKIP", BLOCKS_TO_SKIP, "Number of EVIO blocks to skip parsing at start of file (typically 1 block=40 events)");
	app->SetDefaultParameter("EVIO:PHYSICS_BLOCKS_TO_SKIP", PHYSICS_BLOCKS_TO_SKIP, "Number of Physics EVIO blocks to skip parsing at start of file (typically 1 block=40 events). n.b. BOR, EPICS, and Control events will still be processed.");
	app->SetDefaultParameter("EVIO:PHYSICS_BLOCKS_TO_KEEP", PHYSICS_BLOCKS_TO_KEEP, "Number of Physics EVIO blocks to process, after any skipped if EVIO:PHYSICS_BLOCKS_TO_SKIP is > 0. (typically 1 block=40 events). n.b. BOR, EPICS, and Control events will still be processed.");

	if(app->GetJParameterManager()->Exists("RECORD_CALL_STACK")) japp->GetParameter("RECORD_CALL_STACK", RECORD_CALL_STACK);
}

//----------------
// Open
//----------------
void JEventSource_EVIOpp::Open() {
	DONE = false;
	DISPATCHER_END = false;
	NEVENTS_PROCESSED = 0;
	NDISPATCHER_STALLED  = 0;
	NEVENTBUFF_STALLED   = 0;
	NPARSER_STALLED      = 0;

	auto source_name = GetResourceName();
	
	// Define base set of status bits
	DStatusBits::SetStatusBitDescriptions();


	// Set rocids of all systems to parse (if specified)
	ROCIDS_TO_PARSE = DTranslationTable::GetSystemsToParse(SYSTEMS_TO_PARSE, SYSTEMS_TO_PARSE_FORCE);

	jobtype = DEVIOWorkerThread::JOB_NONE;
	if( PARSE ) jobtype |= DEVIOWorkerThread::JOB_FULL_PARSE;
	if( LINK  ) jobtype |= DEVIOWorkerThread::JOB_ASSOCIATE;
	if(!LINK  ) LINK_BORCONFIG = false;
	
	source_type          = kNoSource;
	hdevio               = NULL;
	hdet                 = NULL;
	et_quit_next_timeout = false;

	uint64_t run_number_seed = 0;

	// Open either ET system or file
	if(GetResourceName().find("ET:") == 0){

		// Try to open ET system
		if(VERBOSE>0) evioout << "Attempting to open \""<<source_name<<"\" as ET (network) source..." <<endl;

		hdet = new HDET(source_name, ET_STATION_NEVENTS, ET_STATION_CREATE_BLOCKING);
		if( ! hdet->is_connected ){
			cerr << hdet->err_mess.str() << endl;
			throw JException("Failed to open ET system: %s", source_name.c_str());
		}
		hdet->VERBOSE = VERBOSE_ET;
		source_type = kETSource;

	}else{

		// Try to open the file.
		if(VERBOSE>0) evioout << "Attempting to open \""<<source_name<<"\" as EVIO file..." <<endl;

		hdevio = new HDEVIO(source_name, true, VERBOSE);
		if( ! hdevio->is_open ){
			cerr << hdevio->err_mess.str() << endl;
			throw JException("Failed to open EVIO file: %s", source_name.c_str()); // throw exception indicating error
		}
		source_type = kFileSource;
		hdevio->IGNORE_EMPTY_BOR = IGNORE_EMPTY_BOR;
		
		run_number_seed = SearchFileForRunNumber(); // try and dig out run number from file
	}

	if(VERBOSE>0) evioout << "Success opening event source \"" << source_name << "\"!" <<endl;
	
	// Create dispatcher thread
	dispatcher_thread = new thread(&JEventSource_EVIOpp::Dispatcher, this);

	// Create worker threads
	for(uint32_t i=0; i<NTHREADS; i++){
		DEVIOWorkerThread *w = new DEVIOWorkerThread(this, parsed_events, MAX_PARSED_EVENTS, PARSED_EVENTS_MUTEX, PARSED_EVENTS_CV, ROCIDS_TO_PARSE);
		w->VERBOSE             = VERBOSE;
		w->MAX_EVENT_RECYCLES  = MAX_EVENT_RECYCLES;
		w->MAX_OBJECT_RECYCLES = MAX_OBJECT_RECYCLES;
		w->PARSE_F250          = PARSE_F250;
		w->SKIP_F250_FORMAT_ERROR = SKIP_F250_FORMAT_ERROR;
		w->PARSE_F125          = PARSE_F125;
		w->PARSE_F1TDC         = PARSE_F1TDC;
		w->PARSE_CAEN1290TDC   = PARSE_CAEN1290TDC;
		w->PARSE_CONFIG        = PARSE_CONFIG;
		w->PARSE_BOR           = PARSE_BOR;
		w->PARSE_EPICS         = PARSE_EPICS;
		w->PARSE_EVENTTAG      = PARSE_EVENTTAG;
		w->PARSE_TRIGGER       = PARSE_TRIGGER;
		w->PARSE_SSP           = PARSE_SSP;
		w->SKIP_SSP_FORMAT_ERROR = SKIP_SSP_FORMAT_ERROR;
		w->PARSE_GEMSRS        = PARSE_GEMSRS;
		w->PARSE_HELICITY      = PARSE_HELICITY;
                w->NSAMPLES_GEMSRS     = NSAMPLES_GEMSRS;
		w->LINK_TRIGGERTIME    = LINK_TRIGGERTIME;
		w->LINK_CONFIG         = LINK_CONFIG;
		w->run_number_seed     = run_number_seed;
		worker_threads.push_back(w);
	}
	
	// Create emulator objects

	if(F250_EMULATION_VERSION == 1) {
		f250Emulator = new Df250EmulatorAlgorithm_v1(japp);
	} else if(F250_EMULATION_VERSION == 2) {
		f250Emulator = new Df250EmulatorAlgorithm_v2(NULL);
	} else {
		cerr << "loading VERSION 3" << endl;
		if(F250_EMULATION_VERSION != 3) 
				jerr << "Invalid fADC250 firmware version specified for emulation == " << F250_EMULATION_VERSION
				     << " ,  Using v3 firmware as default ..." << endl;
		f250Emulator = new Df250EmulatorAlgorithm_v3(japp);
	}

	f125Emulator = new Df125EmulatorAlgorithm_v2();

	// Record start time
	tstart = high_resolution_clock::now();
	
}

//----------------
// Destructor
//----------------
JEventSource_EVIOpp::~JEventSource_EVIOpp()
{
	// Set DONE flag to tell dispatcher thread to quit
	// as well as anyone in a wait state
	DONE = true;
	PARSED_EVENTS_CV.notify_all();
	
	// Wait for dispatcher to complete
	if(dispatcher_thread){
		DISPATCHER_END = true;
		dispatcher_thread->join();
		delete dispatcher_thread;
	}

	// Wait for all worker threads to end and destroy them all
	for(auto w : worker_threads){
		w->Finish();
		delete w;
	}
	
	// Delete emulator objects
	if(f250Emulator) delete f250Emulator;
	if(f125Emulator) delete f125Emulator;
	
    // We don't want to print stats if the source wasn't even opened
    bool source_was_opened = (hdevio != nullptr) || (hdet != nullptr);

	if (PRINT_STATS && source_was_opened) {
		auto tdiff = duration_cast<duration<double>>(tend - tstart);
		double rate = (double)NEVENTS_PROCESSED/tdiff.count();
		
		uint32_t NTHREADS_PROC = 1;
		if(japp->GetJParameterManager()->Exists("NTHREADS")) japp->GetParameter("NTHREADS", NTHREADS_PROC);

		// Calculate fraction of total time each stage spent
		// idle. There was a 1ms sleep for each one of these
		double tfrac_dis   = (double)NDISPATCHER_STALLED/1000.0/tdiff.count()*100.0;
		double tfrac_parse = (double)NPARSER_STALLED/1000.0/(double)NTHREADS/tdiff.count()*100.0;
		double tfrac_proc  = (double)NEVENTBUFF_STALLED/1000.0/tdiff.count()*100.0;
		
		char sdispatcher[256];
		char sparser[256];
		char sprocessor[256];
		sprintf(sdispatcher, "  NDISPATCHER_STALLED = %10ld (%4.1f%%)", (unsigned long)NDISPATCHER_STALLED, tfrac_dis  );
		sprintf(sparser    , "      NPARSER_STALLED = %10ld (%4.1f%%)", (unsigned long)NPARSER_STALLED    , tfrac_parse);
		sprintf(sprocessor , "   NEVENTBUFF_STALLED = %10ld (%4.1f%%)", (unsigned long)NEVENTBUFF_STALLED , tfrac_proc );

		cout << endl;
		cout << " EVIO Processing rate = " << rate << " Hz" << endl;
		cout << sdispatcher << endl;
		cout << sparser     << endl;
		cout << sprocessor  << endl;
	}
	
	// Delete all BOR objects
	for(auto p : borptrs_list) delete p;

	// Delete HDEVIO and print stats
	if(hdevio){
	    if(VERBOSE>0) evioout << "Closing hdevio event source \"" << GetResourceName() << "\"" <<endl;
		hdevio->PrintStats();
		delete hdevio;
	}

	// Delete HDET and print stats
	if(hdet){
	    if(VERBOSE>0) evioout << "Closing hdet event source \"" << GetResourceName() << "\"" <<endl;
		hdet->PrintStats();
		delete hdet;
	}
}

//----------------
// Dispatcher
//----------------
void JEventSource_EVIOpp::Dispatcher(void)
{
	/// This is run in a dedicated thread created by the constructor.
	/// It's job is to read in events and dispatch the processing of
	/// them to worker threads. When a worker thread is done, it adds
	/// the event to the parsed_events list and clears its own "in_use"
	/// flag thereby, marking itself as available for another job.
	/// The worker threads will stall if adding the event(s) it produced
	/// would make parsed_events contain more than MAX_PARSED_EVENTS.
	/// This creates backpressure here by having no worker threads
	/// available.
	
	if( BLOCKS_TO_SKIP>0 ) SkipEVIOBlocks(BLOCKS_TO_SKIP);
	
	bool allow_swap = false; // Defer swapping to DEVIOWorkerThread
	uint64_t istreamorder = 0;
	while(true){
	
		if(japp->IsQuitting()) break;

		// Get worker thread to handle this
		DEVIOWorkerThread *thr = NULL;
		while( !thr){
			for(auto t : worker_threads){
				if(t->in_use) continue;
				thr = t;
				break;
			}
			if(!thr) {
				NDISPATCHER_STALLED++;
				this_thread::sleep_for(milliseconds(1));
			}
			if(DONE) break;
		}
		if(DONE) break;
		
		uint32_t* &buff     = thr->buff;
		uint32_t  &buff_len = thr->buff_len;
		
		bool swap_needed = false;

		if(source_type==kFileSource){
			// ---- Read From File ----
//			hdevio->read(buff, buff_len, allow_swap);
//			hdevio->readSparse(buff, buff_len, allow_swap);
			hdevio->readNoFileBuff(buff, buff_len, allow_swap);
			thr->pos = hdevio->last_event_pos;
			if(hdevio->err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL){
				delete[] buff;
				buff_len = hdevio->last_event_len;
				buff = new uint32_t[buff_len];
				continue;
			}else if(hdevio->err_code!=HDEVIO::HDEVIO_OK){
				if(LOOP_FOREVER && NEVENTS_PROCESSED>=1){
					if(hdevio){
						hdevio->rewind();
						continue;
					}
				}else{
					// Some CDAQ files do not have a proper trailer at the end of the
					// EVIO file and so get reported by HDEVIO as truncated. We do not
					// want to treat those as an error, but DO want to treat legitimate
					// truncations as errors. If this is a CDAQ event and there are
					// exactly zero words left in the file then assume it is OK. 
					// n.b. the IS_CDAQ_FILE flag is set in DEVIOWorkerThread::ParseCDAQBank
					// during the parsing of a previous event. It is possible we could
					// reach here before that is set leading to a race condition!
					if( IS_CDAQ_FILE && (hdevio->err_code == HDEVIO::HDEVIO_FILE_TRUNCATED) && (hdevio->GetNWordsLeftInFile()==0) ){
						jout << "Missing EVIO file trailer in CDAQ file (ignoring..)" << endl; 
					}else{
						cout << hdevio->err_mess.str() << endl;
						if(hdevio->err_code != HDEVIO::HDEVIO_EOF){
							bool ignore_error = false;
							if( (!TREAT_TRUNCATED_AS_ERROR) && (hdevio->err_code == HDEVIO::HDEVIO_FILE_TRUNCATED) ) ignore_error = true;
							if(!ignore_error) japp->SetExitCode(hdevio->err_code);
						}
					}
				}
				break;
			}else{
				// HDEVIO_OK

				// skip some physics blocks if specified by user
				if(PHYSICS_BLOCKS_SKIPPED < PHYSICS_BLOCKS_TO_SKIP){
				    if( hdevio->GetCurrentBlockType() == HDEVIO::kBT_PHYSICS ){
                        PHYSICS_BLOCKS_SKIPPED++;
                        // buff is not byte swapped so number of events should be first
                        // byte of second word
                        //uint32_t M = (buff[1]>>24);
                        //Nevents_read += M;  // (this is never really used)
								//if(dapp) dapp->AddNEventsRead( M ); // This is so the JANA ticker will reflect how many events were actually read
								continue;
					 }
				}

                // quit after a fixed number of physics blocks if specified by user
				if( PHYSICS_BLOCKS_TO_KEEP > 0 ){
                    if( hdevio->GetCurrentBlockType() == HDEVIO::kBT_PHYSICS ){
                        PHYSICS_BLOCKS_KEPT++;
 				        if(PHYSICS_BLOCKS_KEPT > PHYSICS_BLOCKS_TO_KEEP) {
                            // We've already processed the max number of physics blocks requested
                            // by the user. Break outer loop signifying we are out of events.
                            break;
                        }
 				    }
				}

				swap_needed = hdevio->swap_needed;
			}
		}else{
			// ---- Read From ET ----
			hdet->read(buff, buff_len, allow_swap);
			thr->pos = 0;
			static uint64_t ntimeouts=0;
			if(hdet->err_code == HDET::HDET_TIMEOUT){
				if(et_quit_next_timeout) break;
				if( ++ntimeouts>=2 ){ // timeout is set to 2 seconds in HDET.cc
					int ic = ntimeouts%4;
					const char *c[4] = {"|","/","-","\\"};
					if(ntimeouts==2) cout << endl;
					cout << " ET stalled ... " << c[ic] << " \r";
					cout.flush();
				}
				continue;
			}else if(hdet->err_code != HDET::HDET_OK){
				cout << hdet->err_mess.str() << endl;
				break;
			}else{
				// HDET_OK
				swap_needed = hdet->swap_needed;
				if(ntimeouts>=2) cout << endl;
				ntimeouts=0;
			}
		}

		uint32_t myjobtype = jobtype;
		if(swap_needed && SWAP) myjobtype |= DEVIOWorkerThread::JOB_SWAP;
		
		// Wake up worker thread to handle event
		thr->jobtype = (DEVIOWorkerThread::JOBTYPE)myjobtype;
		thr->in_use = true;
		thr->istreamorder = istreamorder++;

		thr->cv.notify_all();
	}
	
	// If we get here then there are no more events in the source.
	// Wait for all worker threads to become available so we know 
	// the system is drained of events. Then set the DONE flag so
	// GetEvent will properly return NO_MORE_EVENTS_IN_SOURCE.
	for( auto w : worker_threads ){
		w->done = true;
		while(w->in_use){
			this_thread::sleep_for(milliseconds(10));
		}
	}

	DONE = true;

	// Free the worker threads (and their associated object pools)
	// to reduce overall memory consumption. This JEventSource_EVIOpp
	// object will not be deleted until just before the program exits
	// so waiting for the destructor call will cause unneccessary
	// memory use. (This is really for the case when more than one
	// file is being processed). Since deleting the DEVIOWorkerThread
	// object will also delete all of its DParsedEvents and
	// some of those may still be in use, we must wait for all
	// events to have their in_use flag set before deleting the
	// worker thread object.
	for( auto w : worker_threads ){
	
		while( true ){
			bool in_use = false;
			for( auto pe : w->parsed_event_pool ) in_use |= pe->in_use;
			if( !in_use ) break;
			if( DISPATCHER_END ) break;
			this_thread::sleep_for(milliseconds(10));
		}
		
		w->Finish();
		delete w;
	}
 	worker_threads.clear();
	
	tend = std::chrono::high_resolution_clock::now();
}

//----------------
// SkipEVIOBlocks
//----------------
jerror_t JEventSource_EVIOpp::SkipEVIOBlocks(uint32_t N)
{
	/// Read and discard a specified number of EVIO blocks.
	/// This is used to skip parsing completely for blocks
	/// of EVIO events where corruption is suspected. Set the
	/// number of EVIO blocks to skip using the EVIO:BLOCKS_TO_SKIP
	/// configuration parameter. Remember that for typical GlueX
	/// production data, one EVIO block contains 40 physics 
	/// events.

	uint32_t  buff_len = 20480;
	uint32_t* buff     = new uint32_t[buff_len];

	jout << "Skipping " << N << " EVIO blocks " << endl;
	while(N>0){
		hdevio->readNoFileBuff(buff, buff_len, false);
		if(hdevio->err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL){
			delete[] buff;
			buff_len = hdevio->last_event_len;
			buff = new uint32_t[buff_len];
			continue;
		}else if(hdevio->err_code!=HDEVIO::HDEVIO_OK){
			cout << hdevio->err_mess.str() << endl;
			if(hdevio->err_code != HDEVIO::HDEVIO_EOF){
				bool ignore_error = false;
				if( (!TREAT_TRUNCATED_AS_ERROR) && (hdevio->err_code == HDEVIO::HDEVIO_FILE_TRUNCATED) ) ignore_error = true;
				if(!ignore_error) japp->SetExitCode(hdevio->err_code);
			}
			break;
		}else{
			// HDEVIO_OK
			N--;
		}
	}
		
	if( buff )delete[] buff;
	
	return NOERROR;
}

//----------------
// Emit
//----------------
JEventSource::Result JEventSource_EVIOpp::Emit(JEvent& event)
{
	// Get next event from list, waiting if necessary
	unique_lock<std::mutex> lck(PARSED_EVENTS_MUTEX);
	while(parsed_events.empty()){
		if(DONE){
			return Result::FailureFinished;
		}
		NEVENTBUFF_STALLED++;
		PARSED_EVENTS_CV.wait_for(lck,std::chrono::milliseconds(1));
	}

	DParsedEvent *pe = parsed_events.front();
	parsed_events.pop_front();
	
	// Release mutex and notify workers they can use it again
	lck.unlock();
	PARSED_EVENTS_CV.notify_all();
	
	// If this is a BOR event, then take ownership of
	// the DBORptrs object. If not, then copy a pointer
	// to the latest DBORptrs object into the event.
	if(pe->borptrs) borptrs_list.push_front(pe->borptrs);

	// Copy info for this parsed event into the JEvent
	event.SetEventNumber(pe->event_number);
	event.SetRunNumber(USER_RUN_NUMBER>0 ? USER_RUN_NUMBER:pe->run_number);
	event.Insert(pe)->SetFactoryFlag(JFactory::NOT_OBJECT_OWNER);   // Returned to pool in FinishEvent

	// Set event status bits
	DStatusBits* statusBits = new DStatusBits;
	statusBits->SetStatus(pe->event_status_bits);

	statusBits->SetStatusBit(kSTATUS_EVIO);
	if( source_type == kFileSource ) statusBits->SetStatusBit(kSTATUS_FROM_FILE);
	if( source_type == kETSource   ) statusBits->SetStatusBit(kSTATUS_FROM_ET);

	event.Insert(statusBits);

	// EPICS and BOR events are barrier events
	if(statusBits->GetStatusBit(kSTATUS_EPICS_EVENT)) event.SetSequential(true);
	if(statusBits->GetStatusBit(kSTATUS_BOR_EVENT  )) event.SetSequential(true);
	
	// Only add BOR events to physics events
	if(pe->borptrs==NULL)
		if(!borptrs_list.empty()) pe->borptrs = borptrs_list.front();
    
	return Result::Success;
}

//----------------
// FreeEvent
//----------------
void JEventSource_EVIOpp::FinishEvent(JEvent &event)
{
	// CAUTION: This is called by the EventBufferThread in JANA which
	// effectively serializes everything done here. (Don't delete all
	// objects in pe which can be slow.)
	DParsedEvent *pe = const_cast<DParsedEvent*>(event.GetSingle<DParsedEvent>());
	pe->in_use = false; // return pe to pool
	
	NEVENTS_PROCESSED++;
}

//----------------
// GetObjects
//----------------
bool JEventSource_EVIOpp::GetObjects(const std::shared_ptr<const JEvent> &event, JFactory *factory)
{
	// This callback is called to extract objects of a specific type from
	// an event and store them in the factory pointed to by JFactory_base.
	// The data type desired can be obtained via factory->GetDataClassName()
	// and the tag via factory->Tag().
	//
	// If the object is not one of a type this source can provide, then
	// it should return OBJECT_NOT_AVAILABLE. Otherwise, it should return
	// NOERROR;
	
	// When first called, the data objects will still be in the DParsedEvent
	// object and not yet copied into their respective factories. This will
	// do that copy of the pointers and set the "copied_to_factories" flag
	// in DParsedEvent indicating this has been done. It will then check the
	// name of the class being requested and return the appropriate value
	// depending on whether we supply that class or not.
	
	// We must have a factory to hold the data
	if(!factory)throw RESOURCE_UNAVAILABLE;

	// Get name of data class we're trying to extract and the factory tag
	string dataClassName = factory->GetObjectName();
	string tag = factory->GetTag();
	if(tag.length()!=0) return false; // can't provide tagged factory objects
	
	// Get any translation tables we'll need to apply
	vector<const DTranslationTable*> translationTables;
	if(APPLY_TRANSLATION_TABLE){
		event->Get<DTranslationTable>(translationTables);
	}
  
	// Copy pointers to all hits to appropriate factories.
	// Link BORconfig objects and apply translation tables if appropriate.
	DParsedEvent *pe = const_cast<DParsedEvent*>(event->GetSingle<DParsedEvent>());
	if(!pe->copied_to_factories){

		// Optionally link BOR object associations
		if(LINK_BORCONFIG && pe->borptrs) LinkBORassociations(pe);
		
		// Optionally emulate flash firmware
		if(!pe->vDf250WindowRawData.empty()) EmulateDf250Firmware(pe);
		if(!pe->vDf125WindowRawData.empty()) {
			if(APPLY_TRANSLATION_TABLE)
				EmulateDf125Firmware(pe, translationTables[0]);
			else 
				EmulateDf125Firmware(pe, nullptr);
		}

		// Copy all low-level hits to appropriate factories
		pe->CopyToFactories(event);

		// Apply translation tables to create DigiHit objects
		for(auto tt : translationTables){
			tt->ApplyTranslationTable(event);
		}
	}
	
	// Optionally record call stack
	if(RECORD_CALL_STACK) AddToCallStack(pe, event);

	// Decide whether this is a data type the source supplies
    // If the data type is that of some derived data that is nominally
    // supplied by another factory, but is being stored in EVIO format
    // for some special reason (e.g. calibration skims), then we 
    // say that we supply this data ONLY IF we actually have data
    // of this type.  Otherwise, we feign ignorance so that the 
    // data is produced by its usual factory
    bool isSuppliedType = pe->IsParsedDataType(dataClassName)
                           && pe->IsNonEmptyDerivedDataType(dataClassName);
	for(auto tt : translationTables){
		if(isSuppliedType) break;  // once this is set, it's set
		isSuppliedType = tt->IsSuppliedType(dataClassName);
	}

	// Check if this is a class we provide and return appropriate value
	return isSuppliedType;
}

//----------------
// LinkBORassociations
//----------------
void JEventSource_EVIOpp::LinkBORassociations(DParsedEvent *pe)
{
	/// Add BORConfig objects as associated objects
	/// to selected hit objects. Most other object associations
	/// are made in DEVIOWorkerThread::LinkAllAssociations. The
	/// BORConfig objects however, are not available when that
	/// is called.
	/// This is called from GetObjects() which is called from
	/// one of the processing threads.
	
	// n.b. the values of nsamples_integral and nsamples_pedestal
	// in the Df250PulseData objects that were copied from the 
	// config objcts will be overwritten here with values from
	// the BOR record (if available). f125 and older f250 data
	// doesn't do this because nped is not available there.
	//
	// n.b. all of the BORConfig object vectors will already
	// be sorted by rocid, then slot when the data was parsed.
	// The other hit objects are also already sorted (in
	// DEVIOWorkerThread::LinkAllAssociations).

	DBORptrs *borptrs = pe->borptrs;

	LinkModule(borptrs->vDf250BORConfig, pe->vDf250WindowRawData);
	LinkModule(borptrs->vDf250BORConfig, pe->vDf250PulseIntegral);
	LinkModuleBORSamplesCopy(borptrs->vDf250BORConfig, pe->vDf250PulseData);

	LinkModule(borptrs->vDf125BORConfig, pe->vDf125WindowRawData);
	LinkModule(borptrs->vDf125BORConfig, pe->vDf125PulseIntegral);
	LinkModule(borptrs->vDf125BORConfig, pe->vDf125CDCPulse);
	LinkModule(borptrs->vDf125BORConfig, pe->vDf125FDCPulse);

	LinkModule(borptrs->vDF1TDCBORConfig, pe->vDF1TDCHit);

	LinkModule(borptrs->vDCAEN1290TDCBORConfig, pe->vDCAEN1290TDCHit);

}

////----------------
//// Cleanup
////----------------
//void JEventSource_EVIOpp::Cleanup(void)
//{
//	/// This is called internally by the JEventSource_EVIO class
//	/// once all events have been read in. Its purpose is to
//	/// free the hidden memory in all of the container class
//	/// members of the JEventSource_EVIO class. This is needed
//	/// for jobs that process a lot of input files and therefore
//	/// create a lot JEventSource_EVIO objects. JANA does not delete
//	/// these objects until the end of the job so this tends to
//	/// act like a memory leak. The data used can be substantial
//	/// (nearly 1GB per JEventSource_EVIO object).
//
//	if(hdevio) delete hdevio;
//	hdevio = NULL;
//
//	if(hdet) delete hdet;
//	hdet = NULL;
//
//}

//----------------
// SearchFileForRunNumber
//----------------
uint64_t JEventSource_EVIOpp::SearchFileForRunNumber(void)
{
	/// This is called from the constructor when reading
	/// from a file to seed the default run number. This
	/// is needed because the first event is probably a BOR
	/// event which does not contain the run number. The run
	/// number would then be reported as "0" and incorrect 
	/// calibrations and field maps would be read in. This 
	/// will try searching past the first (BOR) event for 
	/// either a physics event, or an EPICS event that has
	/// the run number defined in it.
	///
	/// The value returned from this will be copied into each
	/// DEVIOWorkerThread object when it is created. It will
	/// be used to initialize the run number for each event
	/// the worker thread processes. If the worker thread 
	/// processes a physics event containing a run number,
	/// then that will overwrite the run number in the
	/// DParsedEvent. Finally, if the user specified a run
	/// number, then that will be reported to JANA in lieu of
	/// the one in DParsedEvent.

	std::string source_name = GetResourceName();

	if(VERBOSE>2) evioout << "     In JEventSource_EVIOpp::SearchFileForRunNumber() source_name=" << source_name << " ..." << endl;

	uint32_t buff_len = 4000000;
	uint32_t *buff = new uint32_t[buff_len];
	HDEVIO *hdevio = new HDEVIO(source_name, false, VERBOSE);
	while(hdevio->readNoFileBuff(buff, buff_len)){

		// Assume first word is number of words in bank
		uint32_t *iptr = buff;
		uint32_t *iend = &iptr[*iptr - 1];
		if(VERBOSE>2) evioout << "Checking event with header= 0x" << hex << iptr[1] << dec << endl;
		if(*iptr > 2048) iend = &iptr[2048]; // only search the first 2kB of each event
		bool has_timestamps = false;
		while(iptr<iend){
			iptr++;

			// EPICS event
			if( (*iptr & 0xff000f) ==  0x600001){
				if(VERBOSE>2) evioout << "     Found EPICS header. Looking for HD:coda:daq:run_number ..." << endl;
				const char *cptr = (const char*)&iptr[1];
				const char *cend = (const char*)iend;
				const char *needle = "HD:coda:daq:run_number=";
				while(cptr<cend){
					if(VERBOSE>4) evioout << "       \""<<cptr<<"\"" << endl;
					if(!strncmp(cptr, needle, strlen(needle))){
						if(VERBOSE>2) evioout << "     Found it!" << endl;
						uint64_t run_number_seed = atoi(&cptr[strlen(needle)]);
						if(hdevio) delete hdevio;
						if(buff) delete[] buff;
						return run_number_seed;
					}
					cptr+=4; // should only start on 4-byte boundary!
				}
			}

			// BOR event from CODA ER
			if( (*iptr & 0xffffffff) ==  0x00700E01) continue;
			
			// BOR event from CDAQ ER
			// Prior to Fall 2019 this was 0x00700e34 where the 34 represented the number of crates
			// Two crates were added in Fall 2019 so this changed to 0x00700e36. To future-proof
			// this, we ignore the last 8 bits and hope this is unique enough not to get confused!
			//if( (*iptr & 0xffffffff) ==  0x00700e34){
			if( (*iptr & 0xffffff00) ==  0x00700e00){
				iptr++;
				uint32_t crate_len    = iptr[0];
				uint32_t crate_header = iptr[1];
				uint32_t *iend_crate  = &iptr[crate_len];

				// Make sure crate tag is right
				if( (crate_header>>16) == 0x71 ){

					// Loop over modules
					iptr += 2;
					while(iptr<iend_crate){
						uint32_t module_header = *iptr++;
						uint32_t module_len    = module_header&0xFFFF;
						uint32_t modType       = (module_header>>20)&0x1f;

						if(  modType == DModuleType::CDAQTSG){
							uint64_t run_number_seed = iptr[0];
							if(hdevio) delete hdevio;
							if(buff) delete[] buff;
							return run_number_seed;
						}
						iptr = &iptr[module_len];
					}
					iptr = iend_crate; // ensure we're pointing past this crate
				}

				continue; // didn't find it in this CDAQ BOR. Keep looking
			}

			// PHYSICS event
			bool not_in_this_buffer = false;
			switch((*iptr)>>16){
				case 0xFF10:
				case 0xFF11:
				case 0xFF20:
				case 0xFF21:
				case 0xFF24:
				case 0xFF25:
				case 0xFF30:
					not_in_this_buffer = true;
					break;
				case 0xFF23:
				case 0xFF27:
					has_timestamps = true;
				case 0xFF22:
				case 0xFF26:
					break;
				default:
					continue;
			}

			if(not_in_this_buffer) break; // go to next EVIO buffer

			iptr++;
			if( ((*iptr)&0x00FF0000) != 0x000A0000) { iptr--; continue; }
			uint32_t M = iptr[-3] & 0x000000FF; // Number of events from Physics Event header
			if(VERBOSE>2) evioout << "       ...(epic quest) Trigger bank " << (has_timestamps ? "does":"doesn't") << " have timestamps. Nevents in block M=" << M <<endl;
			iptr++;
			uint64_t *iptr64 = (uint64_t*)iptr;

			uint64_t event_num = *iptr64;
			if(VERBOSE>3) evioout << "       ....(epic quest) Event num: " << event_num <<endl;
			iptr64++;
			if(has_timestamps) iptr64 = &iptr64[M]; // advance past timestamps

			uint64_t run_number_seed = (*iptr64)>>32;
			if(VERBOSE>1) evioout << "       .. (epic quest) Found run number: " << run_number_seed <<endl;

			if(hdevio) delete hdevio;
			if(buff) delete[] buff;
			return run_number_seed;

		} // while(iptr<iend)

		if(hdevio->Nevents > 500){
			if(VERBOSE>2) evioout << "       more than 500 events checked and no run number seen! abondoning search" << endl;
			break;
		}

	} // while(hdevio->read(buff, buff_len))
	
	if(hdevio->err_code != HDEVIO::HDEVIO_OK) evioout << hdevio->err_mess.str() << endl;

	if(hdevio) delete hdevio;
	if(buff) delete[] buff;

	if(VERBOSE>2) evioout << "     failed to find run number. Returning 0" << endl;

	return 0;
}

//----------------
// EmulateDf250Firmware
//----------------
void JEventSource_EVIOpp::EmulateDf250Firmware(DParsedEvent *pe)
{
	/// This is called from GetObjects, but only if window raw
	/// data objects exist. It will emulate the firmware, producing
	/// hit objects from the sample data.
	///
	/// There are some complications here worth noting regarding the
	/// behavior of the firmware used for Spring 2016 and earlier
	/// data. The firmware will actually look for a pulse in the
	/// window and if found, emit pulse time and pulse pedestal
	/// words for it. It will NOT emit a pulse integral word. Further,
	/// it will produce those words for at most one pulse. If running
	/// in mode 7 however, (i.e. no window raw data) it will produce
	/// multiple pulses. Thus, to mimic the behavior of mode 7, the
	/// emulator will produce multiple pulses. 
	///
	/// The exact behavior of this is determined by the F250_EMULATION_MODE
	/// parameter which may be one of 3 values. The default is
	/// kEmulationAuto which does the following:
	/// 1. Produce hit objects for all pulses found in window
	/// 2. If pulse time and pedestal objects exist from firmware
	///    then copy the first found hit into the "*_emulated"
	///    fields of those existing objects and discard the emulated
	///    hit.
	/// 3. Add the remaining emulated hits to the event.
	///
	/// The end result is that the first pulse time and pulse pedestal
	/// objects may contain some combination of emulated and firmware
	/// generated information. All pulse integral objects will be
	/// emulated. It may also be worth noting that this is done in such
	/// a way that the data may contain a mixture of mode 8 data 
	/// from some boards and mode 7 from others. It will not affect
	/// those boards producing mode 7 (e.g. won't delete their hits
	/// even if F250_EMULATION_MODE is set to kEmulationAlways).

	if(F250_EMULATION_MODE == kEmulationNone) return;

    // The output data format of the firmware versions is different
    // so we handle their emulation separately...
    if(F250_EMULATION_VERSION == 1) {   // pre-Fall 2016

        for(auto wrd : pe->vDf250WindowRawData){
            const Df250PulseTime     *cf250PulseTime     = NULL;
            const Df250PulsePedestal *cf250PulsePedestal = NULL;
            
            try{ wrd->GetSingle(cf250PulseTime);     }catch(...){}
            try{ wrd->GetSingle(cf250PulsePedestal); }catch(...){}
            
            Df250PulseTime     *f250PulseTime     = (Df250PulseTime*    )cf250PulseTime;
            Df250PulsePedestal *f250PulsePedestal = (Df250PulsePedestal*)cf250PulsePedestal;
            
            // Emulate firmware
            vector<Df250PulseTime*>     em_pts;
            vector<Df250PulsePedestal*> em_pps;
            vector<Df250PulseIntegral*> em_pis;
            f250Emulator->EmulateFirmware(wrd, em_pts, em_pps, em_pis);
            
            // Spring 2016 and earlier data may have one pulse time and
            // one pedestal object in mode 8 data. Emulation mimics mode
            // 7 which may have more. If there are firmware generated 
            // pulse time/pedestal objects, copy the first emulated hit's
            // info into them and then the rest of the emulated hits into
            // appropriate lists.
            if(!em_pts.empty() && f250PulseTime){
                Df250PulseTime *em_pt = em_pts[0];
                f250PulseTime->time_emulated = em_pt->time_emulated;
                f250PulseTime->quality_factor_emulated = em_pt->quality_factor_emulated;
                if(F250_EMULATION_MODE == kEmulationAlways){
                    f250PulseTime->time = em_pt->time;
                    f250PulseTime->quality_factor = em_pt->quality_factor;
                    f250PulseTime->emulated = true;
                }
                delete em_pt;
                em_pts.erase(em_pts.begin());
            }

            if(!em_pps.empty() && f250PulsePedestal){
                Df250PulsePedestal *em_pp = em_pps[0];
                f250PulsePedestal->pedestal_emulated   = em_pp->pedestal_emulated;
                f250PulsePedestal->pulse_peak_emulated = em_pp->pulse_peak_emulated;
                if(F250_EMULATION_MODE == kEmulationAlways){
                    f250PulsePedestal->pedestal   = em_pp->pedestal_emulated;
                    f250PulsePedestal->pulse_peak = em_pp->pulse_peak_emulated;
                    f250PulsePedestal->emulated = true;
                }
                delete em_pp;
                em_pps.erase(em_pps.begin());
            }

            pe->vDf250PulseTime.insert(pe->vDf250PulseTime.end(),         em_pts.begin(), em_pts.end());
            pe->vDf250PulsePedestal.insert(pe->vDf250PulsePedestal.end(), em_pps.begin(), em_pps.end());
            pe->vDf250PulseIntegral.insert(pe->vDf250PulseIntegral.end(), em_pis.begin(), em_pis.end());
        }

    } else if(F250_EMULATION_VERSION == 2) {   // Fall 2016 -> ?

        for(auto wrd : pe->vDf250WindowRawData){
            // See if we need to remake Df250PulseData objects?
            vector<const Df250PulseData*> cpdats;   // existing pulse data objects
            try{ wrd->Get(cpdats); }catch(...){}

            vector<Df250PulseData*> pdats;
            for(auto cpdat : cpdats) 
                pdats.push_back((Df250PulseData*)cpdat);

	    // Sort the pulses since we apparently don't always get them in the right order
	    sort(pdats.begin(), pdats.end(), sortf250pulsenumbers);

            // Flag all objects as emulated and their values will be replaced with emulated quantities
            if (F250_EMULATION_MODE == kEmulationAlways){
                for(auto pdat : pdats)
                    pdat->emulated = 1;
            }

            // Emulate firmware
            f250Emulator->EmulateFirmware(wrd, pdats);
				
	    // Above call overwrites values with emulated values, but may also
	    // find additional pulses. Add any extra pulse data objects found
	    // to end of list
	    for(uint32_t i=cpdats.size(); i<pdats.size(); i++){
		    pe->vDf250PulseData.push_back(pdats[i]);
	    }
        }

    } else if(F250_EMULATION_VERSION == 3) {   // Fall 2019 -> ?

        for(auto wrd : pe->vDf250WindowRawData){
            // See if we need to remake Df250PulseData objects?
            vector<const Df250PulseData*> cpdats;   // existing pulse data objects
            try{ wrd->Get(cpdats); }catch(...){}

            vector<Df250PulseData*> pdats;
            for(auto cpdat : cpdats) 
                pdats.push_back((Df250PulseData*)cpdat);

	    // Sort the pulses since we apparently don't always get them in the right order
	    sort(pdats.begin(), pdats.end(), sortf250pulsenumbers);

            // Flag all objects as emulated and their values will be replaced with emulated quantities
            if (F250_EMULATION_MODE == kEmulationAlways){
                for(auto pdat : pdats)
                    pdat->emulated = 1;
            }

            // Emulate firmware
            f250Emulator->EmulateFirmware(wrd, pdats);
				
	    // Above call overwrites values with emulated values, but may also
	    // find additional pulses. Add any extra pulse data objects found
	    // to end of list
	    for(uint32_t i=cpdats.size(); i<pdats.size(); i++){
		    pe->vDf250PulseData.push_back(pdats[i]);
	    }
        }

    } else {
        stringstream ss;
        ss << "Invalid fADC250 firmware version specified: " << F250_EMULATION_VERSION;
        throw JException(ss.str());
    }
	
	// One last bit of nastiness here. Because the emulator creates
	// new objects rather than pulling them from the DParsedEvent pools,
	// the pools will tend to get filled with them. Delete any pool
	// objects for the types the emulator creates to prevent memory-
	// leak-like behavior.
	for(auto p : pe->vDf250PulseTime_pool    ) delete p;
	for(auto p : pe->vDf250PulsePedestal_pool) delete p;
	for(auto p : pe->vDf250PulseIntegral_pool) delete p;
	pe->vDf250PulseTime_pool.clear();
	pe->vDf250PulsePedestal_pool.clear();
	pe->vDf250PulseIntegral_pool.clear();
	
}

//----------------
// EmulateDf125Firmware
//----------------
void JEventSource_EVIOpp::EmulateDf125Firmware(DParsedEvent *pe, const DTranslationTable *ttab)
{
	/// This is called from GetObjects, but only if window raw
	/// data objects exist. It will emulate the firmware, producing
	/// hit objects from the sample data.

	if(F125_EMULATION_MODE == kEmulationNone) return;

	for(auto wrd : pe->vDf125WindowRawData){
		const Df125CDCPulse *cf125CDCPulse = NULL;
		const Df125FDCPulse *cf125FDCPulse = NULL;

		try{ wrd->GetSingle(cf125CDCPulse); }catch(...){}
		try{ wrd->GetSingle(cf125FDCPulse); }catch(...){}

		Df125CDCPulse *f125CDCPulse = (Df125CDCPulse*)cf125CDCPulse;
		Df125FDCPulse *f125FDCPulse = (Df125FDCPulse*)cf125FDCPulse;

		// If the the pulse objects do not exist, create new ones to go with our raw data
		// This should rarely happen since CDC_long and FDC_long mores have the raw data
		// along with the calculated quantities in a pulse word. Pure raw mode would be the only time
		// when this would not be the case. 
	        // While this is infrequently used, usually different detectors use 
	        // different types, so let's pick the pulse type based on the 
	        // translated detector info (this came up in FMWPC testing)
	
		DTranslationTable::Detector_t locDetector = DTranslationTable::UNKNOWN_DETECTOR;
		if(ttab) {
 			auto rocid_by_system = ttab->Get_ROCID_By_System();
 			if( rocid_by_system[ DTranslationTable::CDC ].count( wrd->rocid ) != 0 )
 				locDetector = DTranslationTable::CDC;
 			else if( rocid_by_system[ DTranslationTable::FDC_CATHODES ].count( wrd->rocid ) != 0 )
 				locDetector = DTranslationTable::FDC_CATHODES;
 			else if( rocid_by_system[ DTranslationTable::FMWPC ].count( wrd->rocid ) != 0 )
 				locDetector = DTranslationTable::FMWPC;
		}

	        if(f125CDCPulse == NULL && ( locDetector == DTranslationTable::CDC || locDetector == DTranslationTable::FMWPC ) ){
	            f125CDCPulse           = pe->NEW_Df125CDCPulse();
	            f125CDCPulse->rocid    = wrd->rocid;
	            f125CDCPulse->slot     = wrd->slot;
	            f125CDCPulse->channel  = wrd->channel;
	            f125CDCPulse->emulated = true;
	            f125CDCPulse->AddAssociatedObject(wrd);
	        }
	        else if(f125FDCPulse == NULL && ( locDetector == DTranslationTable::FDC_CATHODES ) ){
	            f125FDCPulse           = pe->NEW_Df125FDCPulse();
	            f125FDCPulse->rocid    = wrd->rocid;
	            f125FDCPulse->slot     = wrd->slot;
	            f125FDCPulse->channel  = wrd->channel;
	            f125FDCPulse->emulated = true;
	            f125FDCPulse->AddAssociatedObject(wrd);
	        }
	        
	        // if there's no translation table or the hits have no pulse data
	        // and can't be matched to a known detector (not a good situation!)
	        // then give up
	        if( f125FDCPulse == NULL && f125CDCPulse == NULL ) return;
	
		// Flag all objects as emulated and their values will be replaced with emulated quantities
		if (F125_EMULATION_MODE == kEmulationAlways){
			if(f125CDCPulse!=NULL) f125CDCPulse->emulated = 1;
			if(f125FDCPulse!=NULL) f125FDCPulse->emulated = 1;
		}

		// Perform the emulation
		f125Emulator->EmulateFirmware(wrd, f125CDCPulse, f125FDCPulse);
	}
}

//----------------
// AddToCallStack
//----------------
void JEventSource_EVIOpp::AddToCallStack(DParsedEvent *pe, const std::shared_ptr<const JEvent>& loop)
{
	/// Add information to JANA's call stack so that the janadot
	/// plugin can better display the actual relationship of the
	/// the data objects. This only gets called if the RECORD_CALL_STACK
	/// config. parameter is set (which is done automatically by
	/// the janadot plugin)

	// Add all source object types as defined in DParsedEvent
	vector<string> sourcetypes;
	pe->GetParsedDataTypes(sourcetypes);
	for(auto t:sourcetypes) AddSourceObjectsToCallStack(loop, t);
}

//----------------
// AddSourceObjectsToCallStack
//----------------
void JEventSource_EVIOpp::AddSourceObjectsToCallStack(const std::shared_ptr<const JEvent>& loop, string className)
{
	/// This is used to give information to JANA regarding the origin of objects
	/// that *should* come from the source. We add them in explicitly because
	/// the file may not have any, but factories may ask for them. We want those
	/// links to indicate that the "0" objects in the factory came from the source
	/// so that janadot draws these objects correctly.
	// TODO: NWB: Re-add me

	// JEventLoop::call_stack_t cs;
	// cs.caller_name = "<ignore>"; // tells janadot this object wasn't actually requested by anybody
	// cs.caller_tag = "";
	// cs.callee_name = className;
	// cs.callee_tag = "";
	// cs.start_time = 0.0;
	// cs.end_time = 0.0;
	// cs.data_source = JEventLoop::DATA_FROM_SOURCE;
	// loop->AddToCallStack(cs);
}

//----------------
// AddEmulatedObjectsToCallStack
//----------------
void JEventSource_EVIOpp::AddEmulatedObjectsToCallStack(const std::shared_ptr<const JEvent>& loop, string caller, string callee)
{
	/// This is used to give information to JANA regarding the relationship and
	/// origin of some of these data objects. This is really just needed so that
	/// the janadot program can be used to produce the correct callgraph. Because
	/// of how this plugin works, JANA can't record the correct call stack (at
	/// least not easily!) Therefore, we have to give it a little help here.

	// TODO: NWB: Re-add me
	// JEventLoop::call_stack_t cs;
	// cs.caller_name = caller;
	// cs.callee_name = callee;
	// cs.data_source = JEventLoop::DATA_FROM_SOURCE;
	// loop->AddToCallStack(cs);
	// cs.callee_name = cs.caller_name;
	// cs.caller_name = "<ignore>";
	// cs.data_source = JEventLoop::DATA_FROM_FACTORY;
	// loop->AddToCallStack(cs);
}


//----------------
// DumpBinary
//----------------
void JEventSource_EVIOpp::DumpBinary(const uint32_t *iptr, const uint32_t *iend, uint32_t MaxWords, const uint32_t *imark)
{
    /// This is used for debugging. It will print to the screen the words
    /// starting at the address given by iptr and ending just before iend
    /// or for MaxWords words, whichever comes first. If iend is NULL,
    /// then MaxWords will be printed. If MaxWords is zero then it is ignored
    /// and only iend is checked. If both iend==NULL and MaxWords==0, then
    /// only the word at iptr is printed.

    cout << "Dumping binary: istart=" << hex << iptr << " iend=" << iend << " MaxWords=" << dec << MaxWords << endl;

    if(iend==NULL && MaxWords==0) MaxWords=1;
    if(MaxWords==0) MaxWords = (uint32_t)0xffffffff;

    uint32_t Nwords=0;
    while(iptr!=iend && Nwords<MaxWords){

        // line1 is hex and line2 is decimal
        stringstream line1, line2;

        // print words in columns 8 words wide. First part is
        // reserved for word number
        uint32_t Ncols = 8;
        line1 << setw(5) << Nwords;
        line2 << string(5, ' ');

        // Loop over columns
        for(uint32_t i=0; i<Ncols; i++, iptr++, Nwords++){

            if(iptr == iend) break;
            if(Nwords>=MaxWords) break;

            stringstream iptr_hex;
            iptr_hex << hex << "0x" << *iptr;

            string mark = (iptr==imark ? "*":" ");

            line1 << setw(12) << iptr_hex.str() << mark;
            line2 << setw(12) << *iptr << mark;
        }

        cout << line1.str() << endl;
        cout << line2.str() << endl;
        cout << endl;
    }
}
