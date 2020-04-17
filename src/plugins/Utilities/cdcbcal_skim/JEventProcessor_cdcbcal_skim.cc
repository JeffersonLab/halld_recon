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
    app->AddProcessor(new JEventProcessor_cdcbcal_skim());
  }
} // "C"


//------------------
// JEventProcessor_cdcbcal_skim (Constructor)
//------------------
JEventProcessor_cdcbcal_skim::JEventProcessor_cdcbcal_skim()
{
  
}

//------------------
// ~JEventProcessor_cdcbcal_skim (Destructor)
//------------------
JEventProcessor_cdcbcal_skim::~JEventProcessor_cdcbcal_skim()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_cdcbcal_skim::init(void)
{  
  num_epics_events = 0;
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_cdcbcal_skim::brun(JEventLoop *eventLoop, int32_t runnumber)
{
    return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_cdcbcal_skim::evnt(JEventLoop *loop, uint64_t eventnumber)
{
    const DEventWriterEVIO* locEventWriterEVIO = NULL;
    loop->GetSingle(locEventWriterEVIO);

	// always write out BOR events
	if(loop->GetJEvent().GetStatusBit(kSTATUS_BOR_EVENT)) {
	  //jout << "Found BOR!" << endl;
	  locEventWriterEVIO->Write_EVIOEvent( loop, "cdcbcal_skim" );
	  return NOERROR;
	}

	// write out the first few EPICS events to save run number & other meta info
	if(loop->GetJEvent().GetStatusBit(kSTATUS_EPICS_EVENT) && (num_epics_events<5)) {
	  //jout << "Found EPICS!" << endl;
	  locEventWriterEVIO->Write_EVIOEvent( loop, "cdcbcal_skim" );
	  num_epics_events++;
	  return NOERROR;
	}

	vector<const DTrackTimeBased *> tracks;
	loop->Get(tracks);

	if(tracks.size() == 0)
		return NOERROR;

	vector<const DDetectorMatches *> detectorMatchVec;
	loop->Get(detectorMatchVec);
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
		locEventWriterEVIO->Write_EVIOEvent( loop, "cdcbcal_skim" );

    return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_cdcbcal_skim::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// Fin
//------------------
jerror_t JEventProcessor_cdcbcal_skim::fini(void)
{
  // Called before program exit after event processing is finished.
  return NOERROR;
}

