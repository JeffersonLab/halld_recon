// $Id$
//
//    File: JEventProcessor_merge_rawevents.cc
//

#include <math.h>

#include "JEventProcessor_merge_rawevents.h"

// Routine used to create our JEventProcessor
#include "GlueX.h"
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include "DAQ/JEventSource_EVIO.h"

extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_merge_rawevents());
  }
} // "C"

static bool WRITE_RAW_EVENTS = false;

//------------------
// JEventProcessor_merge_rawevents (Constructor)
//------------------
JEventProcessor_merge_rawevents::JEventProcessor_merge_rawevents() : dEventWriterEVIO(NULL)
{
	SetTypeName("JEventProcessor_merge_rawevents");
}

//------------------
// ~JEventProcessor_merge_rawevents (Destructor)
//------------------
JEventProcessor_merge_rawevents::~JEventProcessor_merge_rawevents()
{
}

//------------------
// Init
//------------------
void JEventProcessor_merge_rawevents::Init()
{
	auto app = GetApplication();
	app->SetDefaultParameter( "MERGERAWEVENTS:WRITE_RAW_EVENTS", WRITE_RAW_EVENTS );
}

//------------------
// BeginRun
//------------------
void JEventProcessor_merge_rawevents::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // create a new EVIO writer if it doesn't exist
    if(dEventWriterEVIO == NULL) {
        dEventWriterEVIO = new DEventWriterEVIO(event);
        dEventWriterEVIO->Set_MergeFiles(true);
    }
}

//------------------
// Process
//------------------
void JEventProcessor_merge_rawevents::Process(const std::shared_ptr<const JEvent>& event)
{

    if(WRITE_RAW_EVENTS) {
        // get EVIO information associated with the event
        void* the_event_ref = (void*) event->GetSingle<JEventSource_EVIO::ObjList>();
        uint32_t* output_buffer = JEventSource_EVIO::GetEVIOBufferFromRef(the_event_ref);
        uint32_t  output_buffer_size = JEventSource_EVIO::GetEVIOBufferSizeFromRef(the_event_ref);
        
        cout << "Writing out event " << event->GetEventNumber() << " buffer size = " << (output_buffer_size/4) << " words"  << endl;
    
        // write the buffer out
        // WARNING: this will work for non-entangled events, but hasn't been tested for entagled EVIO events
        dEventWriterEVIO->Write_EVIOBuffer( event, output_buffer, output_buffer_size, "merged" );
    } else {
        dEventWriterEVIO->Write_EVIOEvent( event, "merged" );
    }
}

//------------------
// EndRun
//------------------
void JEventProcessor_merge_rawevents::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Fin
//------------------
void JEventProcessor_merge_rawevents::Finish()
{
    // Called before program exit after event processing is finished.
    delete dEventWriterEVIO;
}


