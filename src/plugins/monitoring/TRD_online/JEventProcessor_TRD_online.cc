#include <stdint.h>
#include <vector>

#include "JEventProcessor_TRD_online.h"

using namespace std;

#include <DANA/DEvent.h>

#include <TRIGGER/DL1Trigger.h>
#include <TRD/DTRDDigiHit.h>
#include <TRD/DTRDHit.h>
#include <TRD/DTRDStripCluster.h>
#include <TRD/DTRDPoint.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>

// root hist pointers
// fix constants for now
static const int NTRDplanes = 2;
static const int NTRD_xstrips = 720;
static const int NTRD_ystrips = 432;

static TH1I *trd_num_events;
static TH1I *hDigiHit_NHits;
static TH1I *hDigiHit_QualityFactor[NTRDplanes];
static TH1I *hDigiHit_Occupancy[NTRDplanes];
static TH2I *hDigiHit_PeakVsStrip[NTRDplanes];
static TH1I *hDigiHit_PulseTime[NTRDplanes];
static TH1I *hDigiHit_Time[NTRDplanes];
static TH2I *hDigiHit_TimeVsStrip[NTRDplanes];
static TH2I *hDigiHit_TimeVsPeak[NTRDplanes];

static TH1I *hHit_NHits;
static TH1I *hHit_Occupancy[NTRDplanes];
static TH1I *hHit_Time[NTRDplanes];
static TH1I *hHit_PulseHeight[NTRDplanes];


//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_TRD_online());
    }
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_online::JEventProcessor_TRD_online() {
	SetTypeName("JEventProcessor_TRD_online");
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_online::~JEventProcessor_TRD_online() {
}


//----------------------------------------------------------------------------------

void JEventProcessor_TRD_online::Init() {

    auto app = GetApplication();
    lockService = app->GetService<JLockService>();

    // create root folder for TRD and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *trdDir = gDirectory->mkdir("TRD");
    trdDir->cd();
    // book hists
    trd_num_events = new TH1I("trd_num_events","TRD number of events",1,0.5,1.5);

    // digihit-level hists
    trdDir->cd();
    gDirectory->mkdir("DigiHit")->cd();
    hDigiHit_NHits = new TH1I("DigiHit_NfadcHits","TRD fADC hit multiplicity;raw hits;events",100,0.5,0.5+200);
    
    // histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
		int NTRDstrips = 0.;
		if(i==0)
			NTRDstrips = NTRD_xstrips;
		else
			NTRDstrips = NTRD_ystrips;

		hDigiHit_Occupancy[i] = new TH1I(Form("DigiHit_Occupancy_Plane%d", i),Form("Plane %d TRD hit occupancy;strip;raw hits / counter",i),NTRDstrips,-0.5,-0.5+NTRDstrips);
		hDigiHit_QualityFactor[i] = new TH1I(Form("DigiHit_QualityFactor_Plane%d", i),Form("Plane %d TRD quality factor;quality factor;raw hits",i),4,-0.5,3.5);
		hDigiHit_PeakVsStrip[i] = new TH2I(Form("DigiHit_PeakVsStrip_Plane%d", i),Form("Plane %d TRD pulse peak vs. strip;strip;pulse peak",i),NTRDstrips,-0.5,-0.5+NTRDstrips,410,0.0,4100.0);
		hDigiHit_PulseTime[i] = new TH1I(Form("DigiHit_PulseTime_Plane%d", i),Form("Plane %d TRD pulse time;pulse time [62.5 ps];raw hits",i),1000,0.0,5000.0);
		hDigiHit_Time[i] = new TH1I(Form("DigiHit_Time_Plane%d", i),Form("Plane %d TRD pulse time;pulse time [ns];raw hits / 2 ns",i),2500,0.0,5000.0);
		hDigiHit_TimeVsStrip[i] = new TH2I(Form("DigiHit_TimeVsStrip_Plane%d", i),Form("Plane %d TRD pulse time vs. strip;strip;pulse time [ns]",i),NTRDstrips,-0.5,-0.5+NTRDstrips,200,0.0,1000.0);
		hDigiHit_TimeVsPeak[i] = new TH2I(Form("DigiHit_TimeVsPeak_Plane%d", i),Form("Plane %d TRD time vs. peak;pulse peak;time [ns]",i),410,0.0,4100.0,200,0.0,1000.0);
		
	}
		
    // hit-level hists
    trdDir->cd();
    gDirectory->mkdir("Hit")->cd();
    hHit_NHits = new TH1I("Hit_NHits","TRD calibrated hit multiplicity;calibrated hits;events",100,0.5,0.5+200);
    
    // histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
		int NTRDstrips = 0.;
		if(i==0)
			NTRDstrips = NTRD_xstrips;
		else
			NTRDstrips = NTRD_ystrips;

		hHit_Occupancy[i] = new TH1I(Form("Hit_Occupancy_Plane%d", i),Form("Plane %d TRD hit occupancy;strip;calibrated hits / counter",i),NTRDstrips,-0.5,-0.5+NTRDstrips);
		hHit_Time[i] = new TH1I(Form("Hit_Time_Plane%d", i),Form("Plane %d TRD pulse time;pulse time [ns];calibrated hits / 2 ns",i),1000,-500.0,1500.0);
		hHit_PulseHeight[i] = new TH1I(Form("Hit_PulseHeight_Plane%d", i),Form("Plane %d TRD pulse height;pulse height [fADC units];calibrated hits / 1 unit",i),600,-100.0,500.0);

	}
    

    trdDir->cd();


    
    // back to main dir
    mainDir->cd();
}

//----------------------------------------------------------------------------------


void JEventProcessor_TRD_online::BeginRun(const std::shared_ptr<const JEvent>& event) {
    // This is called whenever the run number changes
    auto runnumber = event->GetRunNumber();

    const DGeometry *geom = GetDGeometry(event);

    vector<double> z_trd;
    geom->GetTRDZ(z_trd);
}


//----------------------------------------------------------------------------------


void JEventProcessor_TRD_online::Process(const std::shared_ptr<const JEvent>& event) {
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // loop-Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

/*
    // Get trigger words and filter on PS trigger (if it exists?)
    const DL1Trigger *trig_words = nullptr;
    uint32_t trig_mask, fp_trig_mask;
    try {
        event->GetSingle(trig_words);
    } catch(...) {};
    if (trig_words != nullptr) {
        trig_mask = trig_words->trig_mask;
        fp_trig_mask = trig_words->fp_trig_mask;
    }
    else {
        trig_mask = 0;
        fp_trig_mask = 0;
    }
    int trig_bits = fp_trig_mask > 0 ? 10 + fp_trig_mask:trig_mask;
    // Select PS-triggered events
    if (trig_bits != 8) {
        return;
    }
*/

    vector<const DTRDDigiHit*> digihits;
    event->Get(digihits);
    vector<const DTRDHit*> hits;
    event->Get(hits);
//     vector<const DTRDStripCluster*> clusters;
//     eventLoop->Get(clusters);
//     vector<const DTRDPoint*> points;
//     eventLoop->Get(points);

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK

    ///////////////////////////
    // TRD DigiHits and Hits //
    ///////////////////////////

    if (digihits.size() > 0) trd_num_events->Fill(1);
    hDigiHit_NHits->Fill(digihits.size());
    for (const auto& hit : digihits) {
	    int plane = hit->plane-1;  // plane and strip counting starts from 1
	    int strip = hit->strip;
	    
	    hDigiHit_Occupancy[plane]->Fill(strip);
	    hDigiHit_QualityFactor[plane]->Fill(hit->QF);
	    hDigiHit_PeakVsStrip[plane]->Fill(strip,hit->pulse_peak);
	    hDigiHit_PulseTime[plane]->Fill(hit->pulse_time);
	    double t_ns = 0.8*hit->pulse_time;
	    hDigiHit_Time[plane]->Fill(t_ns);
	    hDigiHit_TimeVsStrip[plane]->Fill(strip,t_ns);
	    hDigiHit_TimeVsPeak[plane]->Fill(hit->pulse_peak,t_ns);
    }


    hHit_NHits->Fill(hits.size());
    for (const auto& hit : hits) {
	    int plane = hit->plane-1;

	    hHit_Occupancy[plane]->Fill(hit->strip);
	    hHit_Time[plane]->Fill(hit->t);
	    hHit_PulseHeight[plane]->Fill(hit->pulse_height);
    }



    lockService->RootUnLock(); //RELEASE ROOT FILL LOCK
}
//----------------------------------------------------------------------------------


void JEventProcessor_TRD_online::EndRun() {
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}


//----------------------------------------------------------------------------------


void JEventProcessor_TRD_online::Finish() {
    // Called before program exit after event processing is finished.
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
