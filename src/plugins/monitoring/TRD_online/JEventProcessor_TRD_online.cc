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
// #include <TRD/DTRDPoint.h>

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
static TH2I *hHit_TimeVsStrip[NTRDplanes];

const int NEventsClusterMonitor = 10;
static TH2I *hClusterHits_TimeVsStrip[NTRDplanes];
static TH2I *hCluster_TimeVsStrip[NTRDplanes];
static TH2I *hClusterHits_TimeVsStripEvent[NTRDplanes][NEventsClusterMonitor];
static TH2I *hCluster_TimeVsStripEvent[NTRDplanes][NEventsClusterMonitor];

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
		hDigiHit_PulseTime[i] = new TH1I(Form("DigiHit_PulseTime_Plane%d", i),Form("Plane %d TRD pulse time;pulse time [62.5 ps];raw hits",i),250,0.0,2000.0);
		hDigiHit_Time[i] = new TH1I(Form("DigiHit_Time_Plane%d", i),Form("Plane %d TRD pulse time;pulse time [ns];raw hits / 2 ns",i),250,0.0,2000.0);
		hDigiHit_TimeVsStrip[i] = new TH2I(Form("DigiHit_TimeVsStrip_Plane%d", i),Form("Plane %d TRD pulse time vs. strip;strip;pulse time [ns]",i),NTRDstrips,-0.5,-0.5+NTRDstrips,250,0.0,2000.0);
		hDigiHit_TimeVsPeak[i] = new TH2I(Form("DigiHit_TimeVsPeak_Plane%d", i),Form("Plane %d TRD time vs. peak;pulse peak;time [ns]",i),410,0.0,4100.0,250,0.0,2000.0);
		
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
		hHit_Time[i] = new TH1I(Form("Hit_Time_Plane%d", i),Form("Plane %d TRD pulse time;pulse time [ns];calibrated hits / 2 ns",i),250,0.0,2000.0);
		hHit_PulseHeight[i] = new TH1I(Form("Hit_PulseHeight_Plane%d", i),Form("Plane %d TRD pulse height;pulse height [fADC units];calibrated hits / 1 unit",i),600,-100.0,500.0);
        hHit_TimeVsStrip[i] = new TH2I(Form("Hit_TimeVsStrip_Plane%d", i),Form("Plane %d TRD pulse time vs. strip;strip;pulse time [ns]",i),NTRDstrips,-0.5,-0.5+NTRDstrips,250,0.0,2000.0);

	}
    

    trdDir->cd();

    eventClusterCount = 0;

    gDirectory->mkdir("Cluster")->cd();
    // hCluster_NClusters = new TH1I("Cluster_NClusters","TRD number of cluster per event;clusters;events",20,0.5,0.5+20);

    // histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
        int NTRDstrips = 0.;
        if(i==0)
	{    NTRDstrips = NTRD_xstrips;}
        else
	{    NTRDstrips = NTRD_ystrips;}

            hClusterHits_TimeVsStrip[i] = new TH2I(Form("ClusterHits_TimeVsStrip_Plane%d", i),Form("Plane %d TRD cluster hits pulse time vs. strip;pulse time [ns];strip",i),250,0,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
            hCluster_TimeVsStrip[i] = new TH2I(Form("Cluster_TimeVsStrip_Plane%d", i),Form("Plane %d TRD cluster pulse time vs. strip;pulse time [ns];strip",i),250,0.,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
        for(int j=0; j<NEventsClusterMonitor; j++) {
            hClusterHits_TimeVsStripEvent[i][j] = new TH2I(Form("ClusterHits_TimeVsStrip_Plane%d_Event%d", i, j),Form("Plane %d TRD cluster hits pulse time vs. strip;pulse time [ns];strip",i),250,0,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
            hCluster_TimeVsStripEvent[i][j] = new TH2I(Form("Cluster_TimeVsStrip_Plane%d_Event%d", i, j),Form("Plane %d TRD cluster pulse time vs. strip;pulse time [ns];strip",i),250,0.,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
        }
    }   
    
    // back to main dir
    mainDir->cd();

    return NOERROR;
}

//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_online::brun(JEventLoop *eventLoop, int32_t runnumber) {
    // This is called whenever the run number changes

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
//     vector<const DTRDPoint*> points;
//     eventLoop->Get(points);

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

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
	    double t_ns = 8.*hit->peak_time;
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
        hHit_TimeVsStrip[plane]->Fill(hit->strip, hit->t);
    }
       if (clusters.size() > 10 && eventClusterCount < NEventsClusterMonitor) {
        cout << "Event " << eventnumber << " has " << clusters.size() << " clusters, eventClusterCount = " << eventClusterCount << endl;
        for (const auto& cluster : clusters) {
            int plane = cluster->plane-1;
            double pos = 0.;
            if (plane == 0) pos = cluster->pos.x();
            else pos = cluster->pos.y();

            hCluster_TimeVsStripEvent[plane][eventClusterCount]->Fill(cluster->t_avg, pos);
        }

        for (const auto& hit : hits) {
            int plane = hit->plane-1;
            hClusterHits_TimeVsStripEvent[plane][eventClusterCount]->Fill(hit->t, hit->strip);
        }

        eventClusterCount++;
    }


        for (const auto& cluster : clusters) {
            int plane = cluster->plane-1;
            double pos = 0.;
            if (plane == 0) pos = cluster->pos.x();
            else pos = cluster->pos.y();
            
            hCluster_TimeVsStrip[plane]->Fill(cluster->t_avg, pos);
    }

        for (const auto& hit : hits) {
            int plane = hit->plane-1;
            hClusterHits_TimeVsStrip[plane]->Fill(hit->t, hit->strip);
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
