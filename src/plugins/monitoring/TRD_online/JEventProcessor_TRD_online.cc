#include <stdint.h>
#include <vector>

#include "JEventProcessor_TRD_online.h"

using namespace std;

#include <DANA/DEvent.h>

//#include <TRIGGER/DL1Trigger.h>
#include <TRD/DTRDDigiHit.h>
#include <TRD/DTRDHit.h>
#include <TRD/DTRDStripCluster.h>
#include <TRD/DTRDPoint.h>
#include <TRD/DTRDSegment.h>
#include <DAQ/Df125FDCPulse.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TProfile.h>
#include <fstream>
#include <iostream>

//#define USE_TRACKING
#ifdef USE_TRACKING 
	#include <PID/DChargedTrack.h>
#endif


// root hist pointers
// fix constants for now
static const int NTRDplanes = 2;
static const int NTRD_xstrips = 720;
static const int NTRD_ystrips = 432; //264 for Spring 2026 Low Energy Period !! These need set more sensibly
static TH1I *trd_num_events;
static ofstream outputTextFile;
static ofstream outputTextFile2;

static TH1I *hDigiHit_NHits;
static TH1I *hDigiHit_NPKs;
static TH1I *hDigiHit_QualityFactor[NTRDplanes];
static TH1D *hDigiHit_Occupancy[NTRDplanes];
static TH2D *hDigiHit_PeakVsStrip[NTRDplanes];
static TH1D *hDigiHit_PulseTime[NTRDplanes];
static TH1D *hDigiHit_Time[NTRDplanes];
static TH2D *hDigiHit_TimeVsStrip[NTRDplanes];
static TH2D *hDigiHit_TimeVsPeak[NTRDplanes];

static TH1I *hHit_NPKs[NTRDplanes];
static TH1I *hHit_NHits[NTRDplanes];
static TH1D *hHit_Occupancy[NTRDplanes];
static TH1D *hHit_Time[NTRDplanes];
static TH1D *hHit_PulseHeight[NTRDplanes];
static TH2D *hHit_TimeVsStrip[NTRDplanes];
static TH2D *hHit_TimeVsdE[NTRDplanes];
static TH2D *hHit_TimeVsdE_Max[NTRDplanes];
static TH2D *hHit_StripVsdE[NTRDplanes];

const int NEventsMonitor = 10; //--Change this for event-by-event viewing histograms
const int NMaxExtrapolations = 5;
const int NMaxSegments = 5;

static TH1I *hCluster_NClusters;
static TH2I *hCluster_NClustersPerTrack;
static TH1D *hCluster_pos_width;
static TH1D *hCluster_time_width;
static TH2D *hClusterHits_TimeVsPos[NTRDplanes];
static TH2D *hCluster_TimeVsPos[NTRDplanes];
static TH2D *hClusterHits_TimeVsStrip[NTRDplanes];
static TH2D *hCluster_TimeVsdE_Max[NTRDplanes];

static TH2D *hClusterHits_TimeVsPosEvent[NTRDplanes][NEventsMonitor];
static TH2D *hCluster_TimeVsPosEvent[NTRDplanes][NEventsMonitor];
static TH2D *hDigiHit_TimeVsStripEvent[NTRDplanes][NEventsMonitor];
static TH2D *hPoint_TimeVsPosEvent[NTRDplanes][NEventsMonitor];
static TH2D *hPointH_TimeVsPosEvent[NTRDplanes][NEventsMonitor];

static TH1D *hSegment_Members_Event[NEventsMonitor][NMaxSegments];
static TH1D *hExtrapolation_Members_Event[NEventsMonitor][NMaxExtrapolations];
static TH2D *hPoint_ZVsX_Event[NEventsMonitor];
static TH2D *hPoint_ZVsY_Event[NEventsMonitor];

static TH3D *hPoint_XYT;
static TH1I *hPoint_NHits;
static TH1D *hPoint_Time;
static TH1D *hPoint_TimeDiff;
static TH1D *hPoint_dE;
static TH1D *hPoint_dEDiff;
static TH1D *hPoint_dERatio;
static TH1D *hPoint_OccupancyX;
static TH1D *hPoint_OccupancyY;
static TH2D *hPoint_TimeVsX;
static TH2D *hPoint_TimeVsY;
static TH2D *hPoint_XYDisplay;
static TH2D *hPoint_ZXDisplay;
static TH2D *hPoint_ZYDisplay;
static TH2D *hPoint_dE_XY;
static TH2D *hPoint_TimeVsdEX;
static TH2D *hPoint_TimeVsdEY;
static TH2D *hPoint_TimeVsdE_Max;

static TH3D *hPointH_XYT;
static TH1I *hPointH_NHits;
static TH1D *hPointH_Time;
static TH1D *hPointH_TimeDiff;
static TH1D *hPointH_dE;
static TH1D *hPointH_dEDiff;
static TH1D *hPointH_dERatio;
static TH1D *hPointH_OccupancyX;
static TH1D *hPointH_OccupancyY;
static TH2D *hPointH_TimeVsX;
static TH2D *hPointH_TimeVsY;
static TH2D *hPointH_XYDisplay;
static TH2D *hPointH_ZXDisplay;
static TH2D *hPointH_ZYDisplay;
static TH2D *hPointH_dE_XY;
static TH2D *hPointH_TimeVsdEX;
static TH2D *hPointH_TimeVsdEY;
static TH2D *hPointH_TimeVsdE_Max;
static TH2D *hPointH_TimeVsdE_el_Max;
static TH2D *hPointH_TimeVsdE_pi_Max;

static TH1I *hSegment_NHits;
static TH1D *hSegment_TX;
static TH1D *hSegment_TY;
static TH1D *hSegment_OccupancyX;
static TH1D *hSegment_OccupancyY;

bool useNizarTrackMatching;
bool verboseNizarTrackMatching;

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
	
	
#ifdef USE_TRACKING
	useNizarTrackMatching = true;
	verboseNizarTrackMatching = true;
#else
	useNizarTrackMatching = false;
    verboseNizarTrackMatching = false;
#endif
	
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();
	
    // create root folder for TRD and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *trdDir = gDirectory->mkdir("TRD");
    trdDir->cd();
	
	///////////////////////////////
    //   Book Histos 			//
    /////////////////////////////
	
    trd_num_events = new TH1I("trd_num_events","TRD Number of Events",1,0.5,1.5);
	outputTextFile.open("TRDOnline_pointHits_electron_candidate_list.txt");
	outputTextFile2.open("TRDOnline_points_electron_candidate_list.txt");
	
    // digihit-level hists
    trdDir->cd();
    gDirectory->mkdir("DigiHit")->cd();
    hDigiHit_NHits = new TH1I("DigiHit_NHits","TRD fADC Hit Multiplicity;Raw Hits;Events",5000,0.5,0.5+5000);
	hDigiHit_NPKs = new TH1I("DigiHit_NPKs","TRD fADC Peak Multiplicity;Raw Peaks;Events",15,0.5,0.5+15);
	
    // histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
		int NTRDstrips = 0.;
		if(i==0) NTRDstrips = NTRD_xstrips;
		else NTRDstrips = NTRD_ystrips;
		
		hDigiHit_Occupancy[i] = new TH1D(Form("DigiHit_Occupancy_Plane%d",i),Form("TRD Plane %d Hit Occupancy;Strip;Raw Hits / Counter",i),NTRDstrips,0.5,0.5+NTRDstrips);
		hDigiHit_QualityFactor[i] = new TH1I(Form("DigiHit_QualityFactor_Plane%d",i),Form("TRD Plane %d Quality Factor;Quality Factor;Raw Hits",i),4,-0.5,3.5);
		hDigiHit_PeakVsStrip[i] = new TH2D(Form("DigiHit_PeakVsStrip_Plane%d",i),Form("TRD Plane %d Pulse Peak vs. Strip;Strip;Pulse Peak [fADC units]",i),NTRDstrips,0.5,0.5+NTRDstrips,410,0.0,4100.0);
		hDigiHit_PulseTime[i] = new TH1D(Form("DigiHit_PulseTime_Plane%d",i),Form("TRD Plane %d Pulse Time;Pulse Time [62.5 ps];Raw Hits",i),180,0.0,1800.0);
		hDigiHit_Time[i] = new TH1D(Form("DigiHit_Time_Plane%d",i),Form("TRD Plane %d Peak Time;8*(Peak Time) [ns];Raw Hits / 2 ns",i),225,0.0,1800.0);
		hDigiHit_TimeVsStrip[i] = new TH2D(Form("DigiHit_TimeVsStrip_Plane%d",i),Form("TRD Plane %d Time vs. Strip;Strip;8*(Peak Time) [ns]",i),NTRDstrips,0.5,0.5+NTRDstrips,225,0.0,1800.0);
		hDigiHit_TimeVsPeak[i] = new TH2D(Form("DigiHit_TimeVsPeak_Plane%d",i),Form("TRD Plane %d Time vs. Pulse Peak;Pulse Peak [fADC units];8*(Peak Time) [ns]",i),410,0.0,4100.0,225,0.0,1800.0);
	}
	
    // hit-level hists
    trdDir->cd();
    gDirectory->mkdir("Hit")->cd();
	// histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
		int NTRDstrips = 0.;
		if(i==0) NTRDstrips = NTRD_xstrips;
		else NTRDstrips = NTRD_ystrips;
		
		hHit_NHits[i] = new TH1I(Form("Hit_NHits_Plane%d",i),Form("TRD Plane %d Calibrated Hit Multiplicity;Calibrated Hits;Events",i),250,0.5,0.5+250);
		hHit_NPKs[i] = new TH1I(Form("Hit_NPKs_Plane%d",i),Form("TRD Plane %d fADC Peak Multiplicity for Stored Hits;Raw Peaks;Events",i),15,0.5,0.5+15);
		hHit_Occupancy[i] = new TH1D(Form("Hit_Occupancy_Plane%d",i),Form("TRD Plane %d Hit Occupancy;Strip;Calibrated Hits / Counter",i),NTRDstrips,0.5,0.5+NTRDstrips);
		hHit_Time[i] = new TH1D(Form("Hit_Time_Plane%d",i),Form("TRD Plane %d Time;8*(Peak Time) [ns];Calibrated Hits / 2 ns",i),225,0.0,1800.0);
		hHit_PulseHeight[i] = new TH1D(Form("Hit_PulseHeight_Plane%d",i),Form("TRD Plane %d Pulse Height;(Pulse Peak - Ped)[fADC units];Calibrated Hits / 1 unit",i),175,0.0,3500.0);
        hHit_TimeVsStrip[i] = new TH2D(Form("Hit_TimeVsStrip_Plane%d",i),Form("TRD Plane %d Time vs. Strip;Strip;8*(Peak Time) [ns]",i),NTRDstrips,0.5,0.5+NTRDstrips,225,0.0,1800.0);
		hHit_StripVsdE[i] = new TH2D(Form("Hit_StripVsdE_Plane%d",i),Form("TRD Plane %d Hit Charge vs. Strip;Strip;dE [q]",i),NTRDstrips,0.5,0.5+NTRDstrips,175,0.,3500.0);
		hHit_TimeVsdE[i] = new TH2D(Form("Hit_TimeVsdE_Plane%d",i),Form("TRD Plane %d Hit Charge vs. Time;dE [q];8*(Peak Time) [ns]",i),175,0.,3500.,250,0.0,2000.0);
		hHit_TimeVsdE_Max[i] = new TH2D(Form("Hit_TimeVsdE_Max_Plane%d",i),Form("TRD Plane %d Max Hit Charge vs. Time;8*(Peak Time) [ns];Max dE [q]",i),250,0.,2000.,175,0.,3500.);
		
	}
    
	// point-level hists
    trdDir->cd();
	// point based on hits
	gDirectory->mkdir("Point_Hit")->cd();
    hPointH_NHits = new TH1I("PointH_NHits","TRD Calibrated Point_Hit Multiplicity;Calibrated Points;Events",100,0.5,0.5+100);
    hPointH_XYT = new TH3D("PointH_XYT","TRD 3D Point_Hits;X [cm];Y [cm];8*(Peak Time) [ns]",450,-90.,0.,450,-90.,0.,250,0.,2000.);
    hPointH_Time = new TH1D("PointH_Time","TRD Point_Hit Time;8*(Peak Time) [ns]",250,0.,2000.);
	hPointH_TimeDiff = new TH1D("PointH_TimeDiff","TRD Point_Hit xTime - yTime; xTime - yTime [ns]",55,-22.,22.);
    hPointH_dE = new TH1D("PointH_dE","TRD Point_Hit Charge;Average dE [q]",175,0.,3500.);
    hPointH_dEDiff = new TH1D("PointH_dEDiff","TRD Point_Hit Charge X,Y Weighted Diff.;(dE_x - dE_y)/(dE_x + dE_y)",100,-1.,1.);
    hPointH_dERatio = new TH1D("PointH_dERatio","TRD Point_Hit dE_x / dE_y;(dE_x / dE_y)",100,-0.,6.);
    hPointH_dE_XY = new TH2D("PointH_dE_XY","TRD Point_Hit Charge Corr. in X,Y;X Strip dE [q]; Y Strip dE [q]",175,0.,3500.,175,0.,3500.);
    hPointH_TimeVsdEX= new TH2D("PointH_TimeVsdEX","TRD Point_Hit Charge of X in Time;X Strip dE [q];8*(Peak Time) [ns]",175,0.,3500.,250,0.,2000.);
    hPointH_TimeVsdEY= new TH2D("PointH_TimeVsdEY","TRD Point_Hit Charge of Y in Time;Y Strip dE [q];8*(Peak Time) [ns]",175,0.,3500.,250,0.,2000.);
	hPointH_TimeVsdE_Max= new TH2D("PointH_TimeVsdE_Max","TRD Max Point_Hit Charge in Time;8*(Peak Time) [ns];Max dE [q]",250,0.,2000.,175,0.,3500.);
	hPointH_TimeVsdE_el_Max= new TH2D("PointH_TimeVsdE_el_Max","TRD Max Point_Hit Charge in Time (El Cut Passed);8*(Peak Time) [ns];Max dE [q]",250,0.,2000.,175,0.,3500.);
	hPointH_TimeVsdE_pi_Max= new TH2D("PointH_TimeVsdE_pi_Max","TRD Max Point_Hit Charge in Time (El Cut Failed);8*(Peak Time) [ns];Max dE [q]",250,0.,2000.,175,0.,3500.);
    hPointH_OccupancyX = new TH1D("PointH_OccupancyX","TRD Point_Hit X;X [cm]",450,-90.,0.);
    hPointH_OccupancyY = new TH1D("PointH_OccupancyY","TRD Point_Hit Y;Y [cm]",450,-90,0.);
    hPointH_XYDisplay = new TH2D("PointH_XYDisplay","TRD XY 2D Point_Hit Display ;X [cm];Y [cm]",450,-90.,0.,450,-90.,0.);
    hPointH_ZXDisplay = new TH2D("PointH_ZXDisplay","TRD XZ 2D Point_Hit Display;X [cm];Z [cm]",450,-90.,0.,480,527.5,533.5);
    hPointH_ZYDisplay = new TH2D("PointH_ZYDisplay","TRD YZ 2D Point_Hit Display;Y [cm];Z [cm]",450,-90.,0.,480,527.5,533.5);
    hPointH_TimeVsX= new TH2D("PointH_TimeVsX","TRD Point_Hit X in Time;X [cm];8*(Peak Time) [ns]",450,-90.,0.,250,0.,2000.);
    hPointH_TimeVsY= new TH2D("PointH_TimeVsY","TRD Point_Hit Y in Time;Y [cm];8*(Peak Time) [ns]",450,-90.,0.,250,0.,2000.);
	
	// point based on clusters
    trdDir->cd();
    gDirectory->mkdir("Point")->cd();
    hPoint_NHits = new TH1I("Point_NHits","TRD Calibrated Point Multiplicity;Calibrated Points;Events",100,0.5,0.5+100);
    hPoint_XYT = new TH3D("Point_XYT","TRD 3D Points;X [cm];Y [cm];8*(Peak Time) [ns]",225,-90.,0.,225,-90.,0.,250,0.,2000.);
    hPoint_Time = new TH1D("Point_Time","TRD Point Time;8*(Peak Time) [ns]; ",225,0.,1800.);
    hPoint_TimeDiff = new TH1D("Point_TimeDiff","TRD Point xTime - yTime; xTime - yTime [ns]",55,-22.,22.);
	hPoint_dE = new TH1D("Point_dE","TRD Point Charge;Average dE [q]; ",250,0.,10000.);
    hPoint_dEDiff = new TH1D("Point_dEDiff","TRD Point Charge X,Y Weighted Diff.;(dE_x - dE_y)/(dE_x + dE_y); ",100,-1.,1.);
    hPoint_dERatio = new TH1D("Point_dERatio","TRD Point dE_x / dE_y;(dE_x / dE_y); ",100,-0.,6.);
    hPoint_dE_XY = new TH2D("Point_dE_XY","TRD Point Charge Corr. in X,Y;X Strip dE [q]; Y Strip dE [q]",175,0.,3500.,175,0.,3500.);
    hPoint_TimeVsdEX= new TH2D("Point_TimeVsdEX","TRD Point Charge of X in Time;X Strip dE [q];8*(Peak Time) [ns]",250,0.,10000.,250,0.,2000.);
    hPoint_TimeVsdEY= new TH2D("Point_TimeVsdEY","TRD Point Charge of Y in Time;Y Strip dE [q];8*(Peak Time) [ns]",250,0.,10000.,250,0.,2000.);
	hPoint_TimeVsdE_Max= new TH2D("Point_TimeVsdE_Max","TRD Max Point Charge in Time;8*(Peak Time) [ns];Max dE [q]",250,0.,2000,250,0.,10000.);
    hPoint_OccupancyX = new TH1D("Point_OccupancyX","TRD Point X;X [cm]; ",450,-90.,0.);
    hPoint_OccupancyY = new TH1D("Point_OccupancyY","TRD Point Y;Y [cm]; ",450,-90.,0.);
    hPoint_XYDisplay = new TH2D("Point_XYDisplay","TRD XY 2D Point Display ;X [cm];Y [cm]",450,-90.,0.,450,-90.,0.);
    hPoint_ZXDisplay = new TH2D("Point_ZXDisplay","TRD XZ 2D Point Display;X [cm];Z [cm]",450,-90.,0.,480,527.5,533.5);
    hPoint_ZYDisplay = new TH2D("Point_ZYDisplay","TRD YZ 2D Point Display;Y [cm];Z [cm]",450,-90.,0.,480,527.5,533.5);
    hPoint_TimeVsX= new TH2D("Point_TimeVsX","TRD Point X in Time;X [cm];8*(Peak Time) [ns]",450,-90.,0.,250,0.,2000.);
    hPoint_TimeVsY= new TH2D("Point_TimeVsY","TRD Point Y in Time;Y [cm];8*(Peak Time) [ns]",450,-90.,0.,250,0.,2000.);

    trdDir->cd();
	// cluster-level hists
    eventClusterCount = 0;
	
    gDirectory->mkdir("Cluster")->cd();
    hCluster_NClusters = new TH1I("Cluster_NClusters","TRD Number of Clusters per Event;N Clusters;Events",75,0.5,0.5+75);
	hCluster_NClustersPerTrack = new TH2I("Cluster_NClustersPerTrack","TRD Number of Clusters per Track;N Clusters;N Tracks",75,0.5,0.5+75,12,0.5,12.5);
    hCluster_pos_width = new TH1D("Cluster_pos_width","TRD Cluster Position Width;Position Width [cm];Events",100,0.,50.);
    hCluster_time_width = new TH1D("Cluster_time_width","TRD Cluster Time Width;Time Width [ns];Events",100,0.,50.0);

    // histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
        int NTRDstrips = 0.;
        if(i==0)
	{    NTRDstrips = NTRD_xstrips;}
        else
	{    NTRDstrips = NTRD_ystrips;}
			
		hClusterHits_TimeVsPos[i] = new TH2D(Form("ClusterHits_TimeVsPos_Plane%d",i),Form("TRD Cluster-Associated Hits Time vs. Pos, Plane %d;8*(Peak Time) [ns];Position [cm]",i),250,0,2000.0,450,-90.,0.);
        hCluster_TimeVsPos[i] = new TH2D(Form("Cluster_TimeVsPos_Plane%d",i),Form("TRD Cluster Time vs. Pos, Plane %d;8*(Peak Time) [ns];Position [cm]",i),250,0.,2000.0,450,-90.,0.);
		hClusterHits_TimeVsStrip[i] = new TH2D(Form("ClusterHits_TimeVsStrip_Plane%d",i),Form("TRD Cluster-Associated Hits Time vs. Strip, Plane %d;8*(Peak Time) [ns];Strip Number",i),250,0,2000.0,NTRDstrips,0.5,0.5+NTRDstrips);
		hCluster_TimeVsdE_Max[i] = new TH2D(Form("Cluster_TimeVsdE_Max_Plane%d",i),Form("TRD Max Cluster Charge vs. Time, Plane %d;8*(Peak Time) [ns];Cluster Max dE [q]",i),250,0.,2000.,175,0.,10000.);
		
    }
    
	trdDir->cd();
    gDirectory->mkdir("Segment")->cd();
    hSegment_NHits = new TH1I("Segment_NHits","TRD Track Segment Multiplicity;Calibrated Track Segments;Events",20,-0.5,20-0.5);
    hSegment_TX = new TH1D("Segment_TX","TRD Track Segment Tx;Tx; ",80,-40.,40.);
    hSegment_TY = new TH1D("Segment_TY","TRD Track Segment Ty;Ty; ",80,-40.,40.);
    hSegment_OccupancyX = new TH1D("Segment_OccupancyX","TRD Track Segment X Occupancy;X [cm]; ",450,-90.,0.);
    hSegment_OccupancyY = new TH1D("Segment_OccupancyY","TRD Track Segment Y Occupancy;Y [cm]; ",450,-90.,0.);
	
	// event-by-event histos
	trdDir->cd();
    TDirectory *eventDir = gDirectory->mkdir("EventMonitor");
	eventDir->cd();
	for(int j=0; j<NEventsMonitor; j++) {
		//sub-folder for each event
		 gDirectory->mkdir(Form("Event_%d",j))->cd();
		
		// histograms for each plane
    	for(int i=0; i<NTRDplanes; i++) {
        	int NTRDstrips = 0.;
        	if(i==0) { NTRDstrips = NTRD_xstrips; }
        	else { NTRDstrips = NTRD_ystrips; }
            hDigiHit_TimeVsStripEvent[i][j] = new TH2D(Form("DigiHit_TimeVsStrip_Plane%d_Event%d",i,j),Form("TRD DigiHit Time vs. Strip, Plane %d;8*(Peak Time) [ns];Strip Number",i),250,0.,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
			hClusterHits_TimeVsPosEvent[i][j] = new TH2D(Form("ClusterHits_TimeVsPos_Plane%d_Event%d",i,j),Form("TRD Cluster-Associated Hits Time vs. Pos, Plane %d;8*(Peak Time) [ns];Position [cm]",i),250,0,2000.0,450,-90.,0.);
            hCluster_TimeVsPosEvent[i][j] = new TH2D(Form("Cluster_TimeVsPos_Plane%d_Event%d",i,j),Form("TRD Cluster Time vs. Pos, Plane %d;8*(Peak Time) [ns];Position [cm]",i),250,0.,2000.0,450,-90.,0.);
            hPointH_TimeVsPosEvent[i][j] = new TH2D(Form("PointH_TimeVsPos_Plane%d_Event%d",i,j),Form("TRD Point_Hit Time vs. Pos, Plane %d;8*(Peak Time) [ns];Position [cm]",i),250,0.,2000.0,450,-90.,0.);
			hPoint_TimeVsPosEvent[i][j] = new TH2D(Form("Point_TimeVsPos_Plane%d_Event%d",i,j),Form("TRD Point Time vs. Pos, Plane %d;8*(Peak Time) [ns];Position [cm]",i),250,0.,2000.0,450,-90.,0.);
		}
		hPoint_ZVsX_Event[j] = new TH2D(Form("Point_ZVsX_Event%d",j),"TRD Point X vs. Z (GlueX Coordinates);Z [cm];X [cm]",100,528,533,450,-90,0);
        hPoint_ZVsY_Event[j] = new TH2D(Form("Point_ZVsY_Event%d",j),"TRD Point Y vs. Z (GlueX Coordinates);Z [cm];Y [cm]",100,528,533,450,-90,0);
		
		eventDir->cd();
	}
	
	if (useNizarTrackMatching) {
	//const double dTRDpos[2] = {-47.4695,-59.0315}; // TRD offsets from geometry //This needs fixed!!! since it has changed by run period. Needs somehow pulled from databases instead of set here
	const TString Segment_Members_Vars[8] = {"x","y","tx","ty","var_x","var_y","var_tx","var_ty"};
    const TString Extrapolation_Members_Vars[8] = {"x","y","dxdz","dydz"};
    for (int i=0; i<NEventsMonitor; i++) {
        for (int j=0; j<NMaxSegments; j++) {
            hSegment_Members_Event[i][j] = new TH1D(Form("Segment_Members_Event%d_Segment%d",i,j),Form("TRD Segment Members Event %d Segment %d",i,j),8,0.,8.);
            for (int k=0; k<8; k++) {
                hSegment_Members_Event[i][j]->GetXaxis()->SetBinLabel(k+1,Segment_Members_Vars[k]);
            }
        }
        for (int j=0; j<NMaxExtrapolations; j++) {
            hExtrapolation_Members_Event[i][j] = new TH1D(Form("Extrapolation_Members_Event%d_Extrapolation%d",i,j),Form("TRD Extrapolation Members Event %d Extrapolation %d",i,j),4,0.,4.);
            for (int k=0; k<4; k++) {
                hExtrapolation_Members_Event[i][j]->GetXaxis()->SetBinLabel(k+1,Extrapolation_Members_Vars[k]);
            }
        }
    }
    NEventsTrack = 0;
    NEventsTrackSegmentMatch = 0;
	
	} //--END if useNizarTrackMatching
	
    // back to main dir
    mainDir->cd();
}

//----------------------------------------------------------------------------------


void JEventProcessor_TRD_online::BeginRun(const std::shared_ptr<const JEvent>& event) {
    // This is called whenever the run number changes
	
    const DGeometry *geom = GetDGeometry(event);
	geom->GetGEMTRDz(dTRDz);
  	vector<double>xvec,yvec;
  	if(geom->GetGEMTRDxy_vec(xvec,yvec)){
    	dTRDx=xvec[0];
    	dTRDy=yvec[0];
  	}
}


//----------------------------------------------------------------------------------


void JEventProcessor_TRD_online::Process(const std::shared_ptr<const JEvent>& event) {
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // loop-Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.
	
	auto eventnumber = event->GetEventNumber();
	auto runnumber = event->GetRunNumber();

    vector<const DTRDDigiHit*> digihits;
    event->Get(digihits);
    vector<const DTRDHit*> hits;
    event->Get(hits);
    vector<const DTRDStripCluster*> clusters;
    event->Get(clusters);
	vector<const DTRDPoint*> points;
    event->Get(points);
	vector<const DTRDPoint*> pointHits;
    event->Get(pointHits,"Hit");
	vector<const DTRDSegment*> segments;
	#ifdef USE_TRACKING
    	event->Get(segments,"Extrapolation");
		vector<const DChargedTrack*> tracks;
    	event->Get(tracks);
	#else
		event->Get(segments);
	#endif
	
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
	
	float hit_maxQ_x = 0., hit_maxQ_y = 0.;
	float hit_maxTime_x = 0., hit_maxTime_y = 0.;
    for (const auto& hit : hits) {
	    int plane = hit->plane-1;
		if (plane == 0) {
			if (hit->q > hit_maxQ_x) {
				hit_maxQ_x = hit->q;
				hit_maxTime_x = hit->t;
			}
		}
		else if (plane == 1) {
			if (hit->q > hit_maxQ_y) {
                hit_maxQ_y = hit->q;
                hit_maxTime_y = hit->t;
            }
		}
		hHit_NHits[plane]->Fill(hits.size());
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
		hHit_NPKs[plane]->Fill(NPK);
    }
	if (hit_maxQ_x > 0.) hHit_TimeVsdE_Max[0]->Fill(hit_maxTime_x, hit_maxQ_x);
	if (hit_maxQ_y > 0.) hHit_TimeVsdE_Max[1]->Fill(hit_maxTime_y, hit_maxQ_y);
	
	///////////////////////////
    //    TRD Clusters       //
    ///////////////////////////
	
    const double STRIP_PITCH=0.1;
    int NCluster_X = 0;
    int NCluster_Y = 0;
	float cluster_maxQ_x = 0., cluster_maxQ_y = 0.; 
    float cluster_maxTime_x = 0., cluster_maxTime_y = 0.;
	
	if (clusters.size() > 0) {
		hCluster_NClusters->Fill(clusters.size());
		#ifdef USE_TRACKING
			hCluster_NClustersPerTrack->Fill(clusters.size(),tracks.size());
		#endif
	}
    
	for (const auto& cluster : clusters) {
        int plane = cluster->plane-1;
        double pos = 1000.;
        if (plane == 0) {
			pos = cluster->pos.x() + dTRDx;
			if (cluster->q_tot > cluster_maxQ_x) {
                cluster_maxQ_x = cluster->q_tot;
                cluster_maxTime_x = cluster->t_avg;
            }
			NCluster_X++;
		}
        else if (plane == 1) {
			pos = cluster->pos.y() + dTRDy;
			if (cluster->q_tot > cluster_maxQ_y) {
                cluster_maxQ_y = cluster->q_tot;
                cluster_maxTime_y = cluster->t_avg;
            }
			NCluster_Y++;
		}
		hCluster_TimeVsPos[plane]->Fill(cluster->t_avg, pos);
		hCluster_pos_width->Fill(cluster->pos_width);
        hCluster_time_width->Fill(cluster->time_width);
    }
	if (cluster_maxQ_x > 0.) hCluster_TimeVsdE_Max[0]->Fill(cluster_maxTime_x, cluster_maxQ_x);
    if (cluster_maxQ_y > 0.) hCluster_TimeVsdE_Max[1]->Fill(cluster_maxTime_y, cluster_maxQ_y);

    for (const auto& hit : hits) {
        int plane = hit->plane-1;
		double pos = 1000.;
		if (plane==0) pos = -1.*STRIP_PITCH*double(NTRD_xstrips/2-hit->strip+0.5) + dTRDx;
		else if (plane==1) pos = STRIP_PITCH*double(NTRD_ystrips/2-hit->strip+0.5) + dTRDy;
		
        hClusterHits_TimeVsStrip[plane]->Fill(hit->t, hit->strip);
		if (pos<1000.) hClusterHits_TimeVsPos[plane]->Fill(hit->t, pos);
    }
		
	///////////////////////////
    //      TRD Points       //
    ///////////////////////////
	
	//Points based on clusters
	if (points.size() > 0) hPoint_NHits->Fill(points.size());
	float point_maxdE=0.;
	float point_maxTime=0.;
    for (const auto& point : points) {
        hPoint_XYT->Fill(point->x,point->y,point->time);
	    hPoint_Time->Fill(point->time);
		hPoint_TimeDiff->Fill(point->t_x - point->t_y);
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
		hPoint_ZXDisplay->Fill(point->x,point->z);
		hPoint_ZYDisplay->Fill(point->y,point->z);
		if (point->dE > point_maxdE) {
            point_maxdE = point->dE;
            point_maxTime = point->time;
        }
    }
	if (point_maxdE > 0.) {
		hPoint_TimeVsdE_Max->Fill(point_maxTime, point_maxdE);
		outputTextFile2<<runnumber<<" "<<eventnumber<<endl;
	}
	
	//Points based on Hits
	if (pointHits.size() > 0) hPointH_NHits->Fill(pointHits.size());
	float pointH_maxdE=0.;
    float pointH_maxTime=0.;
    for (const auto& point : pointHits) {
        hPointH_XYT->Fill(point->x,point->y,point->time);
        hPointH_Time->Fill(point->time);
		hPointH_TimeDiff->Fill(point->t_x - point->t_y);
        hPointH_OccupancyX->Fill(point->x);
        hPointH_OccupancyY->Fill(point->y);
        hPointH_dE->Fill(point->dE);
        hPointH_dE_XY->Fill(point->dE_x,point->dE_y);
        hPointH_TimeVsdEX->Fill(point->dE_x,point->t_x);
        hPointH_TimeVsdEY->Fill(point->dE_y,point->t_y);
        hPointH_dEDiff->Fill((point->dE_x - point->dE_y)/(point->dE_x + point->dE_y));
        hPointH_dERatio->Fill((point->dE_x / point->dE_y));
        hPointH_TimeVsX->Fill(point->x,point->time);
        hPointH_TimeVsY->Fill(point->y,point->time);
        hPointH_XYDisplay->Fill(point->x,point->y);
        hPointH_ZXDisplay->Fill(point->x,point->z);
        hPointH_ZYDisplay->Fill(point->y,point->z);
		if (point->dE > pointH_maxdE) {
            pointH_maxdE = point->dE;
            pointH_maxTime = point->time;
        }
    }
	if (pointH_maxdE > 0.) {
		hPointH_TimeVsdE_Max->Fill(pointH_maxTime, pointH_maxdE);
		if (pointH_maxdE > (1500.-(2*pointH_maxTime))) {
			hPointH_TimeVsdE_el_Max->Fill(pointH_maxTime, pointH_maxdE);
			outputTextFile<<runnumber<<" "<<eventnumber<<endl;
		} else {
			hPointH_TimeVsdE_pi_Max->Fill(pointH_maxTime, pointH_maxdE);
		}
	}
	
	/////////////////////////////
    //      TRD Segments       //
    ////////////////////////////
	
    if (segments.size() > 0) hSegment_NHits->Fill(segments.size());
    for (const auto& segment : segments) {
		hSegment_TX->Fill(segment->tx);
		hSegment_TY->Fill(segment->ty);
		hSegment_OccupancyX->Fill(segment->x);
		hSegment_OccupancyY->Fill(segment->y);
	}
	
	//////////////////////////////////////////////
    //      TRD Event-By-Event Monitoring       //
    /////////////////////////////////////////////
	
	
    if (NCluster_X > 2 && NCluster_Y > 2 && eventClusterCount < NEventsMonitor) {
    	for (const auto& cluster : clusters) {
    		int plane = cluster->plane-1;
        	double pos = 1000.;
        	if (plane == 0) pos = cluster->pos.x() + dTRDx;
        	else pos = cluster->pos.y() + dTRDy;
        	if (pos<1000.) hCluster_TimeVsPosEvent[plane][eventClusterCount]->Fill(cluster->t_avg, pos);
    	}

        for (const auto& hit : hits) {
            int plane = hit->plane-1;
            double pos = 1000.;
            if (plane == 0) pos = -1*STRIP_PITCH*double(NTRD_xstrips/2-hit->strip+0.5) + dTRDx;
            else pos = STRIP_PITCH*double(NTRD_ystrips/2-hit->strip+0.5) + dTRDy;
            if (pos<1000.) hClusterHits_TimeVsPosEvent[plane][eventClusterCount]->Fill(hit->t, pos);
        }
		
		for (const auto& hit : digihits) {
            int plane = hit->plane-1;
            hDigiHit_TimeVsStripEvent[plane][eventClusterCount]->Fill(8.*hit->peak_time, hit->strip);
        }

        for (const auto& point : points) {
            hPoint_TimeVsPosEvent[0][eventClusterCount]->Fill(point->time, point->x);
            hPoint_TimeVsPosEvent[1][eventClusterCount]->Fill(point->time, point->y);
            hPoint_ZVsX_Event[eventClusterCount]->Fill(point->z, point->x);
            hPoint_ZVsY_Event[eventClusterCount]->Fill(point->z, point->y);
        }
		
		for (const auto& point : pointHits) {
            hPointH_TimeVsPosEvent[0][eventClusterCount]->Fill(point->time, point->x);
            hPointH_TimeVsPosEvent[1][eventClusterCount]->Fill(point->time, point->y);
		}
		
		#ifdef USE_TRACKING
		if (useNizarTrackMatching) {
		
        vector<vector<DTrackFitter::Extrapolation_t>> v_extrapolations;
        for (const auto& track: tracks) {
            const DChargedTrackHypothesis *hyp = track->Get_BestTrackingFOM();
            if (hyp == nullptr) continue;
            const DTrackTimeBased *trackTB = hyp->Get_TrackTimeBased();
            if (trackTB == nullptr) continue;
            vector<DTrackFitter::Extrapolation_t> extrapolations = trackTB->extrapolations.at(SYS_TRD);         
            if (extrapolations.size() == 0) continue;
            v_extrapolations.push_back(extrapolations);
        }

        unsigned int iExtrapolation = 0;
        for (const auto& extrapolations : v_extrapolations) {
            DTrackFitter::Extrapolation_t extrapolation = extrapolations[0];
            if (runnumber<140000 && ((extrapolations[0].position.x() < -83.47 || extrapolations[0].position.x() > -11.47) || 
                (extrapolations[0].position.y() < -68.6 || extrapolations[0].position.y() > -32.61))) continue; //Spring 2025 ECAL Commissioning Period
			else if (runnumber>140000 && ((extrapolations[0].position.x() < -70.39 || extrapolations[0].position.x() > -10.39) ||
                (extrapolations[0].position.y() < -51.88 || extrapolations[0].position.y() > -25.48))) continue; //Spring 2026 Low Energy Running Period
            double extrp_x = extrapolation.position.x();
            double extrp_y = extrapolation.position.y();
            double extrp_dxdz = extrapolation.momentum.x()/extrapolation.momentum.z();
            double extrp_dydz = extrapolation.momentum.y()/extrapolation.momentum.z();
            hExtrapolation_Members_Event[eventClusterCount][iExtrapolation]->SetBinContent(1, extrp_x);
            hExtrapolation_Members_Event[eventClusterCount][iExtrapolation]->SetBinContent(2, extrp_y);
            hExtrapolation_Members_Event[eventClusterCount][iExtrapolation]->SetBinContent(3, extrp_dxdz);
            hExtrapolation_Members_Event[eventClusterCount][iExtrapolation]->SetBinContent(4, extrp_dydz);
            iExtrapolation++;
            if (iExtrapolation >= NMaxExtrapolations) break;
        }

        for (unsigned int iSegment = 0; iSegment < segments.size(); iSegment++) {
            const auto& segment = segments[iSegment];
            hSegment_Members_Event[eventClusterCount][iSegment]->SetBinContent(1, segment->x);
            hSegment_Members_Event[eventClusterCount][iSegment]->SetBinContent(2, segment->y);
            hSegment_Members_Event[eventClusterCount][iSegment]->SetBinContent(3, segment->tx);
            hSegment_Members_Event[eventClusterCount][iSegment]->SetBinContent(4, segment->ty);
            hSegment_Members_Event[eventClusterCount][iSegment]->SetBinContent(5, segment->var_x);
            hSegment_Members_Event[eventClusterCount][iSegment]->SetBinContent(6, segment->var_y);
            hSegment_Members_Event[eventClusterCount][iSegment]->SetBinContent(7, segment->var_tx);
            hSegment_Members_Event[eventClusterCount][iSegment]->SetBinContent(8, segment->var_ty);
            if (verboseNizarTrackMatching) cout << "TRD_online:Process() ... segment x = " << segment->x << ", y = " << segment->y << ", tx = " << segment->tx << ", ty = " << segment->ty << endl;
        }
        if (verboseNizarTrackMatching) cout << eventClusterCount << ". Event " << eventnumber << " has " << clusters.size() << " clusters, " << segments.size() << " segments, " << tracks.size() << " tracks, " << iExtrapolation << " extrapolations at TRD" << endl;
		} //--End if useNizarTrackMatching
		#endif
        eventClusterCount++;
    }
	
	#ifdef USE_TRACKING
	if (useNizarTrackMatching) {
	
    // Calculate efficiency of extrapolation to segment matching
    if (tracks.size()>0) {
        for (const auto& track : tracks) {
            const DChargedTrackHypothesis *hyp = track->Get_BestTrackingFOM();
            if (hyp == nullptr) continue;
            const DTrackTimeBased *trackTB = hyp->Get_TrackTimeBased();
            if (trackTB == nullptr) continue;
            vector<DTrackFitter::Extrapolation_t> extrapolations = trackTB->extrapolations.at(SYS_TRD);
            if (extrapolations.size() == 0) continue;
            bool extrapolationFound = false;
            for (const auto& extrapolation : extrapolations) {
				if (runnumber<140000 && ((extrapolation.position.x() < -83.47 || extrapolation.position.x() > -11.47) ||
                (extrapolation.position.y() < -68.6 || extrapolation.position.y() > -32.61))) continue; //Spring 2025 ECAL Commissioning Period
            	else if (runnumber>140000 && ((extrapolation.position.x() < -70.39 || extrapolation.position.x() > -10.39) ||
                (extrapolation.position.y() < -51.88 || extrapolation.position.y() > -25.48))) continue; //Spring 2026 Low Energy Running Period
				
                extrapolationFound = true;
                NEventsTrack++;
                break;                
            }
            if (verboseNizarTrackMatching) { if (extrapolationFound) cout << "extrapolationFound = " << extrapolationFound << ", points.size() = " << points.size() << endl;}
            if (points.size() > 0 && extrapolationFound) NEventsTrackSegmentMatch++;
        }
    }
	} //--End if useNizarTrackMatching
	#endif
	
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
	if (useNizarTrackMatching) {
		cout << "NEventsTrackSegmentMatch = " << NEventsTrackSegmentMatch << endl;
		cout << "NEventsTrack = " << NEventsTrack << endl;
		if (NEventsTrack>0) {
			cout << "Efficiency of extrapolation to segment matching = " 
				<< double(NEventsTrackSegmentMatch)/double(NEventsTrack) << endl;
		}
	}
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
