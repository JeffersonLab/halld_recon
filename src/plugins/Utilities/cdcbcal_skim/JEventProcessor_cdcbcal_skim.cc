// $Id$
//
//    File: JEventProcessor_cdcbcal_skim.cc
// Created: Mon Dec  1 14:57:11 EST 2014 (copied structure from pi0fcalskim plugin)
// Creator: wmcginle (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
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
#include "JEventProcessor_cdcbcal_skim.h"


#include "TRACKING/DMCThrown.h"
// Routine used to create our JEventProcessor
#include "PID/DVertex.h"
#include "DANA/DEvent.h"
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
    app->Add(new JEventProcessor_cdcbcal_skim());
  }
} // "C"


//------------------
// JEventProcessor_cdcbcal_skim (Constructor)
//------------------
JEventProcessor_cdcbcal_skim::JEventProcessor_cdcbcal_skim()
{
  SetTypeName("JEventProcessor_cdcbcal_skim");
}

//------------------
// ~JEventProcessor_cdcbcal_skim (Destructor)
//------------------
JEventProcessor_cdcbcal_skim::~JEventProcessor_cdcbcal_skim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_cdcbcal_skim::Init()
{  
  num_epics_events = 0;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_cdcbcal_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void JEventProcessor_cdcbcal_skim::Process(const std::shared_ptr<const JEvent>& event)
{
    const DEventWriterEVIO* locEventWriterEVIO = NULL;
    event->GetSingle(locEventWriterEVIO);

	// always write out BOR events
	if(GetStatusBit(event, kSTATUS_BOR_EVENT)) {
	  //jout << "Found BOR!" << endl;
	  locEventWriterEVIO->Write_EVIOEvent( event, "cdcbcal_skim" );
	  return;
	}

	// write out the first few EPICS events to save run number & other meta info
	if(GetStatusBit(event, kSTATUS_EPICS_EVENT) && (num_epics_events<5)) {
	  //jout << "Found EPICS!" << endl;
	  locEventWriterEVIO->Write_EVIOEvent( event, "cdcbcal_skim" );
	  num_epics_events++;
	  return;
	}

	vector<const DTrackTimeBased *> tracks;
	event->Get(tracks);

	if(tracks.size() == 0)
		return;

	vector<const DDetectorMatches *> detectorMatchVec;
	event->Get(detectorMatchVec);
	const DDetectorMatches *locDetectorMatching = detectorMatchVec[0];

	vector<shared_ptr<const DBCALShowerMatchParams> > locBCALMatchParams;

	// only save events with a track that is matched to the BCAL
	bool save_event = false;
	for( auto track : tracks ) {
		locDetectorMatching->Get_BCALMatchParams(track,locBCALMatchParams);
		if(locBCALMatchParams.size() > 0) {
			save_event = true;
			break;
		}
	}

	if(save_event)
		locEventWriterEVIO->Write_EVIOEvent( event, "cdcbcal_skim" );
}

//------------------
// EndRun
//------------------
void JEventProcessor_cdcbcal_skim::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Fin
//------------------
void JEventProcessor_cdcbcal_skim::Finish()
{
  // Called before program exit after event processing is finished.
}

