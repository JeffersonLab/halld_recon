#include "DEventWriterEVIO.h"
#include "DAQ/DL1Info.h"

#include <DAQ/JEventSource_EVIO.h>

static  JEventSource *currentEventSource = nullptr;


size_t& DEventWriterEVIO::Get_NumEVIOOutputThreads(void) const
{
	// must be read/used entirely in "EVIOWriter" lock
	static size_t locEVIONumOutputThreads = 0;
	return locEVIONumOutputThreads;
}

map<string, HDEVIOWriter*>& DEventWriterEVIO::Get_EVIOOutputters(void) const
{
	// must be read/used entirely in "EVIOWriter" lock
	static map<string, HDEVIOWriter*> locEVIOOutputters;
	return locEVIOOutputters;
}

map<string, DEVIOBufferWriter*>& DEventWriterEVIO::Get_EVIOBufferWriters(void) const
{
	// must be read/used entirely in "EVIOWriter" lock
	static map<string, DEVIOBufferWriter*> locEVIOBufferWriters;
	return locEVIOBufferWriters;
}

map<string, pthread_t>& DEventWriterEVIO::Get_EVIOOutputThreads(void) const
{
	// must be read/used entirely in "EVIOWriter" lock
	static map<string, pthread_t> locEVIOOutputThreads;
	return locEVIOOutputThreads;
}

DEventWriterEVIO::DEventWriterEVIO(const std::shared_ptr<const JEvent>& locEvent)
{
	auto app = locEvent->GetJApplication();
	lockService = app->GetService<JLockService>();

	COMPACT = true;
	PREFER_EMULATED = false;
	DEBUG_FILES = false; // n.b. also defined in HDEVIOWriter
    CLOSE_FILES = false;
    dMergeFiles = false;
    dMergedFilename = "merged";  

	ofs_debug_input = NULL;
	ofs_debug_output = NULL;

	app->SetDefaultParameter("EVIOOUT:COMPACT" , COMPACT,  "Drop words where we can to reduce output file size. This shouldn't loose any vital information, but can be turned off to help with debugging.");
	app->SetDefaultParameter("EVIOOUT:PREFER_EMULATED" , PREFER_EMULATED,  "If true, then sample data will not be written to output, but emulated hits will. Otherwise, do exactly the opposite.");
	app->SetDefaultParameter("EVIOOUT:DEBUG_FILES" , DEBUG_FILES,  "Write input and output debug files in addition to the standard output.");
	app->SetDefaultParameter("EVIOOUT:CLOSE_FILES" , CLOSE_FILES,  "Close output files once a new input file is opened (just make sure none of the input files have the same file name, or outputs will be overwritten!");
	app->SetDefaultParameter("EVIOOUT:MERGE" , dMergeFiles,  "Write only one output file for each sub-file name");
	app->SetDefaultParameter("EVIOOUT:MERGE_FILENAME" , dMergedFilename, "Base file name for merged files, only used if EVIOOUT:MERGE is enabled");

    //buffer_writer = new DEVIOBufferWriter(COMPACT, PREFER_EMULATED);

    // save a pointer to the translation table
    ttab = NULL;
    locEvent->GetSingle(ttab);

	// Initialize file-level variables
	lockService->WriteLock("EVIOWriter");
	{
		++Get_NumEVIOOutputThreads();
	}
	lockService->Unlock("EVIOWriter");

	if(DEBUG_FILES){
		ofs_debug_input = new ofstream("hdevio_debug_input.evio");
		if( !ofs_debug_input->is_open() ){
			jerr << "Unable to open \"hdevio_debug_input.evio\"!" << endl;
			delete ofs_debug_input;
			ofs_debug_input = NULL;
		}else{
			jout << "Opened \"hdevio_debug_input.evio\" for debug output" << endl;
		}

		ofs_debug_output = new ofstream("hdevio_debug_output_preswap.evio");
		if( !ofs_debug_output->is_open() ){
			jerr << "Unable to open \"hdevio_debug_output_preswap.evio\"!" << endl;
			delete ofs_debug_output;
			ofs_debug_output = NULL;
		}else{
			jout << "Opened \"hdevio_debug_output_preswap.evio\" for debug output" << endl;
		}
	}	
}


void DEventWriterEVIO::SetDetectorsToWriteOut(const std::shared_ptr<const JEvent>& locEvent, string detector_list, string locOutputFileNameSubString) const
{
    // Allow users to set only some detectors to be written out
    // The list of detectors is set on a per-file basis
    // and the list is the same as given in DTranslationTable

    if(ttab == NULL) {
        jerr << "Tried to set values in DEventWriterEVIO::SetDetectorsToWriteOut() but translation table not loaded!" << endl;
        return;
    }

	string locOutputFileName = Get_OutputFileName(locEvent, locOutputFileNameSubString);

    // sanity check
	lockService->WriteLock("EVIOWriter");
    if(Get_EVIOBufferWriters().find(locOutputFileName) == Get_EVIOBufferWriters().end()) {
        lockService->Unlock("EVIOWriter");
        // file must not have been created?
        return;
    }
	lockService->Unlock("EVIOWriter");

    
    // create new roc output list
    set<uint32_t> rocs_to_write_out;

    // if given a blank list, assume we should write everything out
    if(detector_list == "") {
        lockService->WriteLock("EVIOWriter");
        Get_EVIOBufferWriters()[locOutputFileName]->SetROCsToWriteOut(rocs_to_write_out);
        lockService->Unlock("EVIOWriter");
        return;
    }

    // set up some information
    map<string, DTranslationTable::Detector_t> name_to_id;
    for(uint32_t dettype=DTranslationTable::UNKNOWN_DETECTOR; dettype<DTranslationTable::NUM_DETECTOR_TYPES; dettype++){
        name_to_id[ttab->DetectorName((DTranslationTable::Detector_t)dettype)] = (DTranslationTable::Detector_t)dettype;
    }

    // Parse string of system names
    std::istringstream ss(detector_list);
    std::string token;
    while(std::getline(ss, token, ',')) {
                
        // Get initial list of rocids based on token
        set<uint32_t> rocids = ttab->Get_ROCID_By_System()[name_to_id[token]];
                
        // Let "FDC" be an alias for both cathode strips and wires
        if(token == "FDC"){
            set<uint32_t> rocids1 = ttab->Get_ROCID_By_System()[name_to_id["FDC_CATHODES"]];
            set<uint32_t> rocids2 = ttab->Get_ROCID_By_System()[name_to_id["FDC_WIRES"]];
            rocids.insert(rocids1.begin(), rocids1.end());
            rocids.insert(rocids2.begin(), rocids2.end());
        }

        // More likely than not, someone specifying "PS" will also want "PSC" 
        if(token == "PS"){
            set<uint32_t> rocids1 = ttab->Get_ROCID_By_System()[name_to_id["PSC"]];
            rocids.insert(rocids1.begin(), rocids1.end());
        }

        // Finally, add the ROCs to the list
        rocs_to_write_out.insert( rocids.begin(), rocids.end() );
    }

    // save results
	lockService->WriteLock("EVIOWriter");
    Get_EVIOBufferWriters()[locOutputFileName]->SetROCsToWriteOut(rocs_to_write_out);
	lockService->Unlock("EVIOWriter");
}


bool DEventWriterEVIO::Write_EVIOEvent(const std::shared_ptr<const JEvent>& locEvent, string locOutputFileNameSubString) const
{
    vector<const JObject *> locObjectsToSaveNull;
    return Write_EVIOEvent(locEvent, locOutputFileNameSubString, locObjectsToSaveNull);
}


bool DEventWriterEVIO::Write_EVIOEvent(const std::shared_ptr<const JEvent>& locEvent, string locOutputFileNameSubString,
                                       vector<const JObject *> &locObjectsToSave) const
{
	// Get pointer to JEventSource and make sure it is the right type
	JEventSource* locEventSource = locEvent->GetJEventSource();
	if(locEventSource == NULL)
		return false;
	
	// Optionally write input buffer to a debug file
    // Note that this only works with JEventSource_EVIO (old-style parser)
    // since JEventSource_EVIOpp (new parser) doesn't have access to the 
    // raw EVIO buffer
	if(DEBUG_FILES){
		JEventSource_EVIO *jesevio = dynamic_cast<JEventSource_EVIO*>(locEventSource);
		if(!jesevio){
			static bool warned = false;
			if(!warned) jerr << "Event source not a JEventSource_EVIO type!" << endl;
			warned = true;
		}else{
			uint32_t *buff;
			uint32_t buff_size;
			// TODO: NWB: GetEVIOBuffer should accept const std::shared_ptr<const JEvent>& because
			//       Once that happens I can kill the const cast hopefully
			jesevio->GetEVIOBuffer(const_cast<JEvent&>(*locEvent), buff, buff_size);
			if(ofs_debug_input) ofs_debug_input->write((char*)buff, buff_size*sizeof(uint32_t));
		}
	}

	string locOutputFileName = Get_OutputFileName(locEvent, locOutputFileNameSubString);
	lockService->WriteLock("EVIOWriter");
	{
		//check to see if the EVIO file is open
		if(Get_EVIOOutputters().find(locOutputFileName) == Get_EVIOOutputters().end()) {
			//not open, open it
		  if(!Open_OutputFile(locEventLoop, locOutputFileName)){
		    lockService->Unlock("EVIOWriter"); // release lock
		    return false; //failed to open
		  }
		}

		//open: get handle, write event
		HDEVIOWriter *locEVIOWriter = Get_EVIOOutputters()[locOutputFileName];
        DEVIOBufferWriter *locBufferWriter = Get_EVIOBufferWriters()[locOutputFileName];
		// Write event into buffer
		vector<uint32_t> *buff = locEVIOWriter->GetBufferFromPool();
        if(locObjectsToSave.size() == 0)
            locBufferWriter->WriteEventToBuffer(locEvent, *buff);
        else
            locBufferWriter->WriteEventToBuffer(locEvent, *buff, locObjectsToSave);

		// Optionally write buffer to output file
		if(ofs_debug_output) ofs_debug_output->write((const char*)&(*buff)[0], buff->size()*sizeof(uint32_t));
	
		// Add event to output queue
		locEVIOWriter->AddBufferToOutput(buff);
	}
	lockService->Unlock("EVIOWriter");

    return true;
}

bool DEventWriterEVIO::Write_EVIOBuffer(const std::shared_ptr<const JEvent>& locEvent, uint32_t *locOutputBuffer, uint32_t locOutputBufferSize, string locOutputFileNameSubString) const
{
    vector<uint32_t> *locOutputBufferVec = new vector<uint32_t>();
    locOutputBufferVec->reserve(locOutputBufferSize);
    // buffer size is passed to us as number of bytes, but we are copying words, so do the unit conversion
    double locOutputBufferWords = locOutputBufferSize/4;
    
    //cout << "event" << endl;

    // build ouptut buffer from C-style array
    for(uint32_t i=0; i<locOutputBufferWords; i++) {
        //if(i<8)
        //  cout << "0x" << hex << locOutputBuffer[i] << endl;
        locOutputBufferVec->push_back(locOutputBuffer[i]);
    }

    //cout << "Write_EVIOBuffer() locOutputBufferSize = " << locOutputBufferSize << "  vector size = " << locOutputBufferVec->size() << endl;

    return Write_EVIOBuffer(locEvent, locOutputBufferVec, locOutputFileNameSubString);
}


bool DEventWriterEVIO::Write_EVIOBuffer(const std::shared_ptr<const JEvent>& locEvent, vector<uint32_t> *locOutputBuffer, string locOutputFileNameSubString) const
{
    // write out raw EVIO buffer

	// Get pointer to JEventSource and make sure it is the right type
	JEventSource* locEventSource = locEvent->GetJEventSource();
	if(locEventSource == NULL)
		return false;

	JEventSource_EVIO* locEvioSource = dynamic_cast<JEventSource_EVIO*>(locEventSource);
	if(locEvioSource == NULL) {
		jerr << "WARNING!!! You MUST use this only with EVIO formatted data!!!" << endl;
		return false;
	}

	string locOutputFileName = Get_OutputFileName(locEvent, locOutputFileNameSubString);
	lockService->WriteLock("EVIOWriter");
	{
		//check to see if the EVIO file is open
		if(Get_EVIOOutputters().find(locOutputFileName) == Get_EVIOOutputters().end())  {
			//not open, open it
			if(!Open_OutputFile(locEvent, locOutputFileName)){
				jerr << "Unable to open EVIO file \""<< locOutputFileName << "\" for writing!" << endl;
				japp->Quit();
				lockService->Unlock("EVIOWriter");
				return false; //failed to open
			}
		}

		//open: get handle, write event
		HDEVIOWriter *locEVIOWriter = Get_EVIOOutputters()[locOutputFileName];
		// Add event to output queue
		locEVIOWriter->AddBufferToOutput(locOutputBuffer);
	}
	lockService->Unlock("EVIOWriter");

    return true;
}

string DEventWriterEVIO::Get_OutputFileName(const std::shared_ptr<const JEvent>& locEvent, string locOutputFileNameSubString) const
{
    // if we're merging input files, write everything to the specified file
    if(dMergeFiles) {
        return (dMergedFilename + string(".") + locOutputFileNameSubString + string(".evio"));
    }

	//get the event source
	JEventSource* locEventSource = locEvent->GetJEventSource();
	if(locEventSource == NULL)
		return "no_name.evio";

	//get the source file name (strip the path)
	string locSourceFileName = locEventSource->GetResourceName();
	size_t locSlashIndex = locSourceFileName.find_last_of("/");
	string locSourceFileName_Pathless = (locSlashIndex != string::npos) ? locSourceFileName.substr(locSlashIndex + 1) : locSourceFileName;

	//strip the file extension (if present and if is a known format: .root, .evio, or .hddm)
	size_t locDotIndex = locSourceFileName_Pathless.find_last_of(".");
	if(locDotIndex != string::npos)
	{
		string locSuffix = locSourceFileName_Pathless.substr(locDotIndex + 1);
		if((locSuffix == "root") || (locSuffix == "evio") || (locSuffix == "hddm"))
			locSourceFileName_Pathless = locSourceFileName_Pathless.substr(0, locDotIndex);
	}

	return (locSourceFileName_Pathless + string(".") + locOutputFileNameSubString + string(".evio"));
}

bool DEventWriterEVIO::Open_OutputFile(const std::shared_ptr<const JEvent>& locEvent, string locOutputFileName) const
{
	//ASSUMES A LOCK HAS ALREADY BEEN ACQUIRED (by WriteEVIOEvent)
	// and assume that it doesn't exist


    // only close additional files if requested
    if(CLOSE_FILES) {
        //get the event source
        JEventSource* locEventSource = locEvent->GetJEventSource();

        // close all the files when we notice that the input file has changed
        // it's a bit of a hammer, but should work since we only call it once
        if(currentEventSource != locEventSource) {
            currentEventSource = locEventSource;

            // first clear all of the objects being referenced
            for( auto entry : Get_EVIOOutputters() )
                delete entry.second;
            for( auto entry : Get_EVIOBufferWriters() )
                delete entry.second;
            // and close the threads so that they don't use extra CPU with their idle spin-locks
            for( auto entry : Get_EVIOOutputThreads() )
                pthread_cancel(entry.second);

            // now clear all of the entries in these maps
            Get_EVIOOutputters().clear();
            Get_EVIOBufferWriters().clear();
            Get_EVIOOutputThreads().clear();
        }

    }

	// Create object to write the selected events to a file or ET system
	// Run each connection in their own thread
	HDEVIOWriter *locEVIOout = new HDEVIOWriter(locOutputFileName, japp);
	// TODO: NWB: Using japp here because climate change will kill us all anyway
    DEVIOBufferWriter *locEVIOwriter = new DEVIOBufferWriter(COMPACT, PREFER_EMULATED);
	pthread_t locEVIOout_thr;
	int result = pthread_create(&locEVIOout_thr, NULL, HDEVIOOutputThread, locEVIOout);
	bool success = (result == 0);


	//evaluate status
	if(!success)
		jerr << "Unable to open EVIO file:  error code = " << result << endl;
	else
	{
		jout << "Output EVIO file " << locOutputFileName << " created." << endl;
		Get_EVIOOutputters()[locOutputFileName] = locEVIOout; //store the handle
		Get_EVIOOutputThreads()[locOutputFileName] = locEVIOout_thr; //store the thread
        Get_EVIOBufferWriters()[locOutputFileName] = locEVIOwriter; //store the buffer creator
	}

	return success;
}

DEventWriterEVIO::~DEventWriterEVIO(void)
{
	lockService->WriteLock("EVIOWriter");
	{
		--Get_NumEVIOOutputThreads();
		if(Get_NumEVIOOutputThreads() > 0)
		{
			lockService->Unlock("EVIOWriter");
			return; //not the last thread writing to EVIO files
		}

		//last thread writing to EVIO files: close all files and free all memory
		map<string, HDEVIOWriter *>::iterator locIterator = Get_EVIOOutputters().begin();
		for(; locIterator != Get_EVIOOutputters().end(); ++locIterator)
		{
			string locOutputFileName = locIterator->first;
			HDEVIOWriter *locEVIOOutputter = locIterator->second;
			pthread_t locEVIOOutputThread = Get_EVIOOutputThreads()[locOutputFileName];
			
			// Finishsh writing out to the event source
			locEVIOOutputter->Quit();
			// clean up the output thread
			void *retval=NULL;
			int result = pthread_join(locEVIOOutputThread, &retval);
            if(result!=0)
                jerr << "Problem closing EVIO file:  error code = " << result << endl;
			delete locEVIOOutputter;
			std::cout << "Closed EVIO file " << locOutputFileName << std::endl;
		}
		Get_EVIOOutputters().clear();
		Get_EVIOOutputThreads().clear();
	}
	lockService->Unlock("EVIOWriter");
}

