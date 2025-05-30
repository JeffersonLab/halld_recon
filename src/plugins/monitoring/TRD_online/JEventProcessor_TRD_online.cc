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
#include <TRD/DTRDSegment.h>
#include <PID/DChargedTrack.h>
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

const int NEventsMonitor = 100;
static TH1I *hCluster_NClusters;
static TH1D *hCluster_pos_width;
static TH1D *hCluster_time_width;

const int NMaxExtrapolations = 5;
const int NMaxSegments = 5;

static TH2I *hClusterHits_TimeVsPos[NTRDplanes];
static TH2I *hCluster_TimeVsPos[NTRDplanes];
static TH2I *hClusterHits_TimeVsPosEvent[NTRDplanes][NEventsMonitor];
static TH2I *hCluster_TimeVsPosEvent[NTRDplanes][NEventsMonitor];
static TH2I *hDigiHit_TimeVsStripEvent[NTRDplanes][NEventsMonitor];
static TH2I *hPoint_TimeVsPosEvent[NTRDplanes][NEventsMonitor];

static TH3I *hPoint_XYT;
static TH1I *hPoint_NHits;
static TH1I *hPoint_Time;
static TH1I *hPoint_dE;
static TH1I *hPoint_dEDiff;
static TH1I *hPoint_dERatio;
static TH1I *hPoint_OccupancyX;
static TH1I *hPoint_OccupancyY;
static TH2I *hPoint_TimeVsX;
static TH2I *hPoint_TimeVsY;
static TH2I *hPoint_XYDisplay;
static TH2I *hPoint_ZXDisplay;
static TH2I *hPoint_ZYDisplay;
static TH2I *hPoint_dE_XY;
static TH2I *hPoint_TimeVsdEX;
static TH2I *hPoint_TimeVsdEY;
static TH1D *hSegment_Members_Event[NEventsMonitor][NMaxSegments];
static TH1D *hExtrapolation_Members_Event[NEventsMonitor][NMaxExtrapolations];
static TH2I *hPoint_ZVsX_Event[NEventsMonitor];
static TH2I *hPoint_ZVsY_Event[NEventsMonitor];
static TH1I *hNExtrapolations;

static TH3I *hPointH_XYT;
static TH1I *hPointH_NHits;
static TH1I *hPointH_Time;
static TH1I *hPointH_dE;
static TH1I *hPointH_dEDiff;
static TH1I *hPointH_dERatio;
static TH1I *hPointH_OccupancyX;
static TH1I *hPointH_OccupancyY;
static TH2I *hPointH_TimeVsX;
static TH2I *hPointH_TimeVsY;
static TH2I *hPointH_XYDisplay;
static TH2I *hPointH_ZXDisplay;
static TH2I *hPointH_ZYDisplay;
static TH2I *hPointH_dE_XY;
static TH2I *hPointH_TimeVsdEX;
static TH2I *hPointH_TimeVsdEY;

static TH1I *hSegment_NHits;
static TH1I *hSegment_TX;
static TH1I *hSegment_TY;
static TH1I *hSegment_OccupancyX;
static TH1I *hSegment_OccupancyY;

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

		hDigiHit_Occupancy[i] = new TH1I(Form("DigiHit_Occupancy_Plane%d",i),Form("TRD Plane %d Hit Occupancy;Strip;Raw Hits / Counter",i),NTRDstrips,0.5,0.5+NTRDstrips);
		hDigiHit_QualityFactor[i] = new TH1I(Form("DigiHit_QualityFactor_Plane%d",i),Form("TRD Plane %d Quality Factor;Quality Factor;Raw Hits",i),4,-0.5,3.5);
		hDigiHit_PeakVsStrip[i] = new TH2I(Form("DigiHit_PeakVsStrip_Plane%d",i),Form("TRD Plane %d Pulse Height vs. Strip;Strip;Pulse Height [fADC units]",i),NTRDstrips,0.5,0.5+NTRDstrips,410,0.0,4100.0);
		hDigiHit_PulseTime[i] = new TH1I(Form("DigiHit_PulseTime_Plane%d",i),Form("TRD Plane %d Pulse Time;Pulse Time [62.5 ps];Raw Hits",i),180,0.0,1800.0);
		hDigiHit_Time[i] = new TH1I(Form("DigiHit_Time_Plane%d",i),Form("TRD Plane %d Peak Time;8*(Peak Time) [ns];Raw Hits / 2 ns",i),225,0.0,1800.0);
		hDigiHit_TimeVsStrip[i] = new TH2I(Form("DigiHit_TimeVsStrip_Plane%d",i),Form("TRD Plane %d Time vs. Strip;Strip;8*(Peak Time) [ns]",i),NTRDstrips,0.5,0.5+NTRDstrips,225,0.0,1800.0);
		hDigiHit_TimeVsPeak[i] = new TH2I(Form("DigiHit_TimeVsPeak_Plane%d",i),Form("TRD Plane %d Time vs. Pulse Height;Pulse Height [fADC units];8*(Peak Time) [ns]",i),410,0.0,4100.0,225,0.0,1800.0);
		
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
		hHit_Occupancy[i] = new TH1I(Form("Hit_Occupancy_Plane%d",i),Form("TRD Plane %d Hit Occupancy;Strip;Calibrated Hits / Counter",i),NTRDstrips,0.5,0.5+NTRDstrips);
		hHit_Time[i] = new TH1I(Form("Hit_Time_Plane%d",i),Form("TRD Plane %d Time;8*(Peak Time) [ns];Calibrated Hits / 2 ns",i),225,0.0,1800.0);
		hHit_PulseHeight[i] = new TH1I(Form("Hit_PulseHeight_Plane%d",i),Form("TRD Plane %d Pulse Height;Pulse Height [fADC units];Calibrated Hits / 1 unit",i),350,0.0,3500.0);
        hHit_TimeVsStrip[i] = new TH2I(Form("Hit_TimeVsStrip_Plane%d",i),Form("TRD Plane %d Time vs. Strip;Strip;8*(Peak Time) [ns]",i),NTRDstrips,0.5,0.5+NTRDstrips,225,0.0,1800.0);
		hHit_StripVsdE[i] = new TH2I(Form("Hit_StripVsdE_Plane%d",i),Form("TRD Plane %d Hit Charge vs. Strip;Strip;dE [q]",i),NTRDstrips,0.5,0.5+NTRDstrips,350,0.,3500.0);
		hHit_TimeVsdE[i] = new TH2I(Form("Hit_TimeVsdE_Plane%d",i),Form("TRD Plane %d Hit Charge vs. Time;dE [q];8*(Peak Time) [ns]",i),350,0.,3500.,225,0.0,1800.0);
		
	}
    
	// point-level hists
    trdDir->cd();
	// point based on hits
	gDirectory->mkdir("Point_Hit")->cd();
    hPointH_NHits = new TH1I("PointH_NHits","TRD Calibrated Point Multiplicity;Calibrated Points;Events",100,0.5,0.5+100);
    hPointH_XYT = new TH3I("PointH_XYT","TRD 3D Points;X [cm];Y [cm];8*(Peak Time) [ns]",750,-85,-10,400,-70,-30,225,0.,1800.);
    hPointH_Time = new TH1I("PointH_Time","TRD Point Time;8*(Peak Time) [ns]; ",225,0.,1800.);
    hPointH_dE = new TH1I("PointH_dE","TRD Point Charge;Average dE [q]; ",350,0.,3500.);
    hPointH_dEDiff = new TH1I("PointH_dEDiff","TRD Point Charge X,Y Weighted Diff.;(dE_x - dE_y)/(dE_x + dE_y); ",100,-1.,1.);
    hPointH_dERatio = new TH1I("PointH_dERatio","TRD Point dE_x / dE_y;(dE_x / dE_y); ",100,-0.,6.);
    hPointH_dE_XY = new TH2I("PointH_dE_XY","TRD Point Charge Corr. in X,Y;X Strip dE [q]; Y Strip dE [q]",350,0.,3500.,350,0.,3500.);
    hPointH_TimeVsdEX= new TH2I("PointH_TimeVsdEX","TRD Point Charge of X in Time;X Strip dE [q];8*(Peak Time) [ns]",350,0.,3500.,225,0.,1800.);
    hPointH_TimeVsdEY= new TH2I("PointH_TimeVsdEY","TRD Point Charge of Y in Time;Y Strip dE [q];8*(Peak Time) [ns]",350,0.,3500.,225,0.,1800.);
    hPointH_OccupancyX = new TH1I("PointH_OccupancyX","TRD Point X;X [cm]; ",750,-85,-10);
    hPointH_OccupancyY = new TH1I("PointH_OccupancyY","TRD Point Y;Y [cm]; ",400,-70,-30);
    hPointH_XYDisplay = new TH2I("PointH_XYDisplay","TRD XY 2D Point Display ;X [cm];Y [cm]",750,-85,-10,400,-70,-30);
    hPointH_ZXDisplay = new TH2I("PointH_ZXDisplay","TRD XZ 2D Point Display;X [cm];Z [cm]",750,-85,-10,480,527.5,533.5);
    hPointH_ZYDisplay = new TH2I("PointH_ZYDisplay","TRD YZ 2D Point Display;Y [cm];Z [cm]",400,-70,-30,480,527.5,533.5);
    hPointH_TimeVsX= new TH2I("PointH_TimeVsX","TRD Point X in Time;X [cm];8*(Peak Time) [ns]",750,-85,-10,225,0.,1800.);
    hPointH_TimeVsY= new TH2I("PointH_TimeVsY","TRD Point Y in Time;Y [cm];8*(Peak Time) [ns]",400,-70,-30,225,0.,1800.);

    trdDir->cd();
	// cluster-level hists
    eventClusterCount = 0;
    eventPointCount = 0;

    gDirectory->mkdir("Cluster")->cd();
    hCluster_NClusters = new TH1I("Cluster_NClusters","TRD number of cluster per event;clusters;events",100,0.5,0.5+100);
    hCluster_pos_width = new TH1D("Cluster_pos_width","TRD Cluster Position Width;Position Width [cm];Events",100,0.,50.);
    hCluster_time_width = new TH1D("Cluster_time_width","TRD Cluster Time Width;Time Width [ns];Events",100,0.,50.0);

    for(int i=0; i<NTRDplanes; i++) {
        hClusterHits_TimeVsPos[i] = new TH2I(Form("ClusterHits_TimeVsPos_Plane%d",i),Form("TRD Plane %d Cluster Hits Time vs. Position;8*(Peak Time) [ns];Position [cm]",i),250,0,2000.0,100,-50,50);
        hCluster_TimeVsPos[i] = new TH2I(Form("Cluster_TimeVsPos_Plane%d",i),Form("TRD Plane %d Cluster Time vs. Position;8*(Peak Time) [ns];Position [cm]",i),250,0.,2000.0,100,-50,50);
    }

    // point based on clusters
    trdDir->cd();
    gDirectory->mkdir("Point")->cd();
    hPoint_NHits = new TH1I("Point_NHits","TRD Calibrated Point Multiplicity;Calibrated Points;Events",100,0.5,0.5+100);
    hPoint_XYT = new TH3I("Point_XYT","TRD 3D Points;X [cm];Y [cm];8*(Peak Time) [ns]",750,-85,-10,400,-70,-30,225,0.,1800.);
    hPoint_Time = new TH1I("Point_Time","TRD Point Time;8*(Peak Time) [ns]; ",225,0.,1800.);
    hPoint_dE = new TH1I("Point_dE","TRD Point Charge;Average dE [q]; ",350,0.,3500.);
    hPoint_dEDiff = new TH1I("Point_dEDiff","TRD Point Charge X,Y Weighted Diff.;(dE_x - dE_y)/(dE_x + dE_y); ",100,-1.,1.);
    hPoint_dERatio = new TH1I("Point_dERatio","TRD Point dE_x / dE_y;(dE_x / dE_y); ",100,-0.,6.);
    hPoint_dE_XY = new TH2I("Point_dE_XY","TRD Point Charge Corr. in X,Y;X Strip dE [q]; Y Strip dE [q]",350,0.,3500.,350,0.,3500.);
    hPoint_TimeVsdEX= new TH2I("Point_TimeVsdEX","TRD Point Charge of X in Time;X Strip dE [q];8*(Peak Time) [ns]",350,0.,3500.,225,0.,1800.);
    hPoint_TimeVsdEY= new TH2I("Point_TimeVsdEY","TRD Point Charge of Y in Time;Y Strip dE [q];8*(Peak Time) [ns]",350,0.,3500.,225,0.,1800.);
    hPoint_OccupancyX = new TH1I("Point_OccupancyX","TRD Point X;X [cm]; ",750,-85,-10);
    hPoint_OccupancyY = new TH1I("Point_OccupancyY","TRD Point Y;Y [cm]; ",400,-70,-30);
    hPoint_XYDisplay = new TH2I("Point_XYDisplay","TRD XY 2D Point Display ;X [cm];Y [cm]",750,-85,-10,400,-70,-30);
    hPoint_ZXDisplay = new TH2I("Point_ZXDisplay","TRD XZ 2D Point Display;X [cm];Z [cm]",750,-85,-10,480,527.5,533.5);
    hPoint_ZYDisplay = new TH2I("Point_ZYDisplay","TRD YZ 2D Point Display;Y [cm];Z [cm]",400,-70,-30,480,527.5,533.5);
    hPoint_TimeVsX= new TH2I("Point_TimeVsX","TRD Point X in Time;X [cm];8*(Peak Time) [ns]",750,-85,-10,225,0.,1800.);
    hPoint_TimeVsY= new TH2I("Point_TimeVsY","TRD Point Y in Time;Y [cm];8*(Peak Time) [ns]",400,-70,-30,225,0.,1800.);


    trdDir->cd();
    gDirectory->mkdir("Segment")->cd();
    hSegment_NHits = new TH1I("Segment_NHits","TRD Track Segment Multiplicity;Calibrated Track Segments;Events",20,-0.5,20-0.5);
    hSegment_TX = new TH1I("Segment_TX","TRD Track Segment Tx;Tx; ",80,-40.,40.);
    hSegment_TY = new TH1I("Segment_TY","TRD Track Segment Ty;Ty; ",80,-40.,40.);
    hSegment_OccupancyX = new TH1I("Segment_OccupancyX","TRD Track Segment X Occupancy;X [cm]; ",750,-85.,-10.);
    hSegment_OccupancyY = new TH1I("Segment_OccupancyY","TRD Track Segment Y Occupancy;Y [cm]; ",400,-70.,-30.);

    trdDir->cd();
    gDirectory->mkdir("Track")->cd();
    hNExtrapolations = new TH1I("NExtrapolations","Number of Extrapolations at TRD Plane per Event;Number of Extrapolations;Events",10,-0.5,9.5);

    trdDir->cd();
    gDirectory->mkdir("EventMonitor")->cd();
    // histograms for each plane
    const double dTRDpos[2] = {-47.4695,-59.0315}; // TRD offsets from geometry
    for(int i=0; i<NTRDplanes; i++) {
        int NTRDstrips = 0.;
        if(i==0)
	{    NTRDstrips = NTRD_xstrips;}
        else
	{    NTRDstrips = NTRD_ystrips;}
        for(int j=0; j<NEventsMonitor; j++) {
            hClusterHits_TimeVsPosEvent[i][j] = new TH2I(Form("ClusterHits_TimeVsPos_Plane%d_Event%d",i,j),Form("TRD Plane %d Cluster Hits Time vs. Pos;8*(Peak Time) [ns];Position [cm]",i),250,0,2000.0,100,-50,50);
            hCluster_TimeVsPosEvent[i][j] = new TH2I(Form("Cluster_TimeVsPos_Plane%d_Event%d",i,j),Form("TRD Plane %d Cluster Time vs. Pos;8*(Peak Time) [ns];Position [cm]",i),250,0.,2000.0,100,-50,50);
			hDigiHit_TimeVsStripEvent[i][j] = new TH2I(Form("DigiHit_TimeVsStrip_Plane%d_Event%d",i,j),Form("TRD Plane %d DigiHit Time vs. Strip;8*(Peak Time) [ns];Strip",i),250,0.,2000.0,NTRDstrips,-0.5,-0.5+NTRDstrips);
            hPoint_TimeVsPosEvent[i][j] = new TH2I(Form("Point_TimeVsPos_Plane%d_Event%d",i,j),Form("TRD Plane %d Point Time vs. Pos;8*(Peak Time) [ns];Position [cm]",i),250,0.,2000.0,100,-50+dTRDpos[i],50+dTRDpos[i]);
        }
    }

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
        
        hPoint_ZVsX_Event[i] = new TH2I(Form("Point_ZVsX_Event%d",i),Form("TRD Point X vs. Z;Z [cm];X [cm]",i),100,528,533,100,-90,-10);
        hPoint_ZVsY_Event[i] = new TH2I(Form("Point_ZVsY_Event%d",i),Form("TRD Point Y vs. Z;Z [cm];Y [cm]",i),100,528,533,100,-70,-30);        
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
    event->Get(points);
	vector<const DTRDPoint*> pointHits;
    event->Get(pointHits, "Hit");
    
    vector<const DTRDSegment*> segments;
    event->Get(segments,"Extrapolation");

    vector<const DChargedTrack*> tracks;
    event->Get(tracks);

	
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
	
    if (hits.size() > 0) hHit_NHits->Fill(hits.size());
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


    const int NUM_X_STRIPS = 720;
    const int NUM_Y_STRIPS = 528;
    const double STRIP_PITCH=0.1;

    int NCluster_X = 0;
    int NCluster_Y = 0;

    if (clusters.size() > 0) hCluster_NClusters->Fill(clusters.size());
    for (const auto& cluster : clusters) {
        int plane = cluster->plane-1;
        double pos = 0.;
        if (plane == 0) {
            pos = cluster->pos.x();
            NCluster_X++;
        }
        else {
            pos = cluster->pos.y();
            NCluster_Y++;
        }

        hCluster_TimeVsPos[plane]->Fill(cluster->t_avg, pos);
        hCluster_pos_width->Fill(cluster->pos_width);
        hCluster_time_width->Fill(cluster->time_width);
    }
    for (const auto& hit : hits) {

        int plane = hit->plane-1;
        double pos = 0;
        if (plane == 0) pos = -1*STRIP_PITCH*double(NUM_X_STRIPS/2-hit->strip+0.5);
        else pos = STRIP_PITCH*double(NUM_Y_STRIPS/2-hit->strip+0.5);
        hClusterHits_TimeVsPos[plane]->Fill(hit->t, pos);
    }
	
    // if (clusters.size() > 2 && segments.size() > 0 && tracks.size() > 0 && eventPointCount < NEventsMonitor) {
    

    // if (segments.size() > 1) cout << "TRD_online:Process() ... num input segments = " << segments.size() << endl;

		
	///////////////////////////
    //      TRD Points       //
    ///////////////////////////

    // cout << "Event " << eventnumber << " has " << points.size() << " points" << endl;
	
	//Points based on clusters
	if (points.size() > 0) hPoint_NHits->Fill(points.size());
    for (const auto& point : points) {
        hPoint_XYT->Fill(point->x,point->y,point->time);
	    hPoint_Time->Fill(point->time);
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
    }
    

    //Points based on Hits
    if (pointHits.size() > 0) hPointH_NHits->Fill(pointHits.size());
    for (const auto& point : pointHits) {
        hPointH_XYT->Fill(point->x,point->y,point->time);
        hPointH_Time->Fill(point->time);
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
    }
	
	///////////////////////////
    //      TRD Segments       //
    ///////////////////////////

    if (segments.size() > 0) hSegment_NHits->Fill(segments.size());
    for (const auto& segment : segments) {
		hSegment_TX->Fill(segment->tx);
		hSegment_TY->Fill(segment->ty);
		hSegment_OccupancyX->Fill(segment->x);
		hSegment_OccupancyY->Fill(segment->y);
	}

    // if (tracks.size() > 0 && eventPointCount < NEventsMonitor) {
    //     cout << "Event " << eventnumber << " has " << clusters.size() << " clusters, " << segments.size() << " segments, " << tracks.size() << " tracks" << endl;
    //     vector<vector<DTrackFitter::Extrapolation_t>> v_extrapolations;
    //     vector<DTrackFitter::Extrapolation_t> extrapolations;
    //     for (const auto& track: tracks) {
    //         const DChargedTrackHypothesis *hypElectron = track->Get_Hypothesis(Electron);
    //         const DChargedTrackHypothesis *hypPositron = track->Get_Hypothesis(Positron);
    //         const DChargedTrackHypothesis *hyp = (hypElectron != nullptr) ? hypElectron : hypPositron;
    //         if (hyp == nullptr) continue;
    //         const DTrackTimeBased *trackTB = hyp->Get_TrackTimeBased();
    //         if (trackTB == nullptr) continue;
    //         extrapolations = trackTB->extrapolations.at(SYS_TRD);         
    //         // cout << "TRD_online:Process() ... extrapolations.size() = " << extrapolations.size() << endl;
    //         if (extrapolations.size() == 0) continue;
    //         v_extrapolations.push_back(extrapolations);
    //     }

    //     // if (v_extrapolations.size() == 0) {
    //     //     cout << "TRD_online:Process() ... no extrapolations found" << endl;
    //     //     lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
    //     //     return;
    //     // }

    //     unsigned int iExtrapolation = 0;
    //     for (const auto& extrapolations : v_extrapolations) {
    //         DTrackFitter::Extrapolation_t extrapolation = extrapolations[0];
    //         if ((extrapolations[0].position.x() < -83.47 || extrapolations[0].position.x() > -11.47) || 
    //             (extrapolations[0].position.y() < -68.6 || extrapolations[0].position.y() > -32.61)) continue;
    //         double extrp_x = extrapolation.position.x();
    //         double extrp_y = extrapolation.position.y();
    //         double extrp_dxdz = extrapolation.momentum.x()/extrapolation.momentum.z();
    //         double extrp_dydz = extrapolation.momentum.y()/extrapolation.momentum.z();
    //         // cout << "TRD_online:Process() ... segment x = " << segment_x << ", y = " << segment_y << ", tx = " << segment_tx << ", ty = " << segment_ty << endl;
    //         // cout << "TRD_online:Process() ... extrapolation x = " << extrp_x << ", y = " << extrp_y << ", dxdz = " << extrp_dxdz << ", dydz = " << extrp_dydz << endl;
    //         hExtrapolation_Members_Event[eventPointCount][iExtrapolation]->SetBinContent(1, extrp_x);
    //         hExtrapolation_Members_Event[eventPointCount][iExtrapolation]->SetBinContent(2, extrp_y);
    //         hExtrapolation_Members_Event[eventPointCount][iExtrapolation]->SetBinContent(3, extrp_dxdz);
    //         hExtrapolation_Members_Event[eventPointCount][iExtrapolation]->SetBinContent(4, extrp_dydz);
    //         iExtrapolation++;
    //         if (iExtrapolation >= NMaxExtrapolations) break;
    //     }

    //     // double segment_x = 0.;
    //     // double segment_y = 0.;
    //     // double segment_tx = 0.;
    //     // double segment_ty = 0.;
    //     for (unsigned int iSegment = 0; iSegment < segments.size(); iSegment++) {
    //         const auto& segment = segments[iSegment];
    //         hSegment_Members_Event[eventPointCount][iSegment]->SetBinContent(1, segment->x);
    //         hSegment_Members_Event[eventPointCount][iSegment]->SetBinContent(2, segment->y);
    //         hSegment_Members_Event[eventPointCount][iSegment]->SetBinContent(3, segment->tx);
    //         hSegment_Members_Event[eventPointCount][iSegment]->SetBinContent(4, segment->ty);
    //         hSegment_Members_Event[eventPointCount][iSegment]->SetBinContent(5, segment->var_x);
    //         hSegment_Members_Event[eventPointCount][iSegment]->SetBinContent(6, segment->var_y);
    //         hSegment_Members_Event[eventPointCount][iSegment]->SetBinContent(7, segment->var_tx);
    //         hSegment_Members_Event[eventPointCount][iSegment]->SetBinContent(8, segment->var_ty);
    //         // segment_x = segment->x;
    //         // segment_y = segment->y;
    //         // segment_tx = segment->tx;
    //         // segment_ty = segment->ty;
    //     }

    //     for (const auto& point: points) {
    //         hPoint_ZVsX_Event[eventPointCount]->Fill(point->z, point->x);
    //         hPoint_ZVsY_Event[eventPointCount]->Fill(point->z, point->y);
    //     }

    //     eventPointCount++;
    // }

    // hNExtrapolations->Fill(NExtrapolations);

    if (NCluster_X > 3 && NCluster_Y > 3 && eventClusterCount < NEventsMonitor) {
    	for (const auto& cluster : clusters) {
    		int plane = cluster->plane-1;
        	double pos = 0.;
        	if (plane == 0) pos = cluster->pos.x();
        	else pos = cluster->pos.y();

        	hCluster_TimeVsPosEvent[plane][eventClusterCount]->Fill(cluster->t_avg, pos);
    	}

        for (const auto& hit : hits) {
            int plane = hit->plane-1;
            double pos = 0;
            if (plane == 0) pos = -1*STRIP_PITCH*double(NUM_X_STRIPS/2-hit->strip+0.5);
            else pos = STRIP_PITCH*double(NUM_Y_STRIPS/2-hit->strip+0.5);
            hClusterHits_TimeVsPosEvent[plane][eventClusterCount]->Fill(hit->t, pos);
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

        vector<vector<DTrackFitter::Extrapolation_t>> v_extrapolations;
        for (const auto& track: tracks) {
            const DChargedTrackHypothesis *hypElectron = track->Get_Hypothesis(Electron);
            const DChargedTrackHypothesis *hypPositron = track->Get_Hypothesis(Positron);
            const DChargedTrackHypothesis *hypPiMinus = track->Get_Hypothesis(PiMinus);
            const DChargedTrackHypothesis *hypPiPlus = track->Get_Hypothesis(PiPlus);
            const DChargedTrackHypothesis *hypMinus = (hypElectron != nullptr) ? hypElectron : hypPiMinus;
            const DChargedTrackHypothesis *hypPlus = (hypPositron != nullptr) ? hypPositron : hypPiPlus;
            const DChargedTrackHypothesis *hyp = (hypMinus != nullptr) ? hypMinus : hypPlus;
            if (hyp == nullptr) continue;
            const DTrackTimeBased *trackTB = hyp->Get_TrackTimeBased();
            if (trackTB == nullptr) continue;
            vector<DTrackFitter::Extrapolation_t> extrapolations = trackTB->extrapolations.at(SYS_TRD);         
            // cout << "TRD_online:Process() ... extrapolations.size() = " << extrapolations.size() << endl;
            if (extrapolations.size() == 0) continue;
            v_extrapolations.push_back(extrapolations);
        }

        // if (v_extrapolations.size() == 0) {
        //     cout << "TRD_online:Process() ... no extrapolations found" << endl;
        //     lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
        //     return;
        // }

        unsigned int iExtrapolation = 0;
        for (const auto& extrapolations : v_extrapolations) {
            DTrackFitter::Extrapolation_t extrapolation = extrapolations[0];
            if ((extrapolations[0].position.x() < -83.47 || extrapolations[0].position.x() > -11.47) || 
                (extrapolations[0].position.y() < -68.6 || extrapolations[0].position.y() > -32.61)) continue;
            double extrp_x = extrapolation.position.x();
            double extrp_y = extrapolation.position.y();
            double extrp_dxdz = extrapolation.momentum.x()/extrapolation.momentum.z();
            double extrp_dydz = extrapolation.momentum.y()/extrapolation.momentum.z();
            // cout << "TRD_online:Process() ... segment x = " << segment_x << ", y = " << segment_y << ", tx = " << segment_tx << ", ty = " << segment_ty << endl;
            // cout << "TRD_online:Process() ... extrapolation x = " << extrp_x << ", y = " << extrp_y << ", dxdz = " << extrp_dxdz << ", dydz = " << extrp_dydz << endl;
            hExtrapolation_Members_Event[eventClusterCount][iExtrapolation]->SetBinContent(1, extrp_x);
            hExtrapolation_Members_Event[eventClusterCount][iExtrapolation]->SetBinContent(2, extrp_y);
            hExtrapolation_Members_Event[eventClusterCount][iExtrapolation]->SetBinContent(3, extrp_dxdz);
            hExtrapolation_Members_Event[eventClusterCount][iExtrapolation]->SetBinContent(4, extrp_dydz);
            iExtrapolation++;
            if (iExtrapolation >= NMaxExtrapolations) break;
        }

        // double segment_x = 0.;
        // double segment_y = 0.;
        // double segment_tx = 0.;
        // double segment_ty = 0.;
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
            // segment_x = segment->x;
            // segment_y = segment->y;
            // segment_tx = segment->tx;
            // segment_ty = segment->ty;
            // cout << "TRD_online:Process() ... segment x = " << segment->x << ", y = " << segment->y << ", tx = " << segment->tx << ", ty = " << segment->ty << endl;
        }

        cout << eventClusterCount << ". Event " << eventnumber << " has " << clusters.size() << " clusters, " << segments.size() << " segments, " << tracks.size() << " tracks, " << iExtrapolation << " extrapolations at TRD" << endl;
        eventClusterCount++;
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
