
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <thread>
#include <sstream>
#include <numeric>

using namespace std;

#include <TFile.h>

#include "HDEVIOWriter.h"

#undef _DBG_
#undef _DBG__

#include <DAQ/HDEVIO.h>


enum EventType_t {
	kUNKNOWN_EVENT,
	kEPICS_EVENT,
	kBOR_EVENT,
	kCONTROL_EVENT,
	kCODA_PHYSICS_EVENT,
	kCDAQ_PHYSICS_EVENT
};


void Usage(string mess);

void ParseCommandLineArguments(int narg, char *argv[]);

void MakeSkim(string &fname);

bool ProcessEvent(uint32_t *buff, uint32_t buff_len);

void GetEventInfo(uint32_t *buff, uint32_t buff_len, EventType_t &type, uint32_t &Mevents);

void GetTrigMasks(uint32_t *buff, uint32_t buff_len, uint32_t Mevents, vector <uint32_t> &trig_masks,
                  vector <uint32_t> &fp_trig_masks);


vector <string> filenames;
string   USER_OFILENAME = "";
uint64_t MAX_EVIO_EVENTS = 1000000;
uint64_t SKIP_EVIO_EVENTS = 0;
uint64_t Nevents = 0;
uint64_t Nevents_saved = 0;
bool     WRITE_SQL_FILE = false;
bool     FP_IGNORE[32];

uint32_t NGTP_TOTAL[32];
uint32_t NFP_TOTAL[32];

uint32_t NEVIO_BLOCKS_TOTAL = 0;
uint32_t NEVENTS_TOTAL = 0;
uint32_t NEVENTS_BY_TYPE[16];

uint32_t RUN_NUMBER = 0;          // will be reset and overwritten for each file processed
uint64_t FIRST_EVENT_NUMBER = 0;  // will be reset and overwritten for each file processed
uint64_t LAST_EVENT_NUMBER = 0;   // will be reset and overwritten for each file processed

//----------------
// main
//----------------
int main(int narg, char *argv[]) {

	ParseCommandLineArguments(narg, argv);

	// Loop over input files
	for (auto fname : filenames) MakeSkim(fname);

	return 0;
}

//----------------
// Usage
//----------------
void Usage(string mess = "") {
	cout << endl;
	cout << "Usage:" << endl;
	cout << endl;
	cout << "    hdskims [options] file.evio [file2.evio ...]" << endl;
	cout << endl;
	cout << "options:" << endl;
	cout << "   -h, --help        Print this usage statement" << endl;
	cout << "   -m max_events     Max. EVIO events (not physics events) to process." << endl;
	cout << "   -i ignore_events  Num. EVIO events (not physics events) to ignore at start." << endl;
	cout << "   -o outfilename    Output filename (only use with single input file!)." << endl;
	cout << "   --sql             Write SQL for entering into skiminfo table to an outputfile" << endl;
	cout << "                     (name will be same as outputfile but with .sql extension)" << endl;
	cout << endl;
	cout << "n.b. When using the -i (ignore) flag, the total number of events" << endl;
	cout << "     read in will be the sum of how many are ignored and the \"max\"" << endl;
	cout << "     events with only the last max_events being processed." << endl;
	cout << endl;

	if (mess != "") cout << endl << mess << endl << endl;

	exit(0);
}

//----------------
// ParseCommandLineArguments
//----------------
void ParseCommandLineArguments(int narg, char *argv[]) {

	if (narg < 2) Usage("You must supply a filename!");
	
	// By default, all FP triggers can cause a block to be written out
	for(int itrig=0; itrig<32; itrig++) FP_IGNORE[itrig] = false;

	for (int i = 1; i < narg; i++) {
		string arg = argv[i];
		string next = (i + 1) < narg ? argv[i + 1] : "";

		if (arg == "-h" || arg == "--help") Usage();
		else if (arg == "-i") {
			SKIP_EVIO_EVENTS = atoi(next.c_str());
			i++;
		}
		else if (arg == "-m") {
			MAX_EVIO_EVENTS = atoi(next.c_str());
			i++;
		}
		else if (arg == "-o") {
			USER_OFILENAME = next.c_str();
			i++;
		}
		else if (arg == "-fp") {
			int itrig = atoi(next.c_str());
			if(itrig<0 || itrig>=32){
				cerr<<"ERROR: argument to -fp option must be number in 0-31 range" << endl;
				exit(-1);
			}
			FP_IGNORE[itrig] = true;
			i++;
		}
		else if (arg == "-sql" || arg == "--sql") {
			WRITE_SQL_FILE = true;
		}
		else if (arg[0] == '-') {
			cout << "Unknown option \"" << arg << "\" !" << endl;
			exit(-1);
		}
		else filenames.push_back(arg);
	}

	if ((filenames.size() > 1) && (USER_OFILENAME.length() > 0)) {
		Usage("ERROR: You may only use the -o option with a single input file! (otherwise output skim files will overwrite one another)");
	}
	
	cout << "The following FP triggers will be saved: ";
	for(int itrig=0; itrig<16; itrig++) if(!FP_IGNORE[itrig]) cout << itrig << ",";
	cout << endl; 
}


//----------------
// MakeSkim
//----------------
void MakeSkim(string &filename) {
	string ofilename = filename;
	auto pos = ofilename.find(".evio");
	ofilename.erase(pos);
	ofilename += "_skims.evio";
	pos = ofilename.find_last_of('/');
	if (pos != string::npos) ofilename.erase(0, pos + 1);

	if (USER_OFILENAME.length() > 0) ofilename = USER_OFILENAME;

	// Reset globals used to capture run and event info
	RUN_NUMBER = 0;
	FIRST_EVENT_NUMBER = 0;
	LAST_EVENT_NUMBER = 0;
	for( int itype=0; itype<16; itype++) NEVENTS_BY_TYPE[itype] = 0;
	for (int ibit = 0; ibit < 32; ibit++) NFP_TOTAL[ibit] = NGTP_TOTAL[ibit] = 0;

	// Open EVIO input file
	cout << "Processing file: " << filename << " -> " << ofilename << endl;
	HDEVIO *hdevio = new HDEVIO(filename);
	if (!hdevio->is_open) {
		cout << hdevio->err_mess.str() << endl;
		return;
	}
	auto Nwords_in_input_file = hdevio->GetNWordsLeftInFile();

	// Open EVIO output file
	HDEVIOWriter evioout(ofilename);
	std::thread thr(HDEVIOOutputThread, &evioout);

	// Read all events in file
	vector <uint32_t> *vbuff = evioout.GetBufferFromPool();
	bool done = false;
	while (!done) {

		uint32_t *buff = vbuff->data();
		uint32_t buff_len = vbuff->capacity();
		vbuff->resize(buff_len); // we do this here so when we resize below, it does not re-initialize the contents
		hdevio->readNoFileBuff(buff, buff_len);

		switch (hdevio->err_code) {
			case HDEVIO::HDEVIO_OK:
				if (Nevents >= SKIP_EVIO_EVENTS) {
					if (ProcessEvent(buff, buff_len)) {
						vbuff->resize((*vbuff)[0] + 1);
						evioout.AddBufferToOutput(vbuff); // HDEVIOWriter takes ownership of buffer
						vbuff = evioout.GetBufferFromPool(); // get a new buffer
						Nevents_saved++;
					}
				}
				break;
			case HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL:
				vbuff->reserve(hdevio->last_event_len);
				break;
			case HDEVIO::HDEVIO_EOF:
				cout << endl << " end of file" << endl;
				done = true;
				break;
			default:
				cout << endl;
				cout << hdevio->err_mess.str() << endl;
				done = true;
				break;
		}

		if ((++Nevents % 1000) == 0) {
			auto Nleft = hdevio->GetNWordsLeftInFile();
			auto Nread = Nwords_in_input_file - Nleft;
			int percent_done = (100 * Nread) / Nwords_in_input_file;
			int percent_saved = (100 * Nevents_saved ) / Nevents;
			cout << " " << Nread << "/" << Nwords_in_input_file << " (" << percent_done << "%) processed  - " << percent_saved << "% saved     \r";
			cout.flush();
		}
		if (Nevents > (SKIP_EVIO_EVENTS + MAX_EVIO_EVENTS)) break;
	}
	cout << endl;

	// Close EVIO files
	delete hdevio;
	evioout.ReturnBufferToPool(vbuff);
	evioout.Quit();
	thr.join();

	// Extract file number from file name (run number extracted from EVIO already)
	uint32_t file_number = 999;
	auto len = filename.length();
	if( len>8 ){
		auto filenumstr = filename.substr(len-8, 3);
		file_number = atoi(filenumstr.c_str());
	}

	// Get host name
	char host[256]="unknown";
	gethostname( host, 256) ;

	auto NFCAL_BCAL_TOTAL    = NGTP_TOTAL[ 0];
	auto NBCAL_TOTAL         = NGTP_TOTAL[ 2];
	auto NPS_TOTAL           = NGTP_TOTAL[ 3];
	auto NFCAL_BCAL_ST_TOTAL = NGTP_TOTAL[ 4];

	auto NFCAL_LED_TOTAL     = NFP_TOTAL[ 2];
	auto NCCAL_LED1_TOTAL    = NFP_TOTAL[ 4];
	auto NCCAL_LED2_TOTAL    = NFP_TOTAL[ 5];
	auto NBCAL_LED_US_TOTAL  = NFP_TOTAL[ 8];
	auto NBCAL_LED_DS_TOTAL  = NFP_TOTAL[ 9];
	auto Nrandom_TOTAL       = NFP_TOTAL[11];
	auto NDIRC_LED_TOTAL     = NFP_TOTAL[14];

	cout << "===== Summary for " << filename << ":" << endl;
	cout << "                 RUN: " << RUN_NUMBER << endl;
	cout << "                FILE: " << file_number << endl;
	cout << "    NFCAL_BCAL_TOTAL: " << NFCAL_BCAL_TOTAL << endl;
	cout << "         NBCAL_TOTAL: " << NBCAL_TOTAL << endl;
	cout << "           NPS_TOTAL: " << NPS_TOTAL << endl;
	cout << " NFCAL_BCAL_ST_TOTAL: " << NFCAL_BCAL_ST_TOTAL << endl;
	cout << endl;
	cout << "  NBCAL_LED_US_TOTAL: " << NBCAL_LED_US_TOTAL << endl;
	cout << "  NBCAL_LED_DS_TOTAL: " << NBCAL_LED_DS_TOTAL << endl;
	cout << "       Nrandom_TOTAL: " << Nrandom_TOTAL << endl;
	cout << "     NFCAL_LED_TOTAL: " << NFCAL_LED_TOTAL << endl;
	cout << "    NCCAL_LED1_TOTAL: " << NCCAL_LED1_TOTAL << endl;
	cout << "    NCCAL_LED2_TOTAL: " << NCCAL_LED2_TOTAL << endl;
	cout << "     NDIRC_LED_TOTAL: " << NDIRC_LED_TOTAL << endl;
	cout << endl;
	cout << "  NEVIO_BLOCKS_TOTAL: " << NEVIO_BLOCKS_TOTAL << endl;
	cout << "       NEVENTS_TOTAL: " << NEVENTS_TOTAL << endl;
	cout << "NCODA_PHYSICS_EVENTS: " << NEVENTS_BY_TYPE[kCODA_PHYSICS_EVENT] << endl;
	cout << "NCDAQ_PHYSICS_EVENTS: " << NEVENTS_BY_TYPE[kCDAQ_PHYSICS_EVENT] << endl;
	cout << "         NBOR_EVENTS: " << NEVENTS_BY_TYPE[kBOR_EVENT] << endl;
	cout << "       NEPICS_EVENTS: " << NEVENTS_BY_TYPE[kEPICS_EVENT] << endl;
	cout << "     NCONTROL_EVENTS: " << NEVENTS_BY_TYPE[kCONTROL_EVENT] << endl;

	// Form SQL commands to upsert this into the skiminfo table.
	stringstream ss;
	ss << "INSERT INTO skiminfo (run,file,num_physics_events,num_bor_events,num_epics_events,num_control_events,first_event,last_event,skim_host)";
	ss << " VALUES(" << RUN_NUMBER << "," << file_number;
	ss << "," << NEVENTS_BY_TYPE[kCODA_PHYSICS_EVENT]+NEVENTS_BY_TYPE[kCDAQ_PHYSICS_EVENT];
	ss << "," << NEVENTS_BY_TYPE[kBOR_EVENT];
	ss << "," << NEVENTS_BY_TYPE[kEPICS_EVENT];
	ss << "," << NEVENTS_BY_TYPE[kCONTROL_EVENT];
	ss << "," << FIRST_EVENT_NUMBER;
	ss << "," << LAST_EVENT_NUMBER;
	ss << ",'" << host << "'";
	ss << ")";
	ss << " ON DUPLICATE KEY UPDATE ";
	ss << " num_physics_events=VALUES(num_physics_events)";
	ss << " ,num_bor_events=VALUES(num_bor_events)";
	ss << " ,num_epics_events=VALUES(num_epics_events)";
	ss << " ,num_control_events=VALUES(num_control_events)";
	ss << " ,first_event=VALUES(first_event)";
	ss << " ,last_event=VALUES(last_event)";
	ss << " ,skim_host=VALUES(skim_host)";
	ss << ";" << endl;

	ss << "UPDATE skiminfo SET ";
	for(int itrig=0; itrig<16; itrig++){
		if( itrig!=0 ) ss <<  ",";
		ss << "NFP" << itrig << "=" << NFP_TOTAL[itrig] ;
		ss << ",NGTP" << itrig << "=" << NGTP_TOTAL[itrig] ;
	}
	ss << " WHERE run=" << RUN_NUMBER << " AND file=" << file_number << ";" << endl;

	// Write SQL to file
	if( WRITE_SQL_FILE ) {
		string sqlfilename = ofilename;
		pos = sqlfilename.find(".evio");
		sqlfilename.erase(pos);
		sqlfilename += ".sql";
		cout << "Writing SQL to: " << sqlfilename << endl;
		ofstream sqlfile(sqlfilename);
		sqlfile << ss.str();
		sqlfile.close();
	}
}

//------------------------------------------------------------------
// ProcessEvent
//
// Scan event in the given buffer and decide whether to write the
// event to the output file. Returns true if it should be written
// and false if it shouldn't.
//
// This will also accumulate statistics on how many events and
// triggers there are.
//------------------------------------------------------------------
bool ProcessEvent(uint32_t *buff, uint32_t buff_len) {
	EventType_t event_type;
	uint32_t Mevents;
	GetEventInfo(buff, buff_len, event_type, Mevents);

//    cout << "Processing buffer: " <<  "   type: " << event_type << "  M: " << Mevents << endl;

	if (event_type == kCODA_PHYSICS_EVENT) {
		vector <uint32_t> trig_masks;
		vector <uint32_t> fp_trig_masks;
		GetTrigMasks(buff, buff_len, Mevents, trig_masks, fp_trig_masks);

		uint32_t NFP[32];
		uint32_t NGTP[32];
		for ( int ibit = 0; ibit < 32; ibit++) NFP[ibit] = NGTP[ibit] = 0;

		for (auto fp_trig_mask : fp_trig_masks) {
			for (int ibit = 0; ibit < 32; ibit++) NFP[ibit] += ((fp_trig_mask >> ibit) & 0x01);
		}

		for (auto trig_mask : trig_masks) {
			for (int ibit = 0; ibit < 32; ibit++) NGTP[ibit] += ((trig_mask >> ibit) & 0x01);
		}

		NEVENTS_TOTAL += Mevents;

		for(int ibit = 0; ibit < 32; ibit++){
			NGTP_TOTAL[ibit] += NGTP[ibit];
			NFP_TOTAL[ibit]  += NFP[ibit];
		}

		auto NFP_TOT = 0;
		for(int itrig=0; itrig<32; itrig++){
			if( FP_IGNORE[itrig] ) continue;  // Allow user to specify certain FP triggers not to cause block to be written
			NFP_TOT += NFP[itrig];
		}
		//auto NFP = NBCAL_LED_US + NBCAL_LED_DS + Nrandom + NFCAL_LED + NCCAL_LED1 + NCCAL_LED2 + NDIRC_LED;
		if (NFP_TOT > 0) {
			NEVIO_BLOCKS_TOTAL++;
			return true; // Tell caller to write this event to file
		}
		return false; // Tell caller not to write this event to file
	}

	return true; // write out all non-physics events
}

//----------------
// GetEventType
//----------------
void GetEventInfo(uint32_t *buff, uint32_t buff_len, EventType_t &type, uint32_t &Mevents) {
	type = kUNKNOWN_EVENT;
	Mevents = 1;

	uint32_t event_head = buff[1];
	uint32_t tag = (event_head >> 16) & 0xFFFF;
	type = kUNKNOWN_EVENT;
	switch (tag) {
		case 0x0060:
			type = kEPICS_EVENT;
			break;
		case 0x0070:
			type = kBOR_EVENT;
			break;

		case 0xFFD0:
		case 0xFFD1:
		case 0xFFD2:
		case 0xFFD3:
		case 0xFFD4:
			type = kCONTROL_EVENT;
			break;

		case 0xFF58:
		case 0xFF78:
		case 0xFF50:
		case 0xFF70:
			type = kCODA_PHYSICS_EVENT;
			Mevents = event_head & 0xFF;
			break;

		case 0xFF32:
		case 0xFF33:
			type = kCDAQ_PHYSICS_EVENT;
			break;
	}

	NEVENTS_BY_TYPE[ type ] += Mevents;
}

//----------------
// GetFPmasks
//----------------
void GetTrigMasks(uint32_t *buff, uint32_t buff_len, uint32_t Mevents, vector <uint32_t> &trig_masks,
                  vector <uint32_t> &fp_trig_masks) {

	// sanity check:
	if (Mevents == 0) {
		stringstream ss;
		ss << "ERROR: GetFPmasks called with Mevents=0 ! " << endl;
		return;
	}

	uint32_t *iptr = buff;
	uint32_t physics_event_len = *iptr++;
	uint32_t *iend_physics_event = &iptr[physics_event_len];
	iptr++;

	// Built Trigger Bank
	//uint32_t built_trigger_bank_len  = *iptr;
	//uint32_t *iend_built_trigger_bank = &iptr[built_trigger_bank_len+1];

	iptr++; // advance past length word
	uint32_t mask = 0xFF202000;
	if (((*iptr) & mask) != mask) {
		stringstream ss;
		ss << "Bad header word in Built Trigger Bank: " << hex << *iptr;
		return;
	}

	uint32_t tag = (*iptr) >> 16; // 0xFF2X
	uint32_t Nrocs = (*iptr++) & 0xFF;

	//-------- Common data (64bit)
	uint32_t common_header64 = *iptr++;
	uint32_t common_header64_len = common_header64 & 0xFFFF;
	uint64_t *iptr64 = (uint64_t*)iptr;
	iptr = &iptr[common_header64_len];

	// First and last event numbers
	uint64_t first_event_num = *iptr64++;
	uint64_t last_event_num = first_event_num + (uint64_t)Mevents - 1;
	if( FIRST_EVENT_NUMBER==0 ) FIRST_EVENT_NUMBER = first_event_num;
	if( LAST_EVENT_NUMBER<last_event_num ) LAST_EVENT_NUMBER = last_event_num;

	// Hi and lo 32bit words in 64bit numbers seem to be
	// switched for events read from ET, but not read from
	// file. We only read from file here so never swap.
	// if(event_source->source_type==event_source->kETSource) first_event_num = (first_event_num>>32) | (first_event_num<<32);

	// Average timestamps
	//uint32_t Ntimestamps = (common_header64_len / 2) - 1;
	//if (tag & 0x2) Ntimestamps--; // subtract 1 for run number/type word if present
	//vector <uint64_t> avg_timestamps;
	//for(uint32_t i=0; i<Ntimestamps; i++) avg_timestamps.push_back(*iptr64++);

	// run number and run type
	//uint32_t run_type   = 0;
	if (tag & 0x02) {
		iptr64 = &iptr64[Mevents]; // iptr64 should be pointing to first timestamp before this line
		RUN_NUMBER = (*iptr64) >> 32;
		//run_type   = (*iptr64) & 0xFFFFFFFF;
		//iptr64++;
	}

	//-------- Common data (16bit)
	uint32_t common_header16 = *iptr++;
	uint32_t common_header16_len = common_header16 & 0xFFFF;
	uint16_t *iptr16 = (uint16_t *) iptr;
	iptr = &iptr[common_header16_len];

	vector <uint16_t> event_types;
	for (uint32_t i = 0; i < Mevents; i++) event_types.push_back(*iptr16++);

	//-------- ROC data (32bit)
	for (uint32_t iroc = 0; iroc < Nrocs; iroc++) {
		uint32_t common_header32 = *iptr++;
		uint32_t common_header32_len = common_header32 & 0xFFFF;
		uint32_t *iend_common_header = &iptr[common_header32_len];
		uint32_t rocid = common_header32 >> 24;

		if (rocid == 1) {
			uint32_t Nwords_per_event = common_header32_len / Mevents;
			for (uint32_t i = 0; i < Mevents; i++) {

				iptr++; // uint64_t ts_low
				iptr++; // uint64_t ts_high
				// uint64_t timestamp = (ts_high<<32) + ts_low;
				if (Nwords_per_event > 3) {
					uint32_t trig_mask = *iptr++;
					uint32_t fp_trig_mask = *iptr++;

					trig_masks.push_back(trig_mask);
					fp_trig_masks.push_back(fp_trig_mask);
				}

				if (iptr > iend_common_header) {
					cout << "ERROR: Bad data format in Built Trigger Bank!" << endl;
					return;
				}
			}
		}

		iptr = iend_common_header;
	}

	iptr = iend_physics_event;

	return;
}



