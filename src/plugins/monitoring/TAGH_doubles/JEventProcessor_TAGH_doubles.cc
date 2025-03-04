// $Id$
//
//    File: JEventProcessor_TAGH_doubles.cc
// Created: Fri Apr 29 15:19:27 EDT 2016
// Creator: nsparks (on Linux cua2.jlab.org 3.10.0-327.13.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_TAGH_doubles.h"
using namespace std;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>

#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGHGeometry.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

const int Nslots = DTAGHGeometry::kCounterCount;
const int NmultBins = 200; // number of bins for multiplicity histograms

static TH2I *hBM_tdiffVsIDdiff;
static TH2I *hAM_tdiffVsIDdiff;
static TH1F *hBM1_Occupancy;
static TH1F *hBM2_Occupancy;
static TH1F *hAM1_Occupancy;
static TH1F *hAM2_Occupancy;
static TH1F *hAM3_Occupancy;
static TH1F *hBM1_Energy;
static TH1F *hBM2_Energy;
static TH1F *hAM1_Energy;
static TH1F *hAM2_Energy;
static TH1F *hAM3_Energy;
static TH1I *hBM_NHits;
static TH1I *hAM_NHits;
static TH2I *hBM1_PulseHeightVsID;
static TH2I *hBM2_PulseHeightVsID;

extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_TAGH_doubles());
    }
} // "C"


//------------------
// JEventProcessor_TAGH_doubles (Constructor)
//------------------
JEventProcessor_TAGH_doubles::JEventProcessor_TAGH_doubles()
{
	SetTypeName("JEventProcessor_TAGH_doubles");
}

//------------------
// ~JEventProcessor_TAGH_doubles (Destructor)
//------------------
JEventProcessor_TAGH_doubles::~JEventProcessor_TAGH_doubles()
{

}

//------------------
// Init
//------------------
void JEventProcessor_TAGH_doubles::Init()
{
    auto app = GetApplication();
    lockService = app->GetService<JLockService>();

    // This is called once at program startup.
    TDirectory *mainDir = gDirectory;
    TDirectory *taghDir = gDirectory->mkdir("TAGH_doubles");
    taghDir->cd();
    gDirectory->mkdir("BeforeMergingDoubles")->cd();
    hBM_tdiffVsIDdiff = new TH2I("BM_tdiffVsIDdiff","TAGH 2-hit time difference vs. counter ID difference;counter ID difference;time difference [ns]",15,0.5,15.5,200,-5.0,5.0);
    hBM_NHits = new TH1I("BM_NHits","TAGH multiplicity: All hits, before merging doubles;hits;events",NmultBins,0.5,0.5+NmultBins);
    hBM1_Occupancy = new TH1F("BM1_Occupancy","TAGH occ.: All hits, before merging doubles;counter (slot) ID;hits / counter",Nslots,0.5,0.5+Nslots);
    hBM2_Occupancy = new TH1F("BM2_Occupancy","TAGH occ.: Doubles only, before merging doubles;counter (slot) ID;hits / counter",Nslots,0.5,0.5+Nslots);
    hBM1_Energy    = new TH1F("BM1_Energy","TAGH energy: All hits, before merging doubles;photon energy [GeV];hits / counter",180,3.0,12.0);
    hBM2_Energy    = new TH1F("BM2_Energy","TAGH energy: Doubles only, before merging doubles;photon energy [GeV];hits / counter",180,3.0,12.0);
    hBM1_PulseHeightVsID = new TH2I("BM1_PulseHeightVsID","TAGH ADC pulse height vs. ID: All hits;counter (slot) ID;pulse height",Nslots,0.5,0.5+Nslots,410,0.0,4100.0);
    hBM2_PulseHeightVsID = new TH2I("BM2_PulseHeightVsID","TAGH ADC pulse height vs. ID: Doubles only;counter (slot) ID;pulse height",Nslots,0.5,0.5+Nslots,410,0.0,4100.0);
    taghDir->cd();
    gDirectory->mkdir("AfterMergingDoubles")->cd();
    hAM_tdiffVsIDdiff = new TH2I("AM_tdiffVsIDdiff","TAGH 2-hit time difference vs. counter ID difference;counter ID difference;time difference [ns]",15,0.5,15.5,200,-5.0,5.0);
    hAM_NHits = new TH1I("AM_NHits","TAGH multiplicity: All hits, after merging doubles;hits;events",NmultBins,0.5,0.5+NmultBins);
    hAM1_Occupancy    = new TH1F("AM1_Occupancy","TAGH occ.: All hits, after merging doubles;counter (slot) ID;hits / counter",Nslots,0.5,0.5+Nslots);
    hAM2_Occupancy    = new TH1F("AM2_Occupancy","TAGH occ.: Doubles only, after merging doubles;counter (slot) ID;hits / counter",Nslots,0.5,0.5+Nslots);
    hAM3_Occupancy    = new TH1F("AM3_Occupancy","TAGH occ.: Doubles only w/ overlaps, after merging doubles;counter (slot) ID;hits / counter",Nslots,0.5,0.5+Nslots);
    hAM1_Energy    = new TH1F("AM1_Energy","TAGH energy: All hits, after merging doubles;photon energy [GeV];hits / counter",180,3.0,12.0);
    hAM2_Energy    = new TH1F("AM2_Energy","TAGH energy: Doubles only, after merging doubles;photon energy [GeV];hits / counter",180,3.0,12.0);
    hAM3_Energy    = new TH1F("AM3_Energy","TAGH energy: Doubles only w/ overlaps, after merging doubles;photon energy [GeV];hits / counter",180,3.0,12.0);
    // back to main dir
    mainDir->cd();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_TAGH_doubles::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_TAGH_doubles::Process(const std::shared_ptr<const JEvent>& event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

    // Get unmerged hits
    vector<const DTAGHHit*> hits_c;
    event->Get(hits_c, "Calib");
    // Get merged hits
    vector<const DTAGHHit*> hits;
    event->Get(hits);

    // Extract the TAGH geometry
    vector<const DTAGHGeometry*> taghGeomVect;
    event->Get(taghGeomVect);
    if (taghGeomVect.size() < 1)
        throw JException("TAGHGeometry missing");

    const DTAGHGeometry& taghGeom = *(taghGeomVect[0]);

    lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK
    // Before merging doubles
    int BM_NHits = 0;
    for (const auto& hit : hits_c) {
        if (!hit->has_TDC||!hit->has_fADC) continue;
        BM_NHits++;
        hBM1_Occupancy->Fill(hit->counter_id);
        hBM1_Energy->Fill(hit->E);
        hBM1_PulseHeightVsID->Fill(hit->counter_id,hit->pulse_peak);
        if (hit->is_double) hBM2_Occupancy->Fill(hit->counter_id);
        if (hit->is_double) hBM2_Energy->Fill(hit->E);
        if (hit->is_double) hBM2_PulseHeightVsID->Fill(hit->counter_id,hit->pulse_peak);
    }
    hBM_NHits->Fill(BM_NHits);
    for (size_t i = 0; i < hits_c.size(); i++) {
        const DTAGHHit* hit1 = hits_c[i];
        if (!hit1->has_TDC||!hit1->has_fADC) continue;
        for (size_t j = i+1; j < hits_c.size(); j++) {
            const DTAGHHit* hit2 = hits_c[j];
            if (!hit2->has_TDC||!hit2->has_fADC) continue;
            hBM_tdiffVsIDdiff->Fill(abs(hit1->counter_id-hit2->counter_id),hit1->t-hit2->t);
        }
    }
    // After merging doubles
    int AM_NHits = 0;
    for (const auto& hit : hits) {
        if (!hit->has_TDC||!hit->has_fADC) continue;
        AM_NHits++;
        bool has_overlap = false;
        if (hit->counter_id < 274) {
            double El = taghGeom.getElow(hit->counter_id); double Eh1 = taghGeom.getEhigh(hit->counter_id+1);
            has_overlap = (Eh1 > El);
        }
        hAM1_Occupancy->Fill(hit->counter_id);
        hAM1_Energy->Fill(hit->E);
        if (hit->is_double) hAM2_Occupancy->Fill(hit->counter_id);
        if (hit->is_double) hAM2_Energy->Fill(hit->E);
        if (hit->is_double && has_overlap) hAM3_Occupancy->Fill(hit->counter_id);
        if (hit->is_double && has_overlap) hAM3_Energy->Fill(hit->E);
    }
    hAM_NHits->Fill(AM_NHits);
    for (size_t i = 0; i < hits.size(); i++) {
        const DTAGHHit* hit1 = hits[i];
        if (!hit1->has_TDC||!hit1->has_fADC) continue;
        for (size_t j = i+1; j < hits.size(); j++) {
            const DTAGHHit* hit2 = hits[j];
            if (!hit2->has_TDC||!hit2->has_fADC) continue;
            hAM_tdiffVsIDdiff->Fill(abs(hit1->counter_id-hit2->counter_id),hit1->t-hit2->t);
        }
    }
    lockService->RootUnLock(); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_TAGH_doubles::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_TAGH_doubles::Finish()
{
    // Called before program exit after event processing is finished.
}
