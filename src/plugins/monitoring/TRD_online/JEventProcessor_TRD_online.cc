#include <stdint.h>
#include <vector>

#include "JEventProcessor_TRD_online.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include <DANA/DApplication.h>

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
const int NTRDplanes = 4;
const int NTRDstrips = 240;
const int NTRDwires = 24;
const int NGEMplanes = 10;
const int NGEMstrips = 256;
const int NAPVchannels = 128;
const int NGEMsamples = 21;

static TH1I *trd_num_events;
static TH1I *hDigiHit_NHits;
static TH1I *hDigiHit_QualityFactor[NTRDplanes];
static TH1I *hDigiHit_Occupancy[NTRDplanes];
static TH2I *hDigiHit_PeakVsStrip[NTRDplanes];
static TH1I *hDigiHit_PulseTime[NTRDplanes];
static TH1I *hDigiHit_Time[NTRDplanes];
static TH2I *hDigiHit_TimeVsStrip[NTRDplanes];
static TH2I *hDigiHit_TimeVsPeak[NTRDplanes];

static TH2I *hWireTRDPointAmp_DeltaT, *hGEMTRDPointAmp_DeltaT;
static TH2I *hGEMSRSPointAmp_DeltaT, *hGEMSRSPointAmp_DeltaT_Good;
static TH2I *hWireTRDPoint_WireStrip, *hGEMTRDPoint_XY; 
static TH1I *hWireTRDX_Time, *hGEMTRDX_Time;
static TH2I *hGEMTRDPoint_StripCorr;

static TH2I *hWire_GEMTRDX, *hWire_GEMTRDXstrip, *hWire_GEMTRDX_DeltaT;
static TH2I *hStrip_GEMTRDY;
static TH2I *hWire_GEMSRSXstrip[5], *hStrip_GEMSRSYstrip[5];
static TH2I *hStrip_GEMSRSY[5], *hWire_GEMSRSX[5];

//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->AddProcessor(new JEventProcessor_TRD_online());
    }
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_online::JEventProcessor_TRD_online() {
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_online::~JEventProcessor_TRD_online() {
}


//----------------------------------------------------------------------------------

jerror_t JEventProcessor_TRD_online::init(void) {

    // create root folder for TRD and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *trdDir = gDirectory->mkdir("TRD");
    trdDir->cd();
    // book hists
    trd_num_events = new TH1I("trd_num_events","TRD number of events",1,0.5,1.5);

    // digihit-level hists
    trdDir->cd();
    gDirectory->mkdir("DigiHit")->cd();
    hDigiHit_NHits = new TH1I("DigiHit_NfadcHits","TRD fADC hit multiplicity;raw hits;events",100,0.5,0.5+100);
    
    // histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
	    if(i<2) { // Wire TRD
		    hDigiHit_Occupancy[i] = new TH1I(Form("DigiHit_Occupancy_WirePlane%d", i),Form("Plane %d TRD Wire hit occupancy;strip;raw hits / counter",i),NTRDwires,-0.5,-0.5+NTRDwires);
		    hDigiHit_QualityFactor[i] = new TH1I(Form("DigiHit_QualityFactor_WirePlane%d", i),Form("Plane %d TRD Wire quality factor;quality factor;raw hits",i),4,-0.5,3.5);
		    hDigiHit_PeakVsStrip[i] = new TH2I(Form("DigiHit_PeakVsStrip_WirePlane%d", i),Form("Plane %d TRD Wire pulse peak vs. strip;strip;pulse peak",i),NTRDwires,-0.5,-0.5+NTRDwires,410,0.0,4100.0);
		    hDigiHit_PulseTime[i] = new TH1I(Form("DigiHit_PulseTime_WirePlane%d", i),Form("Plane %d TRD Wire pulse time;pulse time [62.5 ps];raw hits",i),1000,0.0,5000.0);
		    hDigiHit_Time[i] = new TH1I(Form("DigiHit_Time_WirePlane%d", i),Form("Plane %d TRD Wire pulse time;pulse time [ns];raw hits / 2 ns",i),2500,0.0,5000.0);
		    hDigiHit_TimeVsStrip[i] = new TH2I(Form("DigiHit_TimeVsStrip_WirePlane%d", i),Form("Plane %d TRD Wire pulse time vs. strip;strip;pulse time [ns]",i),NTRDwires,-0.5,-0.5+NTRDwires,200,0.0,1000.0);
		    hDigiHit_TimeVsPeak[i] = new TH2I(Form("DigiHit_TimeVsPeak_WirePlane%d", i),Form("Plane %d TRD Wire time vs. peak;pulse peak;time [ns]",i),410,0.0,4100.0,200,0.0,1000.0);
	    }
	    else { // GEM TRD
		    hDigiHit_Occupancy[i] = new TH1I(Form("DigiHit_Occupancy_GEMPlane%d", i),Form("Plane %d TRD GEM hit occupancy;strip;raw hits / counter",i),NTRDstrips,-0.5,-0.5+NTRDstrips);
		    hDigiHit_QualityFactor[i] = new TH1I(Form("DigiHit_QualityFactor_GEMPlane%d", i),Form("Plane %d TRD GEM hit quality factor;quality factor;raw hits",i),4,-0.5,3.5);
		    hDigiHit_PeakVsStrip[i] = new TH2I(Form("DigiHit_PeakVsStrip_GEMPlane%d", i),Form("Plane %d TRD GEM pulse peak vs. strip;strip;pulse peak",i),NTRDstrips,-0.5,-0.5+NTRDstrips,410,0.0,4100.0);
		    hDigiHit_PulseTime[i] = new TH1I(Form("DigiHit_PulseTime_GEMPlane%d", i),Form("Plane %d TRD GEM pulse time;pulse time [62.5 ps];raw hits",i),1000,0.0,5000.0);
		    hDigiHit_Time[i] = new TH1I(Form("DigiHit_Time_GEMPlane%d", i),Form("Plane %d TRD GEM pulse time;pulse time [ns];raw hits / 2 ns",i),2500,0.0,5000.0);
		    hDigiHit_TimeVsStrip[i] = new TH2I(Form("DigiHit_TimeVsStrip_GEMPlane%d", i),Form("Plane %d TRD GEM pulse time vs. strip;strip;pulse time [ns]",i),NTRDstrips,-0.5,-0.5+NTRDstrips,200,0.0,1000.0);
		    hDigiHit_TimeVsPeak[i] = new TH2I(Form("DigiHit_TimeVsPeak_GEMPlane%d", i),Form("Plane %d TRD GEM time vs. peak;pulse peak;time [ns]",i),410,0.0,4100.0,200,0.0,1000.0);
	    }
    }

    trdDir->cd();

    trdDir->cd();
    gDirectory->mkdir("Correlations")->cd();

//     // TRD plane correlations
//     hWireTRDPointAmp_DeltaT = new TH2I("WireTRDPointAmp_DeltaT", "Wire TRD Point amplitude vs #Delta t; #Delta t (ns); Pulse Amplitude", 100, -100, 100, 100, 0, 4000);
//     hGEMTRDPointAmp_DeltaT = new TH2I("GEMTRDPointAmp_DeltaT", "GEM TRD Point amplitude vs #Delta t; #Delta t (ns); Pulse Amplitude", 100, -100, 100, 100, 0, 4000);
//     hWireTRDPoint_WireStrip = new TH2I("WireTRDPoint_WireStrip", "Wire TRD Point Strip (Y) vx Wire (X); X - Wire # ; Y - Strip #", NTRDwires, -0.5, -0.5+NTRDwires, NTRDwires, -0.5, -0.5+NTRDwires);
//     hGEMTRDPoint_XY = new TH2I("GEMTRDPoint_XY", "GEM TRD Point Y vx X; X (cm); Y (cm)", 100, 0., 10., 100, 0., 10.);
//     hGEMTRDPoint_StripCorr = new TH2I("GEMTRDPoint_StripCorr", "GEM TRD Point Strip (Y) vx (X); X - Strip # ; Y - Strip #", NGEMstrips, -0.5, -0.5+NGEMstrips, NGEMstrips, -0.5, -0.5+NGEMstrips);

    
    // back to main dir
    mainDir->cd();

    return NOERROR;
}

//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_online::brun(JEventLoop *eventLoop, int32_t runnumber) {
    // This is called whenever the run number changes

    // special conditions for different geometries
    if(runnumber < 70000) wirePlaneOffset = 0;
    else wirePlaneOffset = 4;

    DApplication* dapp = dynamic_cast<DApplication*>(eventLoop->GetJApplication());
    const DGeometry *geom = dapp->GetDGeometry(runnumber);
    vector<double> z_trd;
    geom->GetTRDZ(z_trd);

    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_online::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
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
        eventLoop->GetSingle(trig_words);
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
        return NOERROR;
    }
*/

    vector<const DTRDDigiHit*> digihits;
    eventLoop->Get(digihits);
    vector<const DTRDHit*> hits;
    eventLoop->Get(hits);
    vector<const DTRDStripCluster*> clusters;
    eventLoop->Get(clusters);
    vector<const DTRDPoint*> points;
    eventLoop->Get(points);

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    ///////////////////////////
    // TRD DigiHits and Hits //
    ///////////////////////////

    if (digihits.size() > 0) trd_num_events->Fill(1);
    hDigiHit_NHits->Fill(digihits.size());
    for (const auto& hit : digihits) {
	    int plane = hit->plane;
	    int iplane = plane - wirePlaneOffset; // TRD plane indexing
	    int strip = hit->strip;
	    hDigiHit_Occupancy[iplane]->Fill(strip);
	    hDigiHit_QualityFactor[iplane]->Fill(hit->QF);
	    hDigiHit_PeakVsStrip[iplane]->Fill(strip,hit->pulse_peak);
	    hDigiHit_PulseTime[iplane]->Fill(hit->pulse_time);
	    double t_ns = 0.8*hit->pulse_time;
	    hDigiHit_Time[iplane]->Fill(t_ns);
	    hDigiHit_TimeVsStrip[iplane]->Fill(strip,t_ns);
	    hDigiHit_TimeVsPeak[iplane]->Fill(hit->pulse_peak,t_ns);
    }

    //////////////////////
    // Wire/GEM Summary //
    //////////////////////

    // Wire TRD hits correlation in XY plane
    for (const auto& hit : hits) {
	    if(hit->plane != 0 && hit->plane != 4) continue;

	    for (const auto& hit_strip : hits) {
		    if(hit_strip->plane != 1 && hit_strip->plane != 5) continue;
		    
		    hWireTRDPoint_WireStrip->Fill(hit->strip, hit_strip->strip);
	    }
    }

    // Wire TRD points correlation
    for (const auto& point : points) {
	    if(point->detector == 0) { // Wire TRD
		    hWireTRDPointAmp_DeltaT->Fill(point->t_x - point->t_y, point->dE_amp);
	    }

    }



    japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

    return NOERROR;
}
//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_online::erun(void) {
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_online::fini(void) {
    // Called before program exit after event processing is finished.
    return NOERROR;
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
