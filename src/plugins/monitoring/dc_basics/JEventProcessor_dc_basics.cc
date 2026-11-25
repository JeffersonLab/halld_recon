// $Id$
//
//    File: JEventProcessor_dc_basics.cc
// Created: Thu Oct  8 03:30:26 PM EDT 2026
// Creator: njarvis (on Linux gluon00 5.14.0-687.53.1.el9_8.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_dc_basics.h"
#include "TRIGGER/DTrigger.h"
#include "PID/DVertex.h"

#include "CDC/DCDCHit.h"
#include "CDC/DCDCTrackHit.h"

#include "FDC/DFDCHit.h"
#include "FDC/DFDCCathodeCluster.h"
#include "FDC/DFDCPseudo.h"


#include "TRACKING/DTrackTimeBased.h"
#include <PID/DChargedTrackHypothesis.h>
#include <PID/DChargedTrack.h>
#include <TRACKING/DTrackFitter.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>


static TH1F *hcdc_a = NULL;
static TH1F *hcdc_t = NULL;
static TH2F *hcdc_at = NULL;

static TH1F *hcdc_tr_a = NULL;
static TH1F *hcdc_tr_t = NULL;
static TH2F *hcdc_tr_at = NULL;

static TH2F *hcdc_dedx_qp = NULL;
static TH2F *hcdc_dedx_qm = NULL;


static TH1F *hfdc_a = NULL;
static TH1F *hfdc_q = NULL;

static TH1F *hfdc_t = NULL;
static TH1F *hfdc_wt = NULL;
static TH2F *hfdc_qt = NULL;

static TH1F *hfdc_cq = NULL;

static TH1F *hfdc_pst = NULL;
static TH1F *hfdc_psq = NULL;
static TH2F *hfdc_psqt = NULL;

static TH1F *hfdc_tr_pst = NULL;
static TH1F *hfdc_tr_psq = NULL;
static TH2F *hfdc_tr_psqt = NULL;

static TH2F *hfdc_dedx_qp = NULL;
static TH2F *hfdc_dedx_qm = NULL;


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

    TDirectory *main = gDirectory;
    gDirectory->mkdir("dc_basics")->cd();
    
    hcdc_a = new TH1F("cdc_a","CDC hit charge (from pulse height) (fC)",180, 0,18000);
    hcdc_t = new TH1F("cdc_t","CDC hit time (ns)",220,-100,1000);
    hcdc_at = new TH2F("cdc_at","CDC hit charge (from pulse height) vs time; time (ns); charge (fC)",110,-100,1000, 90, 0,18000);

    hcdc_tr_a = new TH1F("cdc_tr_a","Tracks: CDC hit charge (from pulse height) (fC)",180, 0,18000);
    hcdc_tr_t = new TH1F("cdc_tr_t","Tracks: CDC hit time (ns)",220,-100,1000);
    hcdc_tr_at = new TH2F("cdc_tr_at","Tracks: CDC hit charge (from pulse height) vs time; time (ns); charge (fC)",110,-100,1000, 90, 0,18000);
	
    hcdc_dedx_qp = new TH2F("cdc_dedx_qp", "CDC dE/dx vs momentum, q+, best FOM;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);
    hcdc_dedx_qm = new TH2F("cdc_dedx_qm", "CDC dE/dx vs momentum, q-, best FOM;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

    
    hfdc_a = new TH1F("fdc_a","FDC pulse height", 410,0,4100);
    hfdc_q = new TH1F("fdc_q","FDC hit charge (fC)", 250,0,500);
    hfdc_t = new TH1F("fdc_t","FDC cathode hit time (ns)", 250,-100,400);
    hfdc_qt = new TH2F("fdc_qt","FDC hit charge vs time;time (ns);charge (fC)",100,-100,400, 100,0,500);
    
    hfdc_wt = new TH1F("fdc_wt","FDC wire hit time (ns)", 200,-300,500);

    hfdc_cq = new TH1F("fdc_cq","FDC cathode cluster hit charge (fC)", 1000,0,4000);
    
    hfdc_psq = new TH1F("fdc_psq","FDC pseudohit charge (fC)", 100,0,25);
    hfdc_pst = new TH1F("fdc_pst","FDC pseudohit time (ns)", 250,-200,300);
    hfdc_psqt = new TH2F("fdc_psqt","FDC pseudohit charge vs time;time (ns);charge (fC)",100,-200,300, 100,0,25);
    
    hfdc_tr_psq = new TH1F("fdc_tr_psq","Tracks: FDC pseudohit charge (fC)", 100,0,25);
    hfdc_tr_pst = new TH1F("fdc_tr_pst","Tracks: FDC pseudohit time (ns)", 250,-100,400);
    hfdc_tr_psqt = new TH2F("fdc_tr_psqt","Tracks: FDC pseudohit charge vs time;time (ns);charge (fC)",100,-200,300, 100,0,25);
    
    hfdc_dedx_qp = new TH2F("fdc_dedx_qp", "FDC dE/dx vs momentum, q+, best FOM;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);
    hfdc_dedx_qm = new TH2F("fdc_dedx_qm", "FDC dE/dx vs momentum, q-, best FOM;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);


    
    main->cd();
    
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

    const DVertex* locVertex  = NULL;
    event->GetSingle(locVertex);
    double vertexz = locVertex->dSpacetimeVertex.Z();
    if ((vertexz < 52.0) || (vertexz > 78.0)) return;

    
    vector<const DCDCHit*> DCDCHits;
    event->Get(DCDCHits);

    vector<const DFDCHit*> DFDCHits;
    event->Get(DFDCHits);

    vector<const DFDCCathodeCluster*> DFDCClusters;
    event->Get(DFDCClusters);
    
    vector<const DFDCPseudo*> DFDCPseudohits;
    event->Get(DFDCPseudohits);

    vector<const DChargedTrack*> ctracks;
    event->Get(ctracks);
    
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
  	    hfdc_a->Fill(hit->pulse_height);	    
	} else {
            hfdc_wt->Fill(hit->t);
	}
    }

    for (uint32_t i=0; i<DFDCClusters.size(); i++) {
        const DFDCCathodeCluster *cluster = DFDCClusters[i];
  	hfdc_cq->Fill(cluster->q_tot);
    }
    
    for (uint32_t i=0; i<DFDCPseudohits.size(); i++) {
        const DFDCPseudo *hit = DFDCPseudohits[i];
        hfdc_pst->Fill(hit->time);
        hfdc_psq->Fill(hit->q);
        hfdc_psqt->Fill(hit->time, hit->q);
    }

    lockService->RootFillUnLock(this);

    
    for (uint32_t i=0; i<(uint32_t)ctracks.size(); i++) {  
      
        // get the best hypo
        const DChargedTrackHypothesis *hyp=ctracks[i]->Get_BestFOM();    
        if (hyp == NULL) continue;
      
        const DTrackTimeBased *track = hyp->Get_TrackTimeBased();

        double charge = track->charge();
        DVector3 mom = track->momentum();
        double p = mom.Mag();
	
        int nhits_cdc = (int)track->dNumHitsUsedFordEdx_CDC;
        int nhits_fdc = (int)track->dNumHitsUsedFordEdx_FDC; 	

        lockService->RootFillLock(this); 	

	if (nhits_cdc >=4) {

            vector<const DCDCTrackHit*> cdctrackhits;
	    track->Get(cdctrackhits);
	    
	    for (uint32_t j =0 ; j< cdctrackhits.size() ; j++) {
	        const DCDCHit *hit = NULL;
	        cdctrackhits[j]->GetSingle(hit);
                hcdc_tr_at->Fill(hit->t, hit->amp);
                hcdc_tr_a->Fill(hit->amp);	
                hcdc_tr_t->Fill(hit->t);
	    }

            if (charge>0) hcdc_dedx_qp->Fill((float)p, (float)1.0e6*track->ddEdx_CDC_amp);
            if (charge<0) hcdc_dedx_qm->Fill((float)p, (float)1.0e6*track->ddEdx_CDC_amp);	

	}

	if (nhits_fdc >=4) {

            vector<const DFDCPseudo*> fdcpseudohits;
	    track->Get(fdcpseudohits);
	    
	    for (uint32_t j =0 ; j< fdcpseudohits.size() ; j++) {
	      //	        const DCDCHit *hit = NULL;
	      //cdctrackhits[j]->GetSingle(hit);
                hfdc_tr_psqt->Fill(fdcpseudohits[j]->time, fdcpseudohits[j]->q);
                hfdc_tr_psq->Fill(fdcpseudohits[j]->q);	
                hfdc_tr_pst->Fill(fdcpseudohits[j]->time);
	    }

	    if (charge>0) hfdc_dedx_qp->Fill((float)p, (float)1.0e6*track->ddEdx_FDC);
 	    if (charge<0) hfdc_dedx_qm->Fill((float)p, (float)1.0e6*track->ddEdx_FDC);

        }	    
        lockService->RootFillUnLock(this); 
	
    //for (uint32_t j=0; j<pulls.size(); j++) {
    //          if (pulls[j].cdc_hit == NULL) continue;

    }

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

