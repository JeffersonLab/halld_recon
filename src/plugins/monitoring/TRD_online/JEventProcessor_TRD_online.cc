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
#include <DAQ/Df125FDCPulse.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TProfile.h>

// root hist pointers
// fix constants for now
static const int NTRDplanes = 2;
static const int NTRD_xstrips = 720;
static const int NTRD_ystrips = 432;
static TH1I *trd_num_events;

static TH1I *hDigiHit_NHits;
static TH1I *hDigiHit_NPKs;
static TH1I *hDigiHit_QualityFactor[NTRDplanes];
static TH1I *hDigiHit_Occupancy[NTRDplanes];
static TH2I *hDigiHit_PeakVsStrip[NTRDplanes];
static TH1I *hDigiHit_PulseTime[NTRDplanes];
static TH1I *hDigiHit_Time[NTRDplanes];
static TH2I *hDigiHit_TimeVsStrip[NTRDplanes];
static TH2I *hDigiHit_TimeVsPeak[NTRDplanes];

static TH1I *hHit_NHits;
static TH1I *hHit_NPKs;
static TH1I *hHit_NHits_[NTRDplanes];
static TH1I *hHit_Occupancy[NTRDplanes];
static TH1I *hHit_Time[NTRDplanes];
static TH1I *hHit_PulseHeight[NTRDplanes];
static TH2I *hHit_TimeVsStrip[NTRDplanes];
static TH2I *hHit_TimeVsdE[NTRDplanes];
static TH2I *hHit_StripVsdE[NTRDplanes];

const int NEventsClusterMonitor = 10;
static TH2I *hClusterHits_TimeVsStrip[NTRDplanes];
static TH2I *hCluster_TimeVsStrip[NTRDplanes];
static TH2I *hClusterHits_TimeVsStripEvent[NTRDplanes][NEventsClusterMonitor];
static TH2I *hCluster_TimeVsStripEvent[NTRDplanes][NEventsClusterMonitor];
static TH2I *hDigiHit_TimeVsStripEvent[NTRDplanes][NEventsClusterMonitor];

static TH3I *hPoint_XYT;
static TH1I *hPoint_NHits;
static TH1I *hPoint_Time;
static TH1I *hPoint_dE;
static TH1I *hPoint_dEDiff;
static TH1I *hPoint_dERatio;
static TH1I *hPoint_TimeDiff;
static TH1I *hPoint_OccupancyX;
static TH1I *hPoint_OccupancyY;
static TH2I *hPoint_TimeVsX;
static TH2I *hPoint_TimeVsY;
static TH2I *hPoint_XYDisplay;
static TH2I *hPoint_dE_XY;
static TH2I *hPoint_TimeVsdEX;
static TH2I *hPoint_TimeVsdEY;

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
    trd_num_events = new TH1I("trd_num_events","TRD Number of Events",1,0.5,1.5);

    // digihit-level hists
    trdDir->cd();
    gDirectory->mkdir("DigiHit")->cd();
    hDigiHit_NHits = new TH1I("DigiHit_NHits","TRD fADC Hit Multiplicity;Raw Hits;Events",5000,0.5,0.5+5000);
	hDigiHit_NPKs = new TH1I("DigiHit_NPKs","TRD fADC Peak Multiplicity;Raw Peaks;Events",15,0.5,0.5+15);
	
    // histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
		int NTRDstrips = 0.;
		if(i==0)
			NTRDstrips = NTRD_xstrips;
		else
			NTRDstrips = NTRD_ystrips;

		hDigiHit_Occupancy[i] = new TH1I(Form("DigiHit_Occupancy_Plane%d",i),Form("TRD Plane %d Hit Occupancy;Strip;Raw Hits / Counter",i),NTRDstrips,-0.5,-0.5+NTRDstrips);
		hDigiHit_QualityFactor[i] = new TH1I(Form("DigiHit_QualityFactor_Plane%d",i),Form("TRD Plane %d Quality Factor;Quality Factor;Raw Hits",i),4,-0.5,3.5);
		hDigiHit_PeakVsStrip[i] = new TH2I(Form("DigiHit_PeakVsStrip_Plane%d",i),Form("TRD Plane %d Pulse Height vs. Strip;Strip;Pulse Height [fADC units]",i),NTRDstrips,-0.5,-0.5+NTRDstrips,410,0.0,4100.0);
		hDigiHit_PulseTime[i] = new TH1I(Form("DigiHit_PulseTime_Plane%d",i),Form("TRD Plane %d Pulse Time;Pulse Time [62.5 ps];Raw Hits",i),200,0.0,2000.0);
		hDigiHit_Time[i] = new TH1I(Form("DigiHit_Time_Plane%d",i),Form("TRD Plane %d Peak Time;8*(Peak Time) [ns];Raw Hits / 2 ns",i),250,0.0,2000.0);
		hDigiHit_TimeVsStrip[i] = new TH2I(Form("DigiHit_TimeVsStrip_Plane%d",i),Form("TRD Plane %d Time vs. Strip;Strip;8*(Peak Time) [ns]",i),NTRDstrips,-0.5,-0.5+NTRDstrips,250,0.0,2000.0);
		hDigiHit_TimeVsPeak[i] = new TH2I(Form("DigiHit_TimeVsPeak_Plane%d",i),Form("TRD Plane %d Time vs. Pulse Height;Pulse Height [fADC units];8*(Peak Time) [ns]",i),410,0.0,4100.0,250,0.0,2000.0);
		
	}
		
    // hit-level hists
    trdDir->cd();
    gDirectory->mkdir("Hit")->cd();
    hHit_NHits = new TH1I("Hit_NHits","TRD Calibrated Hit Multiplicity;Calibrated Hits;Events",250,0.5,0.5+250);
    hHit_NPKs = new TH1I("Hit_NPKs","TRD fADC Peak Multiplicity for Stored Hits;Raw Peaks;Events",15,0.5,0.5+15);
	// histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
		int NTRDstrips = 0.;
		if(i==0)
			NTRDstrips = NTRD_xstrips;
		else
			NTRDstrips = NTRD_ystrips;
		
		hHit_NHits_[i] = new TH1I(Form("Hit_NHits_Plane%d",i),Form("TRD Plane %d Calibrated Hit Multiplicity;Calibrated Hits;Events",i),250,0.5,0.5+250);
		hHit_Occupancy[i] = new TH1I(Form("Hit_Occupancy_Plane%d",i),Form("TRD Plane %d Hit Occupancy;Strip;Calibrated Hits / Counter",i),NTRDstrips,-0.5,-0.5+NTRDstrips);
		hHit_Time[i] = new TH1I(Form("Hit_Time_Plane%d",i),Form("TRD Plane %d Time;8*(Peak Time) [ns];Calibrated Hits / 2 ns",i),250,0.0,2000.0);
		hHit_PulseHeight[i] = new TH1I(Form("Hit_PulseHeight_Plane%d",i),Form("TRD Plane %d Pulse Height;Pulse Height [fADC units];Calibrated Hits / 1 unit",i),450,0.0,3000.0);
        hHit_TimeVsStrip[i] = new TH2I(Form("Hit_TimeVsStrip_Plane%d",i),Form("TRD Plane %d Time vs. Strip;Strip;8*(Peak Time) [ns]",i),NTRDstrips,-0.5,-0.5+NTRDstrips,250,0.0,2000.0);
		hHit_StripVsdE[i] = new TH2I(Form("Hit_StripVsdE_Plane%d",i),Form("TRD Plane %d Hit Charge vs. Strip;Strip;dE [q]",i),NTRDstrips,-0.5,-0.5+NTRDstrips,450,0.,3000.0);
		hHit_TimeVsdE[i] = new TH2I(Form("Hit_TimeVsdE_Plane%d",i),Form("TRD Plane %d Hit Charge vs. Time;dE [q];8*(Peak Time) [ns]",i),450,0.,3000.,250,0.0,2000.0);
		
	}
    
	// point-level hists
    trdDir->cd();
    gDirectory->mkdir("Point")->cd();
	hPoint_NHits = new TH1I("Point_NHits","TRD Calibrated Point Multiplicity;Calibrated Points;Events",250,0.5,0.5+250);
    hPoint_XYT = new TH3I("Point_XYT","TRD 3D Points;X [cm];Y [cm];8*(Peak Time) [ns]",800,-89.5,-9.5,400,-69.5,-29.5,200,0.,200.*8.);
	hPoint_Time = new TH1I("Point_Time","TRD Point Time;8*(Peak Time) [ns]; ",200,0.,200.*8.);
	hPoint_dE = new TH1I("Point_dE","TRD Point Charge;Average dE [q]; ",450,0.,3000.);
	hPoint_dEDiff = new TH1I("Point_dEDiff","TRD Point Charge X,Y Weighted Diff.;(dE_x - dE_y)/(dE_x + dE_y); ",100,-1.,1.);
	hPoint_dERatio = new TH1I("Point_dERatio","TRD Point dE_x / dE_y;(dE_x / dE_y); ",100,-0.,6.);
	hPoint_dE_XY = new TH2I("Point_dE_XY","TRD Point Charge Corr. in X,Y;X Strip dE [q]; Y Strip dE [q]",450,0.,3000.,450,0.,3000.);
	hPoint_TimeVsdEX= new TH2I("Point_TimeVsdEX","TRD Point Charge of X in Time;X Strip dE [q];8*(Peak Time) [ns]",450,0.,3000.,200,0.,200.*8.);
	hPoint_TimeVsdEY= new TH2I("Point_TimeVsdEY","TRD Point Charge of Y in Time;Y Strip dE [q];8*(Peak Time) [ns]",450,0.,3000.,200,0.,200.*8.);
	hPoint_OccupancyX = new TH1I("Point_OccupancyX","TRD Point X;X [cm]; ",800,-89.5,-9.5);
	hPoint_OccupancyY = new TH1I("Point_OccupancyY","TRD Point Y;Y [cm]; ",400,-69.5,-29.5);
	hPoint_XYDisplay = new TH2I("Point_XYDisplay","TRD 2D Point Display;X [cm];Y [cm]",800,-89.5,-9.5,400,-69.5,-29.5);
	hPoint_TimeVsX= new TH2I("Point_TimeVsX","TRD Point X in Time;X [cm];8*(Peak Time) [ns]",800,-89.5,-9.5,200,0.,200.*8.);
	hPoint_TimeVsY= new TH2I("Point_TimeVsY","TRD Point Y in Time;Y [cm];8*(Peak Time) [ns]",400,-69.5,-29.5,200,0.,200.*8.);
	hPoint_TimeDiff = new TH1I("Point_TimeDiff","TRD Point Time Difference;abs(X Time - Y Time) [ns]; ",80,-40.,40.);
	
    trdDir->cd();
	
	// cluster-level hists
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

            hClusterHits_TimeVsStrip[i] = new TH2I(Form("ClusterHits_TimeVsStrip_Plane%d",i),Form("TRD Plane %d Cluster Hits Time vs. Strip;8*(Peak Time) [ns];Strip",i),250,0,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
            hCluster_TimeVsStrip[i] = new TH2I(Form("Cluster_TimeVsStrip_Plane%d",i),Form("TRD Plane %d Cluster Time vs. Strip;8*(Peak Time) [ns];Strip",i),250,0.,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
        for(int j=0; j<NEventsClusterMonitor; j++) {
            hClusterHits_TimeVsStripEvent[i][j] = new TH2I(Form("ClusterHits_TimeVsStrip_Plane%d_Event%d",i,j),Form("TRD Plane %d Cluster Hits Time vs. Strip;8*(Peak Time) [ns];Strip",i),250,0,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
            hCluster_TimeVsStripEvent[i][j] = new TH2I(Form("Cluster_TimeVsStrip_Plane%d_Event%d",i,j),Form("TRD Plane %d Cluster Time vs. Strip;8*(Peak Time) [ns];Strip",i),250,0.,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
			hDigiHit_TimeVsStripEvent[i][j] = new TH2I(Form("DigiHit_TimeVsStrip_Plane%d_Event%d",i,j),Form("TRD Plane %d DigiHit Time vs. Strip;8*(Peak Time) [ns];Strip",i),250,0.,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
        }
    }
    
    // back to main dir
    mainDir->cd();
}

//----------------------------------------------------------------------------------


void JEventProcessor_TRD_online::BeginRun(const std::shared_ptr<const JEvent>& event) {
    // This is called whenever the run number changes

    //auto runnumber = event->GetRunNumber();
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
	
	auto eventnumber = event->GetEventNumber();
	
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
    event->Get(digihits);
    vector<const DTRDHit*> hits;
    event->Get(hits);
    vector<const DTRDStripCluster*> clusters;
    event->Get(clusters);
	vector<const DTRDPoint*> points;
    event->Get(points,"Hit");
	
    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    ///////////////////////////
    //    TRD DigiHits       //
    ///////////////////////////

    if (digihits.size() > 0) trd_num_events->Fill(1);
    hDigiHit_NHits->Fill(digihits.size());
	for (const auto& hit : digihits) {
	    int plane = hit->plane-1;  // plane and strip counting starts from 1
	    int strip = hit->strip;
	    hDigiHit_NPKs->Fill(hit->NPK);
	    hDigiHit_Occupancy[plane]->Fill(strip);
	    hDigiHit_QualityFactor[plane]->Fill(hit->QF);
	    hDigiHit_PeakVsStrip[plane]->Fill(strip,hit->pulse_peak);
	    hDigiHit_PulseTime[plane]->Fill(hit->pulse_time);
	    double t_ns = 8.*hit->peak_time;
	    hDigiHit_Time[plane]->Fill(t_ns);
	    hDigiHit_TimeVsStrip[plane]->Fill(strip,t_ns);
	    hDigiHit_TimeVsPeak[plane]->Fill(hit->pulse_peak,t_ns);
    }

	///////////////////////////
    //        TRD Hits       //
    ///////////////////////////
	
    hHit_NHits->Fill(hits.size());
    for (const auto& hit : hits) {
	    int plane = hit->plane-1;
		if (plane == 0) hHit_NHits_[plane]->Fill(hits.size());
		else if (plane == 1) hHit_NHits_[plane]->Fill(hits.size());
	    hHit_Occupancy[plane]->Fill(hit->strip);
	    hHit_Time[plane]->Fill(hit->t);
	    hHit_PulseHeight[plane]->Fill(hit->pulse_height);
        hHit_TimeVsStrip[plane]->Fill(hit->strip, hit->t);
		hHit_TimeVsdE[plane]->Fill(hit->q, hit->t);
		hHit_StripVsdE[plane]->Fill(hit->strip, hit->q);
		
		const DTRDDigiHit* locAssociatedDigiHit = NULL;
        hit->GetSingle(locAssociatedDigiHit);
        if(!locAssociatedDigiHit) continue;
        int NPK = locAssociatedDigiHit->NPK;
		hHit_NPKs->Fill(NPK);
    }
	
	///////////////////////////
    //    TRD Clusters       //
    ///////////////////////////
	
    if (clusters.size() > 2 && eventClusterCount < NEventsClusterMonitor) {
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
		
		for (const auto& hit : digihits) {
            int plane = hit->plane-1;
            hDigiHit_TimeVsStripEvent[plane][eventClusterCount]->Fill(8.*hit->peak_time, hit->strip);
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
		
	///////////////////////////
    //      TRD Points       //
    ///////////////////////////
		
	hPoint_NHits->Fill(points.size());
    for (const auto& point : points) {
        hPoint_XYT->Fill(point->x,point->y,point->time);
	    hPoint_Time->Fill(point->time);
  	    hPoint_TimeDiff->Fill(abs(point->t_x - point->t_y));
        hPoint_OccupancyX->Fill(point->x);
	    hPoint_OccupancyY->Fill(point->y);
    	hPoint_dE->Fill(point->dE);
        hPoint_dE_XY->Fill(point->dE_x,point->dE_y);
	    hPoint_TimeVsdEX->Fill(point->dE_x,point->t_x);
    	hPoint_TimeVsdEY->Fill(point->dE_y,point->t_y);
        hPoint_dEDiff->Fill((point->dE_x - point->dE_y)/(point->dE_x + point->dE_y));
	    hPoint_dERatio->Fill((point->dE_x / point->dE_y));
    	hPoint_TimeVsX->Fill(point->x,point->time);
        hPoint_TimeVsY->Fill(point->y,point->time);
        hPoint_XYDisplay->Fill(point->x,point->y);
    }
	
    lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
	
    return;
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
