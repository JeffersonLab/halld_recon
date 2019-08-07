
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
using namespace std;

#include <TFile.h>

#include "HDEVIOWriter.h"
#undef _DBG_
#undef _DBG__

#include <DAQ/HDEVIO.h>


enum EventType_t{
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
bool ProcessEvent( uint32_t *buff, uint32_t buff_len);
void GetEventInfo( uint32_t *buff, uint32_t buff_len, EventType_t &type, uint32_t &Mevents );
void GetTrigMasks( uint32_t *buff, uint32_t buff_len, uint32_t Mevents, vector<uint32_t> &trig_masks, vector<uint32_t> &fp_trig_masks );


vector<string> filenames;
string USER_OFILENAME="";
uint64_t MAX_EVIO_EVENTS = 50000;
uint64_t SKIP_EVIO_EVENTS = 0;
uint64_t Nevents = 0;

uint32_t NFCAL_BCAL_TOTAL = 0;
uint32_t NBCAL_TOTAL = 0;
uint32_t NPS_TOTAL = 0;
uint32_t NFCAL_BCAL_ST_TOTAL = 0;

uint32_t NBCAL_LED_US_TOTAL = 0;
uint32_t NBCAL_LED_DS_TOTAL = 0;
uint32_t Nrandom_TOTAL = 0;
uint32_t NFCAL_LED_TOTAL = 0;
uint32_t NCCAL_LED1_TOTAL = 0;
uint32_t NCCAL_LED2_TOTAL = 0;
uint32_t NDIRC_LED_TOTAL = 0;

uint32_t NEVIO_BLOCKS_TOTAL = 0;
uint32_t NEVENTS_TOTAL = 0;


//----------------
// main
//----------------
int main(int narg, char *argv[])
{

	ParseCommandLineArguments(narg, argv);

    // Loop over input files
    for( auto fname : filenames ) MakeSkim( fname );

	return 0;
}

//----------------
// Usage
//----------------
void Usage(string mess="")
{
	cout << endl;
	cout << "Usage:" << endl;
	cout << endl;
	cout <<"    hdskims [options] file.evio [file2.evio ...]" << endl;
	cout << endl;
	cout << "options:" << endl;
	cout << "   -h, --help        Print this usage statement" << endl;
	cout << "   -m max_events     Max. EVIO events (not physics events) to process." << endl;
	cout << "   -i ignore_events  Num. EVIO events (not physics events) to ignore at start." << endl;
	cout << "   -o outfilename    Output filename (only use with single input file!)." << endl;
	cout << endl;
	cout << "n.b. When using the -i (ignore) flag, the total number of events" << endl;
	cout << "     read in will be the sum of how many are ignored and the \"max\"" << endl;
	cout << "     events with only the last max_events being processed." << endl;
	cout << endl;

	if(mess != "") cout << endl << mess << endl << endl;
	
	exit(0);
}

//----------------
// ParseCommandLineArguments
//----------------
void ParseCommandLineArguments(int narg, char *argv[])
{

	if(narg<2) Usage("You must supply a filename!");

	for(int i=1; i<narg; i++){
		string arg  = argv[i];
		string next = (i+1)<narg ? argv[i+1]:"";
		
		if(arg == "-h" || arg == "--help") Usage();
		else if(arg == "-i"){ SKIP_EVIO_EVENTS = atoi(next.c_str()); i++;}
		else if(arg == "-m"){ MAX_EVIO_EVENTS = atoi(next.c_str()); i++;}
		else if(arg == "-o"){ USER_OFILENAME = next.c_str(); i++;}
		else if(arg[0] == '-') {cout << "Unknown option \""<<arg<<"\" !" << endl; exit(-1);}
		else filenames.push_back(arg);
	}
	
	if( (filenames.size()>1) && (USER_OFILENAME.length()>0) ){
		Usage("ERROR: You may only use the -o option with a single input file! (otherwise output skim files will overwrite one another)");
	}
}


//----------------
// MakeSkim
//----------------
void MakeSkim( string &filename )
{
    string ofilename = filename;
    auto pos = ofilename.find(".evio");
    ofilename.erase( pos );
    ofilename += "_skims.evio";
	 pos = ofilename.find_last_of('/');
	 if( pos != string::npos ) ofilename.erase(0, pos);
	 
	 if( USER_OFILENAME.length()>0 ) ofilename = USER_OFILENAME;

    // Open EVIO input file
    cout << "Processing file: "  << filename << " -> " << ofilename << endl;
    HDEVIO *hdevio = new HDEVIO(filename);
    if(!hdevio->is_open){
        cout << hdevio->err_mess.str() << endl;
        return;
    }
    auto Nwords_in_input_file = hdevio->GetNWordsLeftInFile();

    // Open EVIO output file
	HDEVIOWriter evioout( ofilename );
	std::thread thr(HDEVIOOutputThread, &evioout);

    // Read all events in file
	vector<uint32_t> *vbuff = evioout.GetBufferFromPool();
    bool done = false;
    while(!done){

	    uint32_t *buff    = vbuff->data();
	    uint32_t buff_len = vbuff->capacity();
	    vbuff->resize( buff_len ); // we do this here so when we resize below, it does not re-initialize the contents
	    hdevio->readNoFileBuff(buff, buff_len);

        switch(hdevio->err_code){
            case HDEVIO::HDEVIO_OK:
                if( Nevents>= SKIP_EVIO_EVENTS ) {
                	if( ProcessEvent( buff, buff_len ) ){
                		vbuff->resize( (*vbuff)[0] + 1);
		                evioout.AddBufferToOutput(vbuff); // HDEVIOWriter takes ownership of buffer
		                vbuff = evioout.GetBufferFromPool(); // get a new buffer
                	}
                }
                break;
            case HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL:
            	vbuff->reserve( hdevio->last_event_len );
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

        if((++Nevents % 1000) == 0) {
        	auto Nleft = hdevio->GetNWordsLeftInFile();
        	auto Nread = Nwords_in_input_file - Nleft;
            int percent_done = (100*Nread)/Nwords_in_input_file;
            cout << " " << Nread << "/" << Nwords_in_input_file << " (" << percent_done << "%) processed       \r"; cout.flush();
        }
        if( Nevents > (SKIP_EVIO_EVENTS+MAX_EVIO_EVENTS) ) break;
    }
    cout << endl;

    // Close EVIO files
    delete hdevio;
	evioout.ReturnBufferToPool(vbuff);
	evioout.Quit();
	thr.join();

	cout << "===== Summary for " << filename << ":" << endl;
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
bool ProcessEvent( uint32_t *buff, uint32_t buff_len)
{
    EventType_t event_type;
    uint32_t Mevents;
    GetEventInfo( buff, buff_len, event_type, Mevents );

//    cout << "Processing buffer: " <<  "   type: " << event_type << "  M: " << Mevents << endl;

    if( event_type == kCODA_PHYSICS_EVENT ){
        vector<uint32_t> trig_masks;
        vector<uint32_t> fp_trig_masks;
        GetTrigMasks(buff, buff_len, Mevents, trig_masks, fp_trig_masks);

        uint32_t NFCAL_BCAL = 0;
        uint32_t NBCAL = 0;
        uint32_t NPS = 0;
        uint32_t NFCAL_BCAL_ST = 0;
 
        for( auto trig_mask : trig_masks ) {
            if (trig_mask & 0x001) NFCAL_BCAL++;
            if (trig_mask & 0x004) NBCAL++;
            if (trig_mask & 0x008) NPS++;
            if (trig_mask & 0x010) NFCAL_BCAL_ST++;
        }

        NFCAL_BCAL_TOTAL += NFCAL_BCAL;
        NBCAL_TOTAL += NBCAL;
        NPS_TOTAL += NPS;
        NFCAL_BCAL_ST_TOTAL += NFCAL_BCAL_ST;
        
        uint32_t NBCAL_LED_US = 0;
        uint32_t NBCAL_LED_DS = 0;
        uint32_t Nrandom = 0;
        uint32_t NFCAL_LED = 0;
        uint32_t NCCAL_LED1 = 0;
        uint32_t NCCAL_LED2 = 0;
        uint32_t NDIRC_LED = 0;
        
        for( auto fp_trig_mask : fp_trig_masks ) {
            if (fp_trig_mask & 0x100) NBCAL_LED_US++;
            if (fp_trig_mask & 0x200) NBCAL_LED_DS++;
            if (fp_trig_mask & 0x800) Nrandom++;
            if (fp_trig_mask & 0x004) NFCAL_LED++;
            if (fp_trig_mask & 0x010) NCCAL_LED1++;
            if (fp_trig_mask & 0x020) NCCAL_LED2++;
            if (fp_trig_mask & 0x4000) NDIRC_LED++;
        }

	    NEVENTS_TOTAL += Mevents;
        NBCAL_LED_US_TOTAL += NBCAL_LED_US;
        NBCAL_LED_DS_TOTAL += NBCAL_LED_DS;
        Nrandom_TOTAL += Nrandom;
        NFCAL_LED_TOTAL += NFCAL_LED;
        NCCAL_LED1_TOTAL += NCCAL_LED1;
        NCCAL_LED2_TOTAL += NCCAL_LED2;
        NDIRC_LED_TOTAL += NDIRC_LED;

        auto NFP = NBCAL_LED_US + NBCAL_LED_DS + Nrandom + NFCAL_LED + NCCAL_LED1 + NCCAL_LED2 + NDIRC_LED;
        if( NFP > 0 ) {
            NEVIO_BLOCKS_TOTAL++;
            return true; // Tell caller to write this event to file
        }
		return false; // Tell caller not to write this event to file
//
//        cout << "      trigs: ";
//        for( auto t: trig_masks ){
//            cout << "[";
//            for(int i=0; i<16; i++ ) if( (t>>i) & 0x1 ) cout << i+1 << "+";
//            cout << "],";
//        }
//        cout << endl;
//        cout << "   fp trigs: ";
//        for( auto t: fp_trig_masks ){
//            cout << "[";
//            for(int i=0; i<16; i++ ) if( (t>>i) & 0x1 ) cout << i+1 << "+";
//            cout << "],";
//        }
//        cout << endl;
    }

    return true; // write out all non-physics events
}

//----------------
// GetEventType
//----------------
void GetEventInfo( uint32_t *buff, uint32_t buff_len, EventType_t &type, uint32_t &Mevents )
{
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
}

//----------------
// GetFPmasks
//----------------
void GetTrigMasks( uint32_t *buff, uint32_t buff_len, uint32_t Mevents, vector<uint32_t> &trig_masks, vector<uint32_t> &fp_trig_masks )
{

    // sanity check:
    if(Mevents == 0) {
        stringstream ss;
        ss << "ERROR: GetFPmasks called with Mevents=0 ! "<<endl;
        return;
    }

    uint32_t *iptr = buff;
    uint32_t physics_event_len      = *iptr++;
    uint32_t *iend_physics_event    = &iptr[physics_event_len];
    iptr++;

    // Built Trigger Bank
    //uint32_t built_trigger_bank_len  = *iptr;
    //uint32_t *iend_built_trigger_bank = &iptr[built_trigger_bank_len+1];

    iptr++; // advance past length word
    uint32_t mask = 0xFF202000;
    if( ((*iptr) & mask) != mask ){
        stringstream ss;
        ss << "Bad header word in Built Trigger Bank: " << hex << *iptr;
        return;
    }

    uint32_t tag     = (*iptr)>>16; // 0xFF2X
    uint32_t Nrocs   = (*iptr++) & 0xFF;

    //-------- Common data (64bit)
    uint32_t common_header64 = *iptr++;
    uint32_t common_header64_len = common_header64 & 0xFFFF;
    //uint64_t *iptr64 = (uint64_t*)iptr;
    iptr = &iptr[common_header64_len];

    // First event number
    //uint64_t first_event_num = *iptr64++;

    // Hi and lo 32bit words in 64bit numbers seem to be
    // switched for events read from ET, but not read from
    // file. We only read from file here so never swap.
    // if(event_source->source_type==event_source->kETSource) first_event_num = (first_event_num>>32) | (first_event_num<<32);

    // Average timestamps
    uint32_t Ntimestamps = (common_header64_len/2)-1;
    if(tag & 0x2) Ntimestamps--; // subtract 1 for run number/type word if present
    vector<uint64_t> avg_timestamps;
    //for(uint32_t i=0; i<Ntimestamps; i++) avg_timestamps.push_back(*iptr64++);

    // run number and run type
    //uint32_t run_number = 0;
    //uint32_t run_type   = 0;
    if(tag & 0x02){
        //run_number = (*iptr64) >> 32;
        //run_type   = (*iptr64) & 0xFFFFFFFF;
        //iptr64++;
    }

    //-------- Common data (16bit)
    uint32_t common_header16 = *iptr++;
    uint32_t common_header16_len = common_header16 & 0xFFFF;
    uint16_t *iptr16 = (uint16_t*)iptr;
    iptr = &iptr[common_header16_len];

    vector<uint16_t> event_types;
    for(uint32_t i=0; i<Mevents; i++) event_types.push_back(*iptr16++);

    //-------- ROC data (32bit)
    for(uint32_t iroc=0; iroc<Nrocs; iroc++){
        uint32_t common_header32 = *iptr++;
        uint32_t common_header32_len = common_header32 & 0xFFFF;
        uint32_t *iend_common_header = &iptr[common_header32_len];
        uint32_t rocid = common_header32 >> 24;

        if( rocid == 1 ){
            uint32_t Nwords_per_event = common_header32_len/Mevents;
            for(uint32_t i=0; i<Mevents; i++){

                iptr++; // uint64_t ts_low
                iptr++; // uint64_t ts_high
                // uint64_t timestamp = (ts_high<<32) + ts_low;
                if( Nwords_per_event>3){
                    uint32_t trig_mask = *iptr++;
                    uint32_t fp_trig_mask = *iptr++;

                    trig_masks.push_back(trig_mask);
                    fp_trig_masks.push_back(fp_trig_mask);
                }

                if(iptr > iend_common_header){
                    cout << "ERROR: Bad data format in Built Trigger Bank!" << endl;
                    return;
                }
            }
        }

        iptr = iend_common_header ;
    }

    iptr = iend_physics_event;

    return;
}



