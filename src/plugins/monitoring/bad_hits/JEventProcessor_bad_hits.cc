// $Id$
//
//    File: JEventProcessor_bad_hits.cc
// Created: Sat Jan 31 01:57:43 PM EST 2026
// Creator: njarvis (on Linux gluon00 5.14.0-611.20.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_bad_hits.h"

using namespace std;


#include <JANA/JFactoryT.h>

#include <DAQ/DBadHit.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

#include <JANA/JApplication.h>

static TH1I *hroc = NULL;
static TH2I *hroc_slot = NULL;
static TH1I *hevents = NULL;  // for RSAI


// Routine used to create our JEventProcessor

extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_bad_hits());
}
} // "C"


//------------------
// JEventProcessor_bad_hits (Constructor)
//------------------
JEventProcessor_bad_hits::JEventProcessor_bad_hits()
{
	SetTypeName("JEventProcessor_bad_hits");
}

//------------------
// ~JEventProcessor_bad_hits (Destructor)
//------------------
JEventProcessor_bad_hits::~JEventProcessor_bad_hits()
{
}

//------------------
// Init
//------------------
void JEventProcessor_bad_hits::Init()
{
    // This is called once at program startup. 
    
    auto app = GetApplication();

    lockService = app->GetService<JLockService>();

    TDirectory *main = gDirectory;
    gDirectory->mkdir("bad_hits")->cd();

    hroc = new TH1I("roc","Count of data format errors for each ROC; ROC ", 120,0,120);
    hroc_slot = new TH2I("roc_slot","Count of data format errors; ROC; Slot ", 120,0,120,18,3,21);
    hevents = new TH1I("num_events","Number of events", 1, 0.0, 1.0);

    main->cd();

}

//------------------
// BeginRun
//------------------
void JEventProcessor_bad_hits::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_bad_hits::Process(const std::shared_ptr<const JEvent> &event)
{

    // Event count used by RootSpy->RSAI so it knows how many events have been seen.
    lockService->RootFillLock(this);
    hevents->Fill(0.5);
    lockService->RootFillUnLock(this);  


    vector<const DBadHit*> hitvector;
    event->Get(hitvector);

    int nd = (int)hitvector.size();

    if (nd) {
        lockService->RootFillLock(this);

	
        for (int i=0; i<nd; i++) {
  	    const DBadHit *hit = hitvector[i];

	    if (!hit) continue;
	    hroc->Fill((int)hit->rocid);
	    hroc_slot->Fill((int)hit->rocid,(int)hit->slot);
        }
        lockService->RootFillUnLock(this);
    } 



}

//------------------
// EndRun
//------------------
void JEventProcessor_bad_hits::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_bad_hits::Finish()
{
    // Called before program exit after event processing is finished.
}

