// $Id$
//
//    File: JEventProcessor_dc_basics.cc
// Created: Thu Oct  8 03:30:26 PM EDT 2026
// Creator: njarvis (on Linux gluon00 5.14.0-687.53.1.el9_8.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_dc_basics.h"
#include "TRIGGER/DTrigger.h"
#include "CDC/DCDCHit.h"
#include "FDC/DFDCHit.h"

#include <TH2.h>




static TH2F *hcdc_at = NULL;
static TH1F *hcdc_a = NULL;
static TH1F *hcdc_t = NULL;


static TH2F *hfdc_qt = NULL;
static TH1F *hfdc_q = NULL;
static TH1F *hfdc_t = NULL;



// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_dc_basics());
}
} // "C"


//------------------
// JEventProcessor_dc_basics (Constructor)
//------------------
JEventProcessor_dc_basics::JEventProcessor_dc_basics()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_dc_basics (Destructor)
//------------------
JEventProcessor_dc_basics::~JEventProcessor_dc_basics()
{
}

//------------------
// Init
//------------------
void JEventProcessor_dc_basics::Init()
{
    // This is called once at program startup. 
    
    auto app = GetApplication();
    // lockService should be initialized here like this
    lockService = app->GetService<JLockService>();


    hcdc_at = new TH2F("cdc_at","CDC hit charge (from pulse height) vs time; time (ns); charge (fC)",100,0,1000, 90, 0,18000);
    hcdc_a = new TH1F("cdc_a","CDC hit charge (from pulse height) (fC)",180, 0,18000);
    hcdc_t = new TH1F("cdc_t","CDC hit time (ns)",250,0,1000);
    
    hfdc_qt = new TH2F("fdc_qt","FDC hit charge vs time;time (ns);charge (fC)",100,0,1000, 100,0,500);
    hfdc_q = new TH1F("fdc_q","FDC hit charge (fC)", 250,0,500);
    hfdc_t = new TH1F("fdc_t","FDC hit time (ns)", 250,0,1000);


    


    
}

//------------------
// BeginRun
//------------------
void JEventProcessor_dc_basics::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_dc_basics::Process(const std::shared_ptr<const JEvent> &event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.
    // Here's an example:
    //


    const DTrigger* locTrigger = NULL; 
    event->GetSingle(locTrigger); 
    if (locTrigger->Get_L1FrontPanelTriggerBits()) return;
    if (!locTrigger->Get_IsPhysicsEvent()) return; // ignore PS triggers
  
    vector<const DCDCHit*> DCDCHits;
    event->Get(DCDCHits);

    vector<const DFDCHit*> DFDCHits;
    event->Get(DFDCHits);

    lockService->RootFillLock(this);   

    for (uint32_t i=0; i<DCDCHits.size(); i++) {
        const DCDCHit *hit = DCDCHits[i];
        hcdc_at->Fill(hit->t, hit->amp);
        hcdc_a->Fill(hit->amp);
        hcdc_t->Fill(hit->t);
    }
   

    for (uint32_t i=0; i<DFDCHits.size(); i++) {
        const DFDCHit *hit = DFDCHits[i];
	if (hit->type) {                            
  	    hfdc_qt->Fill(hit->t, hit->q);
  	    hfdc_q->Fill(hit->q);
  	    hfdc_t->Fill(hit->t);
	}
    }
    
    lockService->RootFillUnLock(this);

}

//------------------
// EndRun
//------------------
void JEventProcessor_dc_basics::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_dc_basics::Finish()
{
    // Called before program exit after event processing is finished.
}

