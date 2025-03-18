// $Id$
//
//    File: JEventProcessor_cdc_goodtrack_skim.cc
//

#include <math.h>
#include <TLorentzVector.h>
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include "JEventProcessor_cdc_goodtrack_skim.h"


#include "TRACKING/DMCThrown.h"
// Routine used to create our JEventProcessor
#include "PID/DVertex.h"
#include "DANA/DApplication.h"
#include "JANA/JApplication.h"
#include "JANA/JFactory.h"
#include "BCAL/DBCALShower.h"
#include "RF/DRFTime.h"
#include "PID/DEventRFBunch.h"
#include "HDDM/DEventWriterHDDM.h"

#include "DLorentzVector.h"
#include "TTree.h"
#include "units.h"
#include "ANALYSIS/DAnalysisUtilities.h"

extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_cdc_goodtrack_skim());
  }
} // "C"


//------------------
// JEventProcessor_cdc_goodtrack_skim (Constructor)
//------------------
JEventProcessor_cdc_goodtrack_skim::JEventProcessor_cdc_goodtrack_skim()
{
    SetTypeName("JEventProcessor_cdc_goodtrack_skim");
}

//------------------
// ~JEventProcessor_cdc_goodtrack_skim (Destructor)
//------------------
JEventProcessor_cdc_goodtrack_skim::~JEventProcessor_cdc_goodtrack_skim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_cdc_goodtrack_skim::Init()
{  
  num_epics_events = 0;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_cdc_goodtrack_skim::BeginRun(const std::shared_ptr<const JEvent> &event)
{
}

//------------------
// Process
//------------------
void JEventProcessor_cdc_goodtrack_skim::Process(const std::shared_ptr<const JEvent> &event)
{
    const double MIN_FOM = 0.01;

    const DEventWriterEVIO* locEventWriterEVIO = NULL;
    event->GetSingle(locEventWriterEVIO);

	// always write out BOR events
	if(DEvent::GetStatusBit(event, kSTATUS_BOR_EVENT)) {
	  //jout << "Found BOR!" << endl;
	  locEventWriterEVIO->Write_EVIOEvent( event, "cdc_goodtrack_skim" );
	  return; // NOERROR;
	}
	
	// write out the first few EPICS events to save run number & other meta info
	if(DEvent::GetStatusBit(event, kSTATUS_EPICS_EVENT) && (num_epics_events<5)) {
	  //jout << "Found EPICS!" << endl;
	  locEventWriterEVIO->Write_EVIOEvent( event, "cdc_goodtrack_skim" );
	  num_epics_events++;
	  return; // NOERROR;
	}

	vector<const DTrackTimeBased *> tracks;
	event->Get(tracks);

	if(tracks.size() == 0)
		return; // NOERROR;


	// only save events with a "good" track
	// we define this by tracks with a minimum tracking FOM cut and
	//  a minimum number of hits on the track
	bool save_event = false;
	for( auto track : tracks ) {
	
		// check FOM
		if(!track->IsSmoothed) continue;  // ignore tracks that weren't fully Kalman filtered
		if(track->FOM < MIN_FOM) continue;
		
		// count number of hits on the track, require at least 4
		vector<DTrackFitter::pull_t> pulls = track->pulls;
		for (uint32_t j=0, count=0; j<pulls.size(); j++) {
		  if (pulls[j].cdc_hit == NULL) continue;
		  count++;

		  if(count > 4) {
		  	save_event=true;
		  	break;
		  }
		}
		
		if(save_event)
			break;
	}

	if(save_event)
		locEventWriterEVIO->Write_EVIOEvent( event, "cdc_goodtrack_skim" );

}

//------------------
// EndRun
//------------------
void JEventProcessor_cdc_goodtrack_skim::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_cdc_goodtrack_skim::Finish()
{
  // Called before program exit after event processing is finished.
}

