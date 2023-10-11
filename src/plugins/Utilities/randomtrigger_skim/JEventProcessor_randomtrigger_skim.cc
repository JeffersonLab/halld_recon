//
// JEventProcessor_randomtrigger_skim.cc
//
// JANA event processor plugin to skim random triggers to HDDM files
//
// Author: Sean Dobbs

#include "JEventProcessor_randomtrigger_skim.h"
#include "TRIGGER/DL1Trigger.h"
#include "BCAL/DBCALHit.h"
#include "DAQ/DL1Info.h"
#include <HDDM/DEventWriterHDDM.h>
#include <DAQ/DBeamCurrent.h>

// for initializing plugins
extern "C" {
   void InitPlugin(JApplication *app)
	{
		InitJANAPlugin(app);
		app->Add(new JEventProcessor_randomtrigger_skim());
   }
} // "extern C"

static bool use_beam_fiducial = true;


// variables to control which triggers get read out

//-------------------------------
// Init
//-------------------------------
void JEventProcessor_randomtrigger_skim::Init()
{
    auto app = GetApplication()	;
    int use_beam_fiducial_toggle = 1;

    app->SetDefaultParameter("RANDSKIM:USEBEAM",use_beam_fiducial , "Use beam fiducials");

    if(use_beam_fiducial_toggle == 0)
      use_beam_fiducial = false;
}

//-------------------------------
// BeginRun
//-------------------------------
void JEventProcessor_randomtrigger_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    dBeamCurrentFactory = new DBeamCurrent_factory();
    dBeamCurrentFactory->SetApplication(event->GetJApplication());
    dBeamCurrentFactory->Init();
    dBeamCurrentFactory->BeginRun(event);
    // TODO: NWB: This thing again.
}

//-------------------------------
// Process
//-------------------------------
void JEventProcessor_randomtrigger_skim::Process(const std::shared_ptr<const JEvent>& locEvent)
{
    // Get HDDM writer
    vector<const DEventWriterHDDM*> locEventWriterHDDMVector;
    locEvent->Get(locEventWriterHDDMVector);

    // beam current and fiducial definition
    vector<const DBeamCurrent*> beamCurrent;
    locEvent->Get(beamCurrent);

    //bool is_cosmic_trigger = false;
    bool is_random_trigger = false;

	const DL1Trigger *trig = NULL;
	try {
		locEvent->GetSingle(trig);
	} catch (...) {}

    // parse the triggers we want to save
	if (trig) {

		if (trig->fp_trig_mask & 0x800) {  // Trigger front-panel bit 11
			// Periodic pulser trigger fired
			is_random_trigger = true;
		}

	} 

    // make sure this is a random trigger event
    if(!is_random_trigger)
        return;
    
    if(use_beam_fiducial){
      // make sure we can perform a fiducial beam current cut
      if(beamCurrent.empty())
        return;
      
      // make sure the beam is on
      if(!beamCurrent[0]->is_fiducial)
        return;
    }
        

    // Save events to skim file
    locEventWriterHDDMVector[0]->Write_HDDMEvent(locEvent, "random"); 
}

//-------------------------------
// EndRun
//-------------------------------
void JEventProcessor_randomtrigger_skim::EndRun()
{
}

//-------------------------------
// Finish
//-------------------------------
void JEventProcessor_randomtrigger_skim::Finish()
{
}

