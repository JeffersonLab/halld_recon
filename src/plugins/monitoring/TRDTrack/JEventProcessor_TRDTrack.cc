// $Id$
//
//    File: JEventProcessor_TRDTrack.cc
// Created: Thu Mar 27 02:20:19 PM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

#include "JEventProcessor_TRDTrack.h"
#include <PID/DChargedTrack.h>
#include <TDirectory.h>
#include <PAIR_SPECTROMETER/DPSPair.h>
#include <TRIGGER/DL1Trigger.h>

// Dynamic arrays of histogram pointers
TH1F** hSeenHitsSingleX_fine  = new TH1F*[100];
TH1F** hSeenHitsSingleY_fine  = new TH1F*[100];
TH1F** hSeenPointsSingleX_fine  = new TH1F*[100];
TH1F** hSeenPointsSingleY_fine  = new TH1F*[100];
TH1F** hExtrapsX_fine  = new TH1F*[100];
TH1F** hExtrapsY_fine  = new TH1F*[100];
TH1F** hHit_PulseHeight_X  = new TH1F*[100];
TH1F** hHit_PulseHeight_Y  = new TH1F*[100];
TH1F** hHit_DriftTime_X  = new TH1F*[100];
TH1F** hHit_DriftTime_Y  = new TH1F*[100];

//--Initialize Trigger Counts
    ULong64_t PSPair;
    ULong64_t DL1Triggers;
    uint64_t startTime;
	bool firstGroup;
	int NPSTriggers;
	int NDL1Triggers;
	int NEvents;
	bool PlotEfficienciesOverTime;
		
// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_TRDTrack());
  }

} // "C"


//------------------
// JEventProcessor_TRDTrack (Constructor)
//------------------
JEventProcessor_TRDTrack::JEventProcessor_TRDTrack()
{
}

//------------------
// ~JEventProcessor_TRDTrack (Destructor)
//------------------
JEventProcessor_TRDTrack::~JEventProcessor_TRDTrack()
{
}

void JEventProcessor_TRDTrack::Count(const char *tit) {
  hCount->Fill(tit,1);
}

void JEventProcessor_TRDTrack::Count_el(const char *tit) {
  hCount_el->Fill(tit,1);
}

void JEventProcessor_TRDTrack::Count_pi(const char *tit) {
  hCount_pi->Fill(tit,1);
}

//------------------
// Init
//------------------
void JEventProcessor_TRDTrack::Init()
{
  	auto app = GetApplication();
  	lockService = app->GetService<JLockService>();
	
	// create root folder for TRD and cd to it, store main dir
	TDirectory *mainDir = gDirectory;
	TDirectory *trdDir = gDirectory->mkdir("TRDTrack");
	trdDir->cd();
	
	//--Book Histograms

	//======= Subdirectory for monitoring/selections/misc. ===========
	PlotEfficienciesOverTime = true;
	gDirectory->mkdir("Misc_Monitoring")->cd();
	hCount = new TH1D("hCount","Incrementors for Passing Various Cuts, Best Hyp",3,0,3);
  	hCount->SetStats(0); hCount->SetFillColor(38);  hCount->SetMinimum(1.);
	#if ROOT_VERSION_CODE > ROOT_VERSION(6,0,0)
    	hCount->SetCanExtend(TH1::kXaxis);
  	#else
    	hCount->SetBit(TH1::kCanRebin);
  	#endif
	
	//-- Plots for track quality monitoring / selections
	hTrackMult=new TH1D("TrackMult","Charged Track Multiplicity;Num Tracks",12,0.5,12.5);
	hTrackingFOMChisq=new TH1D("TrackingFOMChisq","Best Hyp. Track FOM Chisq;ChiSq",200,-0.5,199.5);
	hTrackingFOMNdof=new TH1D("TrackingFOMNdof","Best Hyp. Track FOM NDoF;NDoF",30,-0.5,29.5);
    hExtrapPx=new TH1D("ExtrapPx","X Momentum for 'Good' Track Extraps at TRD Z Plane;Extrap Px [GeV/c]",200,-3.,3.);
    hExtrapPy=new TH1D("ExtrapPy","Y Momentum for 'Good' Track Extraps at TRD Z Plane;Extrap Py [GeV/c]",200,-3.,3.);
    hExtrapPz=new TH1D("ExtrapPz","Z Momentum for 'Good' Track Extraps at TRD Z Plane;Extrap Pz [GeV/c]",240,-0.1,11.9);
    hTRDXCorr=new TH2D("TRDXCorr","X Corr. Between Track Extrap and TRD Point_Hits;x(Track Extrap.) [cm];x(TRD Point_Hit) [cm]",900,-90.,0.,900,-90.,0.);
    hTRDYCorr=new TH2D("TRDYCorr","Y Corr. Between Track Extrap and TRD Point_Hits;y(Track Extrap.) [cm];y(TRD Point_Hit) [cm]",900,-90.,0.,900,-90.,0.);
    hTRDXCorr_matched=new TH2D("TRDXCorr_matched","X Corr. Between Track Extrap and TRD Point_Hits with Track Matched to TRD;x(Track Extrap.) [cm];x(TRD Point_Hit) [cm]",900,-90.,0.,900,-90.,0.);
    hTRDYCorr_matched=new TH2D("TRDYCorr_matched","Y Corr. Between Track Extrap and TRD Point_Hits with Track Matched to TRD;y(Track Extrap.) [cm];y(TRD Point_Hit) [cm]",900,-90.,0.,900,-90.,0.);

	//-- Plots for TRD Point_Hit objects
	hExtrapXPointDiff=new TH1D("ExtrapXPointDiff","Track Extrap Difference from TRD Point_Hits, X Plane;x(Extrap) - x(TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapYPointDiff=new TH1D("ExtrapYPointDiff","Track Extrap Difference from TRD Point_Hits, Y Plane;y(Extrap) - y(TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapXYPointDiff=new TH2D("ExtrapXYPointDiff","2D Track Extrap Difference from TRD Point_Hits;x(Extrap) - x(TRD Point_Hit) [cm];y(Extrap) - y(TRD Point_Hit) [cm]",250,-5.5,5.5,250,-5.5,5.5);
	hExtrapXPointDiffvsTime=new TH2D("ExtrapXPointDiffvsTime","Track Extrap Difference from TRD Point_Hits vs Drift Time, X Plane;x(Extrap) - x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,200,0.,200.*8);
    hExtrapYPointDiffvsTime=new TH2D("ExtrapYPointDiffvsTime","Track Extrap Difference from TRD Point_Hits vs Drift Time, Y Plane;y(Extrap) - y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,200,0.,200.*8);
	hExtrapXPointDiffvsX=new TH2D("ExtrapXPointDiffvsX","Track Extrap Difference from TRD Point_Hits vs Position, X Plane;x(Extrap) - x(TRD Point_Hit) [cm]; x(TRD Point_Hit) [cm]",250,-5.5,5.5,450,-90.,0.);
    hExtrapYPointDiffvsY=new TH2D("ExtrapYPointDiffvsY","Track Extrap Difference from TRD Point_Hits vs Position, Y Plane;y(Extrap) - y(TRD Point_Hit);y(TRD Point_Hit) [cm]",250,-5.5,5.5,450,-90.,0.);
	
		
	//--Plots for TRD Hit objects
	hExtrapXHitDiffvsTime=new TH2D("ExtrapXHitDiffvsTime","Track Extrap Difference from TRD Hits vs Drift Time, X Plane;x(Extrap) - x(TRD Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,200,0.,200.*8);
	hExtrapYHitDiffvsTime=new TH2D("ExtrapYHitDiffvsTime","Track Extrap Difference from TRD Hits vs Drift Time, Y Plane;y(Extrap) - y(TRD Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,200,0.,200.*8);
	hResXHitDiffvsTime=new TH2D("ResXHitDiffvsTime","Track Extrap X Difference from TRD Hits vs Drift Time;x(Extrap) - x(TRD Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,200,0.,200.*8);
    hResYHitDiffvsTime=new TH2D("ResYHitDiffvsTime","Track Extrap Y Difference from TRD Hits vs Drift Time;y(Extrap) - y(TRD Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,200,0.,200.*8);
	hResXHitDiff_Corrected=new TH1D("ResXHitDiff_Corrected","Corrected Track Extrap X Difference from TRD Hits;x(Extrap) - x(TRD Hit) [cm]",250,-5.5,5.5);
	hResYHitDiff_Corrected=new TH1D("ResYHitDiff_Corrected","Corrected Track Extrap Y Difference from TRD Hits;y(Extrap) - y(TRD Hit) [cm]",250,-5.5,5.5);
	
	//-- Plots for FCAL selection quality monitoring, visualization, etc.
	hFCALShowerXY=new TH2D("FCALShowerXY","2D Display for FCAL Shower Position;x(FCAL Shower) [cm];y(FCAL Shower) [cm]",280,-140.,140.,280,-140.,140.);
    hFCALShowerXY_TRD=new TH2D("FCALShowerXY_TRD","2D Display for FCAL Shower Position with Track Extrap through TRD;x(FCAL Shower) [cm];y(FCAL Shower) [cm]",280,-140.,140.,140,-140.,0.);
    hFCALShowerTime=new TH1D("FCALShowerTime","Time of FCAL Shower;Time [ns]",210,-6.,99.);
	hFCALExtrapXY=new TH2D("FCALExtrapXY","2D Display for Track Extraps at FCAL Z Plane;x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALExtrapXY_TRD=new TH2D("FCALExtrapXY_TRD","2D Display for Track Extraps at FCAL Z Plane with Extrap through TRD;x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALExtrapTime=new TH1D("FCALExtrapTime","Time of Track Extrap at FCAL Z Plane;Time [ns]",210,-6.,99.);
	
	hFCALXCorr=new TH2D("FCALXCorr","X Corr. Between FCAL Shower and Track Extrap at FCAL Z Plane;x(Track Extrap) [cm];x(FCAL Shower) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALYCorr=new TH2D("FCALYCorr","Y Corr. Between FCAL Shower and Track Extrap at FCAL Z Plane;y(Track Extrap) [cm];y(FCAL Shower) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALXDiff=new TH1D("FCALXDiff","Position Difference Between FCAL Shower and Track Extrap at FCAL Z Plane, X Plane;x(Track Extrap) - x(FCAL Shower) [cm]",320,-16.,16.);
	hFCALYDiff=new TH1D("FCALYDiff","Position Difference Between FCAL Shower and Track Extrap at FCAL Z Plane, Y Plane;y(Track Extrap) - y(FCAL Shower) [cm]",320,-16.,16.);
	hFCALFlightTime=new TH1D("FCALFlightTime","FCAL_Track_Match Flight Time;Time [ns]",210,-6.,99.);
	hFCALTimeCorr=new TH2D("FCALTimeCorr","Time Corr. Between FCAL Shower Time and FCAL_Track_Match Flight Time;Time (FCAL Shower) [ns];Time (Track Flight Time) [ns]",210,-6.,99.,210,-6.,99.);
	hFCALExtrapE_TRD=new TH1D("FCALExtrapE_TRD","FCAL Shower Energy with Track Extrap through TRD;E [GeV]",240,-0.1,11.9);
    hFCALExtrapEP_TRD=new TH1D("FCALExtrapEP_TRD","FCAL E/P with Track Extrap through TRD;(FCAL Shower E) / (Track Extrap P_Mag)",300,-0.1,2.9);
    hFCALExtrapEPvsP_TRD=new TH2D("FCALExtrapEPvsP_TRD","FCAL E/P vs P with Track Extrap through TRD;(FCAL Shower E) / (Track Extrap P_Mag); (Track Extrap P_Mag) [GeV/c]",150,-0.1,2.9,120,-0.1,11.9);
	hFCALExtrapTheta_TRD=new TH1D("FCALExtrapTheta_TRD","FCAL Track Extrap Angle at FCAL Z Plane (with Extrap through TRD);Theta [Degrees]",350,-0.1,34.9);
    hFCALExtrapThetavsP_TRD=new TH2D("FCALExtrapThetavsP_TRD","FCAL Track Extrap Angle (at FCAL Z Plane) vs P (with Extrap through TRD);Theta [Degrees];(Track Extrap P_Mag) [GeV/c]",175,-0.1,34.9,120,-0.1,11.9);
	
	//--Plots for Efficiency estimates
	hnumPointsSeen=new TH1D("numPointsSeen","N Point_Hits within +/-1.5cm of a Track Extrap that Passed Through TRD; # Point_Hits",35,-0.5,34.5);
	hSeenPointsXY=new TH2D("seenPointsXY","2D Display all Point_Hits within +/-1.5cm of a Track Extrap that Passed Through TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",90,-90.,0.,90,-90.,0.);
	hSeenPointsSingleXY=new TH2D("seenPointsSingleXY","2D Display of One-Point-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",90,-90.,0.,90,-90.,0.);
	hSeenPointsSingleX=new TH1D("seenPointsSingleX","One-Point-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD, X Plane; x(Point_Hits) [cm]",90,-90.,0.);
	hSeenPointsSingleY=new TH1D("seenPointsSingleY","One-Point-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD, Y Plane;y(Point_Hits) [cm]",90,-90.,0.);
	
	hExtrapsXY=new TH2D("ExtrapsXY","2D Display for Track Extraps that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",90,-90.,0.,90,-90.,0.);
	hExtrapsX=new TH1D("ExtrapsX","Track Extraps that Pass Through TRD, X Plane; x(Track Extrap.) [cm]",90,-90.,0.);
    hExtrapsY=new TH1D("ExtrapsY","Track Extraps that Pass Through TRD, Y Plane; y(Track Extrap.) [cm]",90,-90.,0.);
    hSeenHitsSingleXY=new TH2D("seenHitsSingleXY","2D Display of One-Hit-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD; x(Hits) [cm]; y(Hits) [cm]",90,-90.,0.,90,-90.,0.);
    hSeenHitsSingleX=new TH1D("seenHitsSingleX","One-Hit-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD, X Plane;x(Hits) [cm]",90,-90.,0.);
    hSeenHitsSingleY=new TH1D("seenHitsSingleY","One-Hit-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD, Y Plane;y(Hits) [cm]",90,-90.,0.);
	//======= End Subdirectory for monitoring/selections/misc. ===========
	
	//======= Electron Hypothesis Subdirectory ===========
	trdDir->cd();
	gDirectory->mkdir("Electron")->cd();
	hCount_el = new TH1D("hCount_el","Incrementors for Passing Various Cuts, e+/e- Hyp",3,0,3);
    hCount_el->SetStats(0); hCount_el->SetFillColor(38);  hCount_el->SetMinimum(1.);
    #if ROOT_VERSION_CODE > ROOT_VERSION(6,0,0)
        hCount_el->SetCanExtend(TH1::kXaxis);
    #else
        hCount_el->SetBit(TH1::kCanRebin);
    #endif
	
    hHypEnergy_el=new TH1D("HypEnergy_el","e+/e- Track Hyp. Energy at Start of Flight Path, for Tracks Seen by TRD;E [GeV]",120,-0.1,11.9);
    hHypMomentum_el=new TH1D("HypMomentum_el","e+/e- Track Hyp. Momentum, for Tracks Seen by TRD;P [GeV/c]",120,-0.1,11.9);
    hHypTheta_el=new TH1D("HypTheta_el","e+/e- Track Hyp. Theta (at origin), for Tracks Seen by TRD;Theta [Degrees]",175,-0.1,34.9);
	
	hFCALEP_TRD_el=new TH1D("FCALEP_TRD_el","FCAL E/P with Track Seen by TRD;(FCAL E) / (Track Extrap P)",300,-0.1,2.9);
	hFCALEP_cut_el=new TH1D("FCALEP_cut_el","FCAL E/P with Track Seen by TRD and E/P Cut;(FCAL Shower E) / (Track Extrap P at FCAL Z Plane)",300,-0.1,2.9);
	hFCALMatchXYDisplay_el=new TH2D("FCALMatchXYDisplay_el","2D Display of FCAL Shower with Track Seen by TRD and E/P Cut;x(FCAL Shower) [cm];y(FCAL Shower) [cm]",280,-140.,140.,140,-140.,0.);
	
	hExtrapXMaxPointDiff_el=new TH1D("ExtrapXMaxPointDiff_el","Track Extrap Difference from Max TRD Point_Hit, X Plane;x(Track Extrap) - x(Max TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapYMaxPointDiff_el=new TH1D("ExtrapYMaxPointDiff_el","Track Extrap Difference from Max TRD Point_Hit, Y Plane;y(Track Extrap) - y(Max TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapXYMaxPointDiff_el=new TH2D("ExtrapXYMaxPointDiff_el","2D Display of Track Extrap Difference from Max TRD Point_Hit;x(Track Extrap) - x(Max TRD Point_Hit) [cm];y(Track Extrap) - y(Max TRD Point_Hit) [cm]",250,-5.5,5.5,250,-5.5,5.5);
	hXPointvsTime_el=new TH2D("XPointvsTime_el","TRD Point_Hits X vs Drift Time for Matched Track Extraps;x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
	hYPointvsTime_el=new TH2D("YPointvsTime_el","TRD Point_Hits Y vs Drift Time for Matched Track Extraps;y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
	hXPointvsTime_QW_el=new TH2D("XPointvsTime_QW_el","TRD Point_Hits X vs Drift Time for Matched Track Extraps (q-weighted);x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
	hYPointvsTime_QW_el=new TH2D("YPointvsTime_QW_el","TRD Point_Hits Y vs Drift Time for Matched Track Extraps (q-weighted);y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
	hXPointvsTime_Qmax_el=new TH2D("XPointvsTime_Qmax_el","TRD Max Point_Hit X vs Drift Time for Matched Track Extraps ;x(Max TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
	hYPointvsTime_Qmax_el=new TH2D("YPointvsTime_Qmax_el","TRD Max Point_Hit Y vs Drift Time for Matched Track Extraps;y(Max TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
	hXPointvsTime_Qmax_QW_el=new TH2D("XPointvsTime_Qmax_QW_el","TRD Max Point_Hit X vs Drift Time for Matched Track Extraps (q-weighted);x(Max TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
	hYPointvsTime_Qmax_QW_el=new TH2D("YPointvsTime_Qmax_QW_el","TRD Max Point_Hit Y vs Drift Time for Matched Track Extraps (q-weighted);y(Max TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);

	
	hXPointvsTime_Converted_el=new TH2D("XPointvsTime_Converted_el","TRD Point_Hits X vs Drift Time for Matched Track Extraps;8*(Peak Time) [ns];x(TRD Point_Hit) [cm]",400,0.,200.*8,900,-90.,0.);
    hYPointvsTime_Converted_el=new TH2D("YPointvsTime_Converted_el","TRD Point_Hits Y vs Drift Time for Matched Track Extraps;8*(Peak Time) [ns];y(TRD Point_Hit) [cm]",400,0.,200.*8,900,-90.,0.);
	hXPointvsTime_Qmax_Converted_el=new TH2D("XPointvsTime_Qmax_Converted_el","TRD Max Point_Hit X vs Drift Time for Matched Track Extraps; 8*(Peak Time) [ns];x(Max TRD Point_Hit) [cm]",400,0.,200.*8,900,-90.,0.);
    hYPointvsTime_Qmax_Converted_el=new TH2D("YPointvsTime_Qmax_Converted_el","TRD Max Point_Hits Y vs Drift Time for Matched Track Extraps; 8*(Peak Time) [ns];y(Max TRD Point_Hit) [cm]",400,0.,200.*8,900,-90.,0.);
    hnumPointsSeen_el=new TH1D("numPointsSeen_el","N Point_Hits within +/-1.5cm of a Track Extrap that Passed Through the TRD; # Point_Hits",35,-0.5,34.5);
    hSeenPointsXY_el=new TH2D("seenPointsXY_el","2D Display all Point_Hits within +/-1.5cm of a Track Extrap that Passed Through TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",90,-90.,0.,90,-90.,0.);
    hSeenPointsSingleXY_el=new TH2D("seenPointsSingleXY_el","2D Display of One-Point-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",90,-90.,0.,90,-90.,0.);
	hSeenPointsSingleX_el=new TH1D("seenPointsSingleX_el","One-Point-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD, X plane; x(Point_Hits) [cm]",90,-90.,0.);
	hSeenPointsSingleY_el=new TH1D("seenPointsSingleY_el","One-Point-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD, Y Plane; y(Point_Hits) [cm]",90,-90.,0.);
    hExtrapsXY_el=new TH2D("ExtrapsXY_el","2D Display for Track Extraps that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",90,-90.,0.,90,-90.,0.);
	hExtrapsX_el=new TH1D("ExtrapsX_el","Track Extraps that Pass Through TRD, X Plane; x(Track Extrap.) [cm]",90,-90.,0.);
	hExtrapsY_el=new TH1D("ExtrapsY_el","Track Extraps that Pass Through TRD, Y Plane; y(Track Extrap.) [cm]",90,-90.,0.);
	hPoint_TimeVsdE_Max_el=new TH2D("hPoint_TimeVsdE_Max_el","TRD Max Point Charge in Time, Electrons;8*(Peak Time) [ns];Max dE [q]",150,0.,1200,500,0.,10000.);
    hPointH_TimeVsdE_Max_el=new TH2D("hPointH_TimeVsdE_Max_el","TRD Max Point_Hit Charge in Time, Electrons;8*(Peak Time) [ns];Max dE [q]",150,0.,1200.,350,0.,3500.);
	//======= End Electron Subdirectory ===========
	
	//======= Pion Subdirectory ===========
	trdDir->cd();
	gDirectory->mkdir("Pion")->cd();
    hCount_pi = new TH1D("hCount_pi","Incrementors for Passing Various Cuts, #pi+/#pi- Hyp",3,0,3);
    hCount_pi->SetStats(0); hCount_pi->SetFillColor(38);  hCount_pi->SetMinimum(1.);
    #if ROOT_VERSION_CODE > ROOT_VERSION(6,0,0)
        hCount_pi->SetCanExtend(TH1::kXaxis);
    #else
        hCount_pi->SetBit(TH1::kCanRebin);
    #endif
	
	hHypEnergy_pi=new TH1D("HypEnergy_pi","#pi+/#pi- Track Hyp. Energy at Start of Flight Path, for Tracks Seen by TRD;E [GeV]",120,-0.1,11.9);
    hHypMomentum_pi=new TH1D("HypMomentum_pi","#pi+/#pi- Track Hyp. Momentum, for Tracks Seen by TRD;P [GeV/c]",120,-0.1,11.9);
    hHypTheta_pi=new TH1D("HypTheta_pi","#pi+/#pi- Track Hyp. Theta (at origin), for Tracks Seen by TRD;Theta [Degrees]",175,-0.1,34.9);

    hFCALEP_TRD_pi=new TH1D("FCALEP_TRD_pi","FCAL E/P with Track Seen by TRD;(FCAL Shower E) / (Track Extrap P at FCAL Z Plane)",300,-0.1,2.9);
	hFCALEP_cut_pi=new TH1D("FCALEP_cut_pi","FCAL E/P with Track Seen by TRD and E/P Cut;(FCAL Shower E) / (Track Extrap P at FCAL Z Plane)",300,-0.1,2.9);
    hFCALMatchXYDisplay_pi=new TH2D("FCALMatchXYDisplay_pi","2D Display of FCAL Shower with Track Seen by TRD and E/P Cut;x(FCAL Shower) [cm];y(FCAL Shower) [cm]",280,-140.,140.,140,-140.,0.);
    
	hExtrapXMaxPointDiff_pi=new TH1D("ExtrapXMaxPointDiff_pi","Track Extrap Difference from Max TRD Point_Hit, X Plane;x(Track Extrap) - x(Max TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapYMaxPointDiff_pi=new TH1D("ExtrapYMaxPointDiff_pi","Track Extrap Difference from Max TRD Point_Hit, Y Plane;y(Track Extrap) - y(Max TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapXYMaxPointDiff_pi=new TH2D("ExtrapXYMaxPointDiff_pi","2D Display of Track Extrap Difference from Max TRD Point_Hit;x(Track Extrap) - x(Max TRD Point_Hit) [cm];y(Track Extrap) - y(Max TRD Point_Hit) [cm]",250,-5.5,5.5,250,-5.5,5.5);
    hXPointvsTime_pi=new TH2D("XPointvsTime_pi","TRD Point_Hits X vs Drift Time for Matched Track Extraps;x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
    hYPointvsTime_pi=new TH2D("YPointvsTime_pi","TRD Point_Hits Y vs Drift Time for Matched Track Extraps;y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
    hXPointvsTime_QW_pi=new TH2D("XPointvsTime_QW_pi","TRD Point_Hits X vs Drift Time for Matched Track Extraps (q-weighted);x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-95.,0.,400,0.,200.*8);
    hYPointvsTime_QW_pi=new TH2D("YPointvsTime_QW_pi","TRD Point_Hits Y vs Drift Time for Matched Track Extraps (q-weighted);y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
    hXPointvsTime_Qmax_pi=new TH2D("XPointvsTime_Qmax_pi","TRD Max Point_Hit X vs Drift Time for Matched Track Extraps;x(Max TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
    hYPointvsTime_Qmax_pi=new TH2D("YPointvsTime_Qmax_pi","TRD Max Point_Hit Y vs Drift Time for Matched Track Extraps;y(Max TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
    hXPointvsTime_Qmax_QW_pi=new TH2D("XPointvsTime_Qmax_QW_pi","TRD Max Point_Hit X vs Drift Time for Matched Track Extraps (q-weighted);x(Max TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
    hYPointvsTime_Qmax_QW_pi=new TH2D("YPointvsTime_Qmax_QW_pi","TRD Max Point_Hit Y vs Drift Time for Matched Track Extraps (q-weighted);y(Max TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",900,-90.,0.,400,0.,200.*8);
	
	hXPointvsTime_Converted_pi=new TH2D("XPointvsTime_Converted_pi","TRD Point_Hits X vs Drift Time for Matched Track Extraps;8*(Peak Time) [ns];x(TRD Point_Hit) [cm]",400,0.,200.*8,900,-90.,0.);
    hYPointvsTime_Converted_pi=new TH2D("YPointvsTime_Converted_pi","TRD Point_Hits Y vs Drift Time for Matched Track Extraps;8*(Peak Time) [ns];y(TRD Point_Hit) [cm]",400,0.,200.*8,900,-90.,0.);
	hXPointvsTime_Qmax_Converted_pi=new TH2D("XPointvsTime_Qmax_Converted_pi","TRD Max Point_Hit X vs Drift Time for Matched Track Extraps; 8*(Peak Time) [ns];x(Max TRD Point_Hit) [cm]",400,0.,200.*8,900,-90.,0.);
    hYPointvsTime_Qmax_Converted_pi=new TH2D("YPointvsTime_Qmax_Converted_pi","TRD Max Point_Hits Y vs Drift Time for Matched Track Extraps; 8*(Peak Time) [ns];y(Max TRD Point_Hit) [cm]",400,0.,200.*8,900,-90.,0.);
    hnumPointsSeen_pi=new TH1D("numPointsSeen_pi","N Point_Hits within +/-1.5cm of a Track Extrap that Passed Through the TRD; # Point_Hits",35,-0.5,34.5);
    hSeenPointsXY_pi=new TH2D("seenPointsXY_pi","2D Display all Point_Hits within +/-1.5cm of a Track Extrap that Passed Through TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",90,-90.,0.,90,-90.,0.);
    hSeenPointsSingleXY_pi=new TH2D("seenPointsSingleXY_pi","2D Display of One-Point-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",90,-90.,0.,90,-90.,0.);
    hSeenPointsSingleX_pi=new TH1D("seenPointsSingleX_pi","One-Point-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD, X Plane; x(Point_Hits) [cm]",90,-90.,0.);
    hSeenPointsSingleY_pi=new TH1D("seenPointsSingleY_pi","One-Point-Per-Track within +/-1.5cm of a Track Extrap that Passed Through TRD, Y Plane; y(Point_Hits) [cm]",90,-90.,0.);
    hExtrapsXY_pi=new TH2D("ExtrapsXY_pi","2D Display for Track Extraps that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",90,-90.,0.,90,-90.,0.);
    hExtrapsX_pi=new TH1D("ExtrapsX_pi","Track Extrapolations that Pass Through TRD, X Plane; x(Track Extrap.) [cm]",90,-90.,0.);
    hExtrapsY_pi=new TH1D("ExtrapsY_pi","Track Extrapolations that Pass Through TRD, Y Plane; y(Track Extrap.) [cm]",90,-90.,0.);
	hPoint_TimeVsdE_Max_pi=new TH2D("hPoint_TimeVsdE_Max_pi","TRD Max Point Charge in Time, Pions;8*(Peak Time) [ns];Max dE [q]",150,0.,1200,500,0.,10000.);
    hPointH_TimeVsdE_Max_pi=new TH2D("hPointH_TimeVsdE_Max_pi","TRD Max Point_Hit Charge in Time, Pions;8*(Peak Time) [ns];Max dE [q]",150,0.,1200.,350,0.,3500.);
	//======= End Pion Subdirectory ===========
	
	//======= Efficiency Studies Subdirectory ===========
	//efficiency array plots
	trdDir->cd();
	gDirectory->mkdir("Time-Based_Efficiency")->cd();
	
	hDL1Time=new TH1D("DL1Time","DL1Trigger Time; 4*(Peak Time) [s]",160,0.,160.);
	hPSPairTime=new TH1D("PSPairTime","PS Identified Pair Trigger Time; 4*(Peak Time) [ns]",400,0.,400.);
	
	hEfficiencyFitsX = new TGraphErrors();
	hEfficiencyFitsX->SetName("EfficiencyFitsX");
	hEfficiencyFitsX->SetTitle("GEMTRD X Plane Hit-Level Extrapolation Efficiency Estimate Vs Time");
	hEfficiencyFitsX->GetXaxis()->SetTitle("Trigger Time [s]");
	hEfficiencyFitsX->GetYaxis()->SetTitle("pol0 Fit Result [(Track Ex. Seen)/(Track Ex. Expected)]");
	hEfficiencyFitsX->SetMarkerStyle(21);
	hEfficiencyFitsX->SetLineColor(4);
	
	hEfficiencyFitsY = new TGraphErrors();
	hEfficiencyFitsY->SetName("EfficiencyFitsY");
	hEfficiencyFitsY->SetTitle("GEMTRD Y Plane Hit-Level Extrapolation Efficiency Estimate Vs Time");
    hEfficiencyFitsY->GetXaxis()->SetTitle("Trigger Time [s]");
    hEfficiencyFitsY->GetYaxis()->SetTitle("pol0 Fit Result [(Track Ex. Seen)/(Track Ex. Expected)]");
	hEfficiencyFitsY->SetMarkerStyle(21);
	hEfficiencyFitsY->SetLineColor(2);
	
	hFluxFits = new TGraphErrors();
	hFluxFits->SetName("FluxFits");
	hFluxFits->SetTitle("PS Pair Flux Vs Time");
	hFluxFits->GetXaxis()->SetTitle("Trigger Time [s]");
	hFluxFits->GetYaxis()->SetTitle("N Identified PS Pairs");
	hFluxFits->SetMarkerStyle(21);
	hFluxFits->SetLineColor(1);
	
	hPulseConstFitsX = new TGraphErrors();
    hPulseConstFitsX->SetName("PulseConstFitsX");
    hPulseConstFitsX->SetTitle("GEMTRD X Plane Hit-Level Extrapolation Pulse Const. Estimate Vs Avg Efficiency");
    hPulseConstFitsX->GetXaxis()->SetTitle("pol0 Fit Result [(Track Ex. Seen)/(Track Ex. Expected)]");
    hPulseConstFitsX->GetYaxis()->SetTitle("Landau Const.");
    hPulseConstFitsX->SetMarkerStyle(21);
    hPulseConstFitsX->SetLineColor(4);

    hPulseConstFitsY = new TGraphErrors();
    hPulseConstFitsY->SetName("PulseConstFitsY");
    hPulseConstFitsY->SetTitle("GEMTRD Y Plane Hit-Level Extrapolation Pulse Const. Estimate Vs Avg Efficiency");
    hPulseConstFitsY->GetXaxis()->SetTitle("pol0 Fit Result [(Track Ex. Seen)/(Track Ex. Expected)]");
    hPulseConstFitsY->GetYaxis()->SetTitle("Landau Const.");
    hPulseConstFitsY->SetMarkerStyle(21);
    hPulseConstFitsY->SetLineColor(2);
	
	hPulseMPVFitsX = new TGraphErrors();
    hPulseMPVFitsX->SetName("PulseMPVFitsX");
    hPulseMPVFitsX->SetTitle("GEMTRD X Plane Hit-Level Extrapolation Pulse MPV Estimate Vs Time");
    hPulseMPVFitsX->GetXaxis()->SetTitle("Trigger Time [s]");
    hPulseMPVFitsX->GetYaxis()->SetTitle("Landau MPV");
    hPulseMPVFitsX->SetMarkerStyle(21);
    hPulseMPVFitsX->SetLineColor(4);

    hPulseMPVFitsY = new TGraphErrors();
    hPulseMPVFitsY->SetName("PulseMPVFitsY");
    hPulseMPVFitsY->SetTitle("GEMTRD Y Plane Hit-Level Extrapolation Pulse MPV Estimate Vs Time");
    hPulseMPVFitsY->GetXaxis()->SetTitle("Trigger Time [s]");
    hPulseMPVFitsY->GetYaxis()->SetTitle("Landau MPV");
    hPulseMPVFitsY->SetMarkerStyle(21);
    hPulseMPVFitsY->SetLineColor(2);

    hPulseSigmaFitsX = new TGraphErrors();
    hPulseSigmaFitsX->SetName("PulseSigmaFitsX");
    hPulseSigmaFitsX->SetTitle("GEMTRD X Plane Hit-Level Extrapolation Pulse Sigma Estimate Vs Avg Efficiency");
    hPulseSigmaFitsX->GetXaxis()->SetTitle("pol0 Fit Result [(Track Ex. Seen)/(Track Ex. Expected)]");
    hPulseSigmaFitsX->GetYaxis()->SetTitle("Landau Sigma");
    hPulseSigmaFitsX->SetMarkerStyle(21);
    hPulseSigmaFitsX->SetLineColor(4);

    hPulseSigmaFitsY = new TGraphErrors();
    hPulseSigmaFitsY->SetName("PulseSigmaFitsY");
    hPulseSigmaFitsY->SetTitle("GEMTRD Y Plane Hit-Level Extrapolation Pulse Sigma Estimate Vs Avg Efficiency");
    hPulseSigmaFitsY->GetXaxis()->SetTitle("pol0 Fit Result [(Track Ex. Seen)/(Track Ex. Expected)]");
    hPulseSigmaFitsY->GetYaxis()->SetTitle("Landau Sigma");
    hPulseSigmaFitsY->SetMarkerStyle(21);
    hPulseSigmaFitsY->SetLineColor(2);
	
	
	for (unsigned int i=0; i<=100; i++) {
		if (PlotEfficienciesOverTime) {
	    hSeenHitsSingleX_fine[i] = new TH1F(Form("seenHitsSingleX_fine_%i", i+1),
                                    Form("Single Hit X Display for Track Extraps Seen in TRD (%i - %i)",i*20000,(i+1)*20000),90,-90.,0.);
		hSeenHitsSingleY_fine[i] = new TH1F(Form("seenHitsSingleY_fine_%i", i+1),
                                    Form("Single Hit Y Display for Track Extraps Seen in TRD (%i - %i)",i*20000,(i+1)*20000),90,-90.,0.);
		}
		hSeenPointsSingleX_fine[i] = new TH1F(Form("seenPointsSingleX_fine_%i", i+1),
                                    Form("Single Point X Display for Track Extraps Seen in TRD (%i - %i)",i*20000,(i+1)*20000),90,-90.,0.);
    	hSeenPointsSingleY_fine[i] = new TH1F(Form("seenPointsSingleY_fine_%i", i+1),
                                    Form("Single Point Y Display for Track Extraps Seen in TRD (%i - %i)",i*20000,(i+1)*20000),90,-90.,0.);
		hExtrapsX_fine[i] = new TH1F(Form("ExtrapsX_fine_%i", i+1),
                                    Form("Track X Extrapolations that Pass Through TRD (%i - %i)",i*20000,(i+1)*20000),90,-90.,0.);
    	hExtrapsY_fine[i] = new TH1F(Form("ExtrapsY_fine_%i", i+1),
                                    Form("Track X Extrapolations that Pass Through TRD (%i - %i)",i*20000,(i+1)*20000),90,-90.,0.);
		if (PlotEfficienciesOverTime) {
		hHit_PulseHeight_X[i] = new TH1F(Form("Hit_PulseHeight_X%i", i+1),
                                    Form("GEMTRD X Plane Pulse Height (%i - %i); Pulse Height [fADC Units]",i*20000,(i+1)*20000),350,0.,3500.);
		hHit_PulseHeight_Y[i] = new TH1F(Form("Hit_PulseHeight_Y%i", i+1),
                                    Form("GEMTRD Y Plane Pulse Height (%i - %i); Pulse Height [fADC Units]]",i*20000,(i+1)*20000),350,0.,3500.);
		hHit_DriftTime_X[i] = new TH1F(Form("Hit_DriftTime_X%i", i+1),
                                    Form("GEMTRD X Plane Drift Time (%i - %i); Drift Time [ns]",i*20000,(i+1)*20000),100,100.,900.);
        hHit_DriftTime_Y[i] = new TH1F(Form("Hit_DriftTime_Y%i", i+1),
                                    Form("GEMTRD Y Plane Drift Time (%i - %i); Drift Time [ns]",i*20000,(i+1)*20000),100,100.,900);
        }
	}
	//======= End Efficiency Studies Subdirectory ===========
	
	mainDir->cd();
	
	//--Initialize Trigger Counts
	PSPair=0;
	DL1Triggers=0;
	startTime=0;
	firstGroup = true;
	NPSTriggers = 0;
	NDL1Triggers = 0;
	NEvents = 0;
	//PlotEfficienciesOverTime = true;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_TRDTrack::BeginRun(const std::shared_ptr<const JEvent> &event)
{
}

//------------------
// Process
//------------------
void JEventProcessor_TRDTrack::Process(const std::shared_ptr<const JEvent> &event)
{
	vector<const DTRDPoint*>pointHits; 
    event->Get(pointHits,"Hit");
	vector<const DTRDPoint*>points; 
    event->Get(points);
	vector<const DTRDHit*>hits; 
    event->Get(hits);
	vector<const DChargedTrack*>tracks; 
    event->Get(tracks);
	vector<const DPSPair*>PSPair;
	event->Get(PSPair);
	vector<const DL1Trigger*>Trig;
	event->Get(Trig);
	//auto eventNumber = event->GetEventNumber();
	auto runNumber = event->GetRunNumber();
		
	if (Trig.size()<1) return;
	NEvents++;
  	lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
	
	int numTracks=0, extrapInTRD=0, goodFCALTrack=0, extrapInTRD_good=0, extrapInTRD_goodEl=0, extrapInTRD_goodPi=0, singleSeenExtrap=0,  singleSeenExtrapHit=0, singleSeenExtrap_el=0, extrapInTRD_el=0, singleSeenExtrap_pi=0, extrapInTRD_pi=0, extrapToFCAL=0, extrapToFCAL_el=0, extrapToFCAL_pi=0, hasMinMom=0, hasMinMom_el=0, hasMinMom_pi=0, matchedToTOF=0, matchedToTOF_el=0, matchedToTOF_pi=0, extrapToECAL=0, extrapToTOF=0, extrapToTRD=0, extrapToECAL_el=0, extrapToTOF_el=0, extrapToTRD_el=0, extrapToECAL_pi=0, extrapToTOF_pi=0, extrapToTRD_pi=0;
	
	// ======================================================
	//				Efficiency vs Time Graphs
	// ======================================================
	
	if (!PSPair.empty()) NPSTriggers += PSPair.size();
	if (PlotEfficienciesOverTime) {
	
	for (const auto& trigger : Trig) {
		NDL1Triggers++;
		//cout<<"***** N_Triggers="<<NDL1Triggers<<", nEvent="<<NEvents<<", eventNum="<<eventNumber<<", nEvent%20K="<<(eventNumber%20000)<<", firstGroup="<<firstGroup<<endl;
		if (startTime == 0) { startTime = trigger->timestamp; }
		double timeStamp = 4*(trigger->timestamp - startTime)/(1.E9);
		if (NEvents%20000==0 && !firstGroup && NEvents<2000000) {
			//cout<<"***** start_time, trigger->timestamp, timeStamp   =   "<<startTime<<", "<<trigger->timestamp<<", "<<timeStamp<<endl;
			hDL1Time->Fill(timeStamp);
			int idx = (NEvents/20000)-1;
			TF1 *func = new TF1("func","pol0",-100.,0.);
			
			for (int j=0; j<=hSeenHitsSingleX_fine[idx]->GetNbinsX(); j++) {
            	if (hSeenHitsSingleX_fine[idx]->GetBinContent(j) > hExtrapsX_fine[idx]->GetBinContent(j)) {
                	hSeenHitsSingleX_fine[idx]->SetBinContent(j,0);
                	hSeenHitsSingleX_fine[idx]->SetBinContent(j,0);
            	}
        	}
			for (int j=0; j<=hSeenHitsSingleY_fine[idx]->GetNbinsX(); j++) {
                if (hSeenHitsSingleY_fine[idx]->GetBinContent(j) > hExtrapsY_fine[idx]->GetBinContent(j)) {
                    hSeenHitsSingleY_fine[idx]->SetBinContent(j,0);
                    hSeenHitsSingleY_fine[idx]->SetBinContent(j,0);
                }
            }
			
			TGraphAsymmErrors *EFFX = new TGraphAsymmErrors();
			EFFX->Divide(hSeenHitsSingleX_fine[idx],hExtrapsX_fine[idx],"cl=0.683 b(1,1) mode");
			EFFX->GetXaxis()->SetTitle("X Position (in GlueX Coordinates) [cm]");
        	EFFX->GetYaxis()->SetTitle("(Track Extrap. Seen) / (Track Extrap. Expected)");
        	EFFX->SetTitle(Form("GEMTRD X Plane Extrapolation Efficiency Estimate %d", idx));
			EFFX->Fit(func,"q0");
			double xVal = func->GetParameter(0);
        	double xValErr = func->GetParError(0);
			
			TGraphAsymmErrors *EFFY = new TGraphAsymmErrors();
			EFFY->Divide(hSeenHitsSingleY_fine[idx],hExtrapsY_fine[idx],"cl=0.683 b(1,1) mode");
            EFFY->GetXaxis()->SetTitle("Y Position (in GlueX Coordinates) [cm]");
            EFFY->GetYaxis()->SetTitle("(Track Extrap. Seen) / (Track Extrap. Expected)");
            EFFY->SetTitle(Form("GEMTRD Y Plane Extrapolation Efficiency Estimate %d", idx));
			EFFY->Fit(func,"q0");
			double yVal = func->GetParameter(0);
        	double yValErr = func->GetParError(0);
			
			hEfficiencyFitsX->SetPoint(idx,timeStamp,xVal);
			hEfficiencyFitsX->SetPointError(idx,0,xValErr);
			hEfficiencyFitsY->SetPoint(idx,timeStamp,yVal);
            hEfficiencyFitsY->SetPointError(idx,0,yValErr);
			
			//Flux Vs Time
			hFluxFits->SetPoint(idx,timeStamp,NPSTriggers);
            hFluxFits->SetPointError(idx,0,sqrt(NPSTriggers));
			NPSTriggers = 0;
			
			//Pulse fits vs Time
            TF1 *land = new TF1("land","landau",0.,1200.);
            hHit_PulseHeight_X[idx]->Fit(land,"Rq0l");
            float Const=land->GetParameter(0);
            float MPV=land->GetParameter(1);
            float Sigma=land->GetParameter(2);
            float ConstErr=land->GetParError(0);
            float MPVErr=land->GetParError(1);
            float SigmaErr=land->GetParError(2);
			
            hPulseConstFitsX->SetPoint(idx,xVal,Const);
            hPulseConstFitsX->SetPointError(idx,xValErr,ConstErr);
            hPulseMPVFitsX->SetPoint(idx,timeStamp,MPV);
            hPulseMPVFitsX->SetPointError(idx,0,MPVErr);
            hPulseSigmaFitsX->SetPoint(idx,xVal,Sigma);
            hPulseSigmaFitsX->SetPointError(idx,xValErr,SigmaErr);
			
			hHit_PulseHeight_Y[idx]->Fit(land,"Rq0l");
            Const=land->GetParameter(0);
            MPV=land->GetParameter(1);
            Sigma=land->GetParameter(2);
            ConstErr=land->GetParError(0);
            MPVErr=land->GetParError(1);
            SigmaErr=land->GetParError(2);
            hPulseConstFitsY->SetPoint(idx,yVal,Const);
            hPulseConstFitsY->SetPointError(idx,yValErr,ConstErr);
            hPulseMPVFitsY->SetPoint(idx,timeStamp,MPV);
            hPulseMPVFitsY->SetPointError(idx,0,MPVErr);
            hPulseSigmaFitsY->SetPoint(idx,yVal,Sigma);
            hPulseSigmaFitsY->SetPointError(idx,yValErr,SigmaErr);
			
		}
		firstGroup = false;
	}
	
	} //--END PlotEfficienciesOverTime
	for (const auto& pair : PSPair) {
			const DPSPair::PSClust* flhit = pair->ee.first; //Left Hit
			const DPSPair::PSClust* frhit = pair->ee.second; //Right Hit
			double t_avg = (flhit->t + frhit->t)/2.;
        	hPSPairTime->Fill(t_avg);
    }
	// ======================================================
	//			END Efficiency vs Time Graphs
	// ======================================================
	
	// ======================================================
    //          BEGIN Loop over tracks
    // ======================================================

	hTrackMult->Fill(tracks.size());
  	for (unsigned int j=0; j<tracks.size(); j++) {
		
		numTracks++;
		bool passedThroughTRD = false;
		bool TrackMatch = false;
		bool hitTrackMatchX = false;
		bool hitTrackMatchY = false;
		bool hitTrackMatch = false;
		bool isEnergetic = false;
		bool extrapSeenByFCAL = false;
		double extrap_FCALEnergy = 0.;
		bool extrap_existsAtTRD = false;
        bool extrap_existsAtFCAL = false;
		bool extrap_existsAtECAL = false;
        bool extrap_existsAtTOF = false;
        bool hyp_matchedToFCAL = false;
        bool hyp_matchedToTOF = false;
		bool hasMinMomentum = false;
		
		//--START MISC. MONITORING/SELECTION FOLDER ------------------------------------------------------
        // ============================================================
        //  BEGIN best hypothesis, Misc_Monitoring subdirectory folder
        // ============================================================

		const DChargedTrackHypothesis *best_hyp=tracks[j]->Get_BestTrackingFOM();
		if (best_hyp!=nullptr) { //--START using Best track hypothesis
			hTrackingFOMChisq->Fill(best_hyp->Get_TrackTimeBased()->chisq);
			hTrackingFOMNdof->Fill(best_hyp->Get_TrackTimeBased()->Ndof);
			
			double p_best_hyp=best_hyp->momentum().Mag();
            shared_ptr<const DFCALShowerMatchParams>fcalparms=best_hyp->Get_FCALShowerMatchParams();
			shared_ptr<const DTOFHitMatchParams>tofparms=best_hyp->Get_TOFHitMatchParams();
            const DTrackTimeBased *time_track=best_hyp->Get_TrackTimeBased();
            vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
            vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
			vector<DTrackFitter::Extrapolation_t> ecal_extrapolations = time_track->extrapolations.at(SYS_ECAL);
			vector<DTrackFitter::Extrapolation_t> tof_extrapolations = time_track->extrapolations.at(SYS_TOF);

            if (fcal_extrapolations.size()>0) extrap_existsAtFCAL = true;
			if (ecal_extrapolations.size()>0) extrap_existsAtECAL = true;
			if (tof_extrapolations.size()>0) extrap_existsAtTOF = true;
            if (trd_extrapolations.size()>0) extrap_existsAtTRD = true;
            if (fcalparms!=nullptr) hyp_matchedToFCAL = true;
            if (tofparms!=nullptr) { hyp_matchedToTOF = true; matchedToTOF++; }
			
			if (extrap_existsAtTRD) {
				extrapToTRD++;
				if (extrap_existsAtECAL) extrapToECAL++;
				if (extrap_existsAtTOF) extrapToTOF++;
				if (extrap_existsAtFCAL) extrapToFCAL++;
				if (extrap_existsAtFCAL || extrap_existsAtECAL) {
				if (runNumber<140000) { //Spring 2025 ECAL Commissioning Period
                    if ((trd_extrapolations[0].position.x() > -83.47) && (trd_extrapolations[0].position.x() < -11.47) && (trd_extrapolations[0].position.y() > -68.6) && (trd_extrapolations[0].position.y() < -32.61)) {
                        passedThroughTRD = true;
                        extrapInTRD++;
                    }
                    if (p_best_hyp>=1.) {
                        hasMinMomentum = true;
						hasMinMom++;
                    }
                } else { //Spring 2026 Low Energy Period
                    if ((trd_extrapolations[0].position.x() > -70.39) && (trd_extrapolations[0].position.x() < -10.39) && (trd_extrapolations[0].position.y() > -51.88) && (trd_extrapolations[0].position.y() < -25.48)) {
                        passedThroughTRD = true;
                        extrapInTRD++;
                    }
                    if (p_best_hyp>=0.8) {
                        hasMinMomentum = true;
						hasMinMom++;
                    }
                }
				
                if (hyp_matchedToFCAL) {
					extrap_FCALEnergy = fcalparms->dFCALShower->getEnergy();
					if (extrap_FCALEnergy>0.) {
                		isEnergetic = true;
                	}
					if (abs(fcal_extrapolations[0].position.x() - fcalparms->dFCALShower->getPosition().X())<5. && abs(fcal_extrapolations[0].position.y() - fcalparms->dFCALShower->getPosition().Y())<5.) {
                    	extrapSeenByFCAL = true;
                	}
				}
				
                if (hasMinMomentum && isEnergetic && extrapSeenByFCAL) {
					goodFCALTrack++;
					hFCALXCorr->Fill(fcal_extrapolations[0].position.x(), fcalparms->dFCALShower->getPosition().X());
	                hFCALXDiff->Fill(fcal_extrapolations[0].position.x() - fcalparms->dFCALShower->getPosition().X());
    	            hFCALYCorr->Fill(fcal_extrapolations[0].position.y(), fcalparms->dFCALShower->getPosition().Y());
        	        hFCALYDiff->Fill(fcal_extrapolations[0].position.y() - fcalparms->dFCALShower->getPosition().Y());
            	    hFCALShowerTime->Fill(fcalparms->dFCALShower->getTime());
                	hFCALExtrapTime->Fill(fcal_extrapolations[0].t);
             	   	hFCALFlightTime->Fill(fcalparms->dFlightTime);
                	hFCALTimeCorr->Fill(fcalparms->dFCALShower->getTime(), fcalparms->dFlightTime);
					hFCALExtrapXY->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
                	hFCALShowerXY->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
					if (passedThroughTRD) {
						hFCALExtrapE_TRD->Fill(extrap_FCALEnergy);
	                    hFCALExtrapEP_TRD->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag()); //previously p_best_hyp
    	                hFCALExtrapEPvsP_TRD->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag(), fcal_extrapolations[0].momentum.Mag()); //previously p_best_hyp
        	            hFCALShowerXY_TRD->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
            	        hFCALExtrapXY_TRD->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
                	    hFCALExtrapThetavsP_TRD->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
                    	hFCALExtrapTheta_TRD->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta());
					}
				}
				
				if (hasMinMomentum && hyp_matchedToTOF) {
                    if (passedThroughTRD) {
                    int i1=0;
                    extrapInTRD_good++;

                    hExtrapPx->Fill(trd_extrapolations[0].momentum.x());
                    hExtrapPy->Fill(trd_extrapolations[0].momentum.y());
                    hExtrapPz->Fill(trd_extrapolations[0].momentum.z());
                    hExtrapsXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
                    hExtrapsX->Fill(trd_extrapolations[0].position.x());
                    hExtrapsY->Fill(trd_extrapolations[0].position.y());
                    if (NEvents<2000000) {
                        hExtrapsX_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.x());
                        hExtrapsY_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.y());
                    }
					
					for (const auto& point : pointHits) {
                        if (abs(trd_extrapolations[0].position.x() - point->x) <= 1.5 && abs(trd_extrapolations[0].position.y() - point->y) <= 1.8) { //if within 1.5cm
                            if (!TrackMatch) {
                                singleSeenExtrap++;
                                hSeenPointsSingleXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
                                hSeenPointsSingleX->Fill(trd_extrapolations[0].position.x());
                                hSeenPointsSingleY->Fill(trd_extrapolations[0].position.y());
                                if (NEvents<2000000) {
                                    hSeenPointsSingleX_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.x());
                                    hSeenPointsSingleY_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.y());
                                }
                            }
                            i1++;
                            TrackMatch = true;
                            hSeenPointsXY->Fill(point->x, point->y);
                            hTRDXCorr_matched->Fill(trd_extrapolations[0].position.x(), point->x);
                            hTRDYCorr_matched->Fill(trd_extrapolations[0].position.y(), point->y);
                        }

                        hExtrapXPointDiff->Fill(trd_extrapolations[0].position.x() - point->x);
                        hExtrapYPointDiff->Fill(trd_extrapolations[0].position.y() - point->y);
                        hExtrapXPointDiffvsTime->Fill(trd_extrapolations[0].position.x() - point->x, point->time);
                        hExtrapYPointDiffvsTime->Fill(trd_extrapolations[0].position.y() - point->y, point->time);
                        hExtrapXPointDiffvsX->Fill(trd_extrapolations[0].position.x() - point->x, point->x);
                        hExtrapYPointDiffvsY->Fill(trd_extrapolations[0].position.y() - point->y, point->y);
                        hExtrapXYPointDiff->Fill(trd_extrapolations[0].position.x() - point->x, trd_extrapolations[0].position.y() - point->y);
                        hTRDXCorr->Fill(trd_extrapolations[0].position.x(), point->x);
                        hTRDYCorr->Fill(trd_extrapolations[0].position.y(), point->y);
                    } //--END Point_Hits loop
                    if (i1 > 0) hnumPointsSeen->Fill(i1);
					
					if (PlotEfficienciesOverTime) {
					
					for (const auto& hit : hits) {
                        //Convert to GlueX coordinates
                        double newX = 0.;
                        double newY = 0.;
                        if (hit->plane == 1) { //X plane
                            if (runNumber<140000) {
								newX = (-47.4695) + -1.*0.1*(720/2-hit->strip + 0.5);
							} else {
								newX = (-40.3895) + -1.*0.1*(600/2-hit->strip + 0.5);
							}
                        } else { //Y Plane
							if (runNumber<140000) {
                            	newY = (-59.0315) + 0.1*(528/2-hit->strip + 0.5);
							} else {
								newY = (-38.6795) + 0.1*(264/2-hit->strip + 0.5);
							}
                        }
                        if (hit->plane == 1 && abs(trd_extrapolations[0].position.x() - newX) <= 1.5) { //if within 1.5cm
                            if (!hitTrackMatchX) {
                                //singleSeenExtrapHit++;
                                hSeenHitsSingleX->Fill(trd_extrapolations[0].position.x());
                                if (NEvents<2000000) {
                                    hSeenHitsSingleX_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.x());
                                }
                                hitTrackMatchX = true;
                            }
                            if (NEvents<2000000) {
                                hHit_PulseHeight_X[NEvents/20000]->Fill(hit->pulse_height);
                                hHit_DriftTime_X[NEvents/20000]->Fill(hit->t);
                            }
                        }
                        if (hit->plane == 1 && passedThroughTRD) {
                            hExtrapXHitDiffvsTime->Fill(trd_extrapolations[0].position.x() - newX, hit->t);
                            hResXHitDiffvsTime->Fill(trd_extrapolations[0].position.x() - newX, hit->t);
                            hResXHitDiff_Corrected->Fill((trd_extrapolations[0].position.x() - newX)-((hit->t - 473.197)/165.83));
                        }
                        if (hit->plane == 2 && abs(trd_extrapolations[0].position.y() - newY) <= 1.8) { //if within 1.8cm
                            if (!hitTrackMatchY) {
                                //singleSeenExtrapHit++;
                                hSeenHitsSingleY->Fill(trd_extrapolations[0].position.y());
                                if (NEvents<2000000) {
                                    hSeenHitsSingleY_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.y());
                                }
                                hitTrackMatchY = true;
                            }
                            if (NEvents<2000000) {
                                hHit_PulseHeight_Y[NEvents/20000]->Fill(hit->pulse_height);
                                hHit_DriftTime_Y[NEvents/20000]->Fill(hit->t);
                            }
                        }
                        if (hit->plane == 2 && passedThroughTRD) {
                            hExtrapYHitDiffvsTime->Fill(trd_extrapolations[0].position.y() - newY, hit->t);
                            hResYHitDiffvsTime->Fill(trd_extrapolations[0].position.y() - newY, hit->t);
                            hResYHitDiff_Corrected->Fill((trd_extrapolations[0].position.y() - newY)-((hit->t - 558.016)/439.767));
                        }
                    } //--END Hits Loop
					if (hitTrackMatchX==true && hitTrackMatchY==true) {
						if (!hitTrackMatch) {
							hSeenHitsSingleXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
							hitTrackMatch = true;
							singleSeenExtrapHit++;
						} 
					}
					} //end PlotEfficienciesOverTime
                } //--END passedThroughTRD
                } //--END hasMinMomentum & hyp_matchedToTOF
            } //--END if extrap_existsAt FCAL || ECAL
			} //--END if extrap_existsAtTRD
		} //--END using Best track hypothesis
		
		//--END MISC. MONITORING FOLDER ------------------------------------------------
		//--START ELECTRON FOLDER ------------------------------------------------------
		
        // ======================================================
        //  BEGIN e+ / e- hypothesis, Electron_hyp subdirectory folder
        // ======================================================
		
		passedThroughTRD = false;
        TrackMatch = false;
        isEnergetic = false;
        extrapSeenByFCAL = false;
        extrap_FCALEnergy = 0.;
        extrap_existsAtTRD = false;
        extrap_existsAtFCAL = false;
		extrap_existsAtECAL = false;
        extrap_existsAtTOF = false;
        hyp_matchedToFCAL = false;
        hyp_matchedToTOF = false;
        hasMinMomentum = false;
		
		const DChargedTrackHypothesis *hyp_el=tracks[j]->Get_Hypothesis(Electron);
        if (hyp_el==nullptr) {  hyp_el=tracks[j]->Get_Hypothesis(Positron); }
    	if (hyp_el!=nullptr) { //--START using e+ / e- track hypothesis
			double p_hyp_el=hyp_el->momentum().Mag();
			shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_el->Get_FCALShowerMatchParams();
			shared_ptr<const DTOFHitMatchParams>tofparms=hyp_el->Get_TOFHitMatchParams();
			const DTrackTimeBased *time_track=hyp_el->Get_TrackTimeBased();
			vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
        	vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
			vector<DTrackFitter::Extrapolation_t> ecal_extrapolations = time_track->extrapolations.at(SYS_ECAL);
			vector<DTrackFitter::Extrapolation_t> tof_extrapolations = time_track->extrapolations.at(SYS_TOF);
			if (fcal_extrapolations.size()>0) extrap_existsAtFCAL = true;
			if (tof_extrapolations.size()>0) extrap_existsAtTOF = true;
			if (ecal_extrapolations.size()>0) extrap_existsAtECAL = true;
            if (trd_extrapolations.size()>0) extrap_existsAtTRD = true;
            if (fcalparms!=nullptr) hyp_matchedToFCAL = true;
            if (tofparms!=nullptr) { hyp_matchedToTOF = true; matchedToTOF_el++; }
			
			if (extrap_existsAtTRD) {
				extrapToTRD_el++;
				if (extrap_existsAtFCAL) extrapToFCAL_el++;
				if (extrap_existsAtECAL) extrapToECAL_el++;
				if (extrap_existsAtTOF) extrapToTOF_el++;
				if (extrap_existsAtFCAL || extrap_existsAtECAL) {
				if (runNumber<140000) { //Spring 2025 ECAL Commissioning Period
                    if ((trd_extrapolations[0].position.x() > -83.47) && (trd_extrapolations[0].position.x() < -11.47) && (trd_extrapolations[0].position.y() > -68.6) && (trd_extrapolations[0].position.y() < -32.61)) {
                        passedThroughTRD = true;
						extrapInTRD_el++;
                    }
                    if (p_hyp_el>=1.) {
                        hasMinMomentum = true;
						hasMinMom_el++;
                    }
                } else { //Spring 2026 Low Energy Period
                    if ((trd_extrapolations[0].position.x() > -70.39) && (trd_extrapolations[0].position.x() < -10.39) && (trd_extrapolations[0].position.y() > -51.88) && (trd_extrapolations[0].position.y() < -25.48)) {
                        passedThroughTRD = true;
						extrapInTRD_el++;
                    }
                    if (p_hyp_el>=0.8) {
                        hasMinMomentum = true;
						hasMinMom_el++;
                    }
                }
				
				if (hyp_matchedToFCAL) {
					extrap_FCALEnergy = fcalparms->dFCALShower->getEnergy();
					if (extrap_FCALEnergy>0.) {
                        isEnergetic = true;
                    }
					if (abs(fcal_extrapolations[0].position.x() - fcalparms->dFCALShower->getPosition().X())<5. && abs(fcal_extrapolations[0].position.y() - fcalparms->dFCALShower->getPosition().Y())<5.) {
						extrapSeenByFCAL = true;
					}
				} //--END hyp_matchedToFCAL
				} //--END if extrap_existsAt FCAL || ECAL
			} //--END if extrap_existsAtTRD
				
			if (hasMinMomentum && hyp_matchedToTOF && passedThroughTRD) {
					extrapInTRD_goodEl++;
                    hExtrapsXY_el->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
					hExtrapsX_el->Fill(trd_extrapolations[0].position.x());
					hExtrapsY_el->Fill(trd_extrapolations[0].position.y());
					int i3 = 0;
					float QX_max=0.;
					float XPos_max=0.;
					float QY_max=0.;
					float YPos_max=0.;
					float TX_max=0.;
					float TY_max=0.;
					float pointH_maxdE=0.;
		            float pointH_maxTime=0.;
					float pointH_maxPosX=100.;
					float pointH_maxPosY=100.;
        		    float point_maxdE=0.;
            		float point_maxTime=0.;
					
					for (const auto& point : pointHits) {
						
						if (abs(trd_extrapolations[0].position.x() - point->x) <= 1.5 && abs(trd_extrapolations[0].position.y() - point->y) <= 1.8) { //if within 1.5cm
                            if (!TrackMatch) {
								singleSeenExtrap_el++;
								hSeenPointsSingleXY_el->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
                                hSeenPointsSingleX_el->Fill(trd_extrapolations[0].position.x());
                                hSeenPointsSingleY_el->Fill(trd_extrapolations[0].position.y());
							}
							i3++;
							TrackMatch = true;
                            hSeenPointsXY_el->Fill(point->x, point->y);
						}
						
						if (TrackMatch) {
							float QX=point->dE_x;
							float QY=point->dE_y;
							if (QX>QX_max && point->time>340.) { QX_max=QX; XPos_max=point->x; TX_max=point->time; }
							if (QY>QY_max && point->time>340.) { QY_max=QY; YPos_max=point->y; TY_max=point->time; }
		    	    		hXPointvsTime_el->Fill(point->x, point->time);
                	    	hYPointvsTime_el->Fill(point->y, point->time);
							hXPointvsTime_Converted_el->Fill(point->time, point->x);
                            hYPointvsTime_Converted_el->Fill(point->time, point->y);
							hXPointvsTime_QW_el->Fill(point->x, point->time, point->dE_x);
                    		hYPointvsTime_QW_el->Fill(point->y, point->time, point->dE_y);
			            }
						
						if (point->dE > pointH_maxdE) {
                            pointH_maxdE = point->dE;
                            pointH_maxTime = point->time;
							pointH_maxPosX = point->x;
							pointH_maxPosY = point->y;
                        }
                    } //END Point_Hits loop
                    if (pointH_maxdE > 0.) {
						hPointH_TimeVsdE_Max_el->Fill(pointH_maxTime, pointH_maxdE);
						if (TrackMatch) {
							hExtrapXMaxPointDiff_el->Fill(trd_extrapolations[0].position.x() - pointH_maxPosX);
                            hExtrapYMaxPointDiff_el->Fill(trd_extrapolations[0].position.y() - pointH_maxPosY);
                            hExtrapXYMaxPointDiff_el->Fill(trd_extrapolations[0].position.x() - pointH_maxPosX, trd_extrapolations[0].position.y() - pointH_maxPosY);
						}
					}
					for (const auto& point : points) {
						if (point->dE > point_maxdE) {
                            point_maxdE = point->dE;
                            point_maxTime = point->time;
                        }
					}
					if (point_maxdE > 0.) {
						hPoint_TimeVsdE_Max_el->Fill(point_maxTime, point_maxdE);
					}
					if (TrackMatch) {
				        hXPointvsTime_Qmax_el->Fill(XPos_max, TX_max);
                    	hYPointvsTime_Qmax_el->Fill(YPos_max, TY_max);
						hXPointvsTime_Qmax_QW_el->Fill(XPos_max, TX_max, QX_max);
                    	hYPointvsTime_Qmax_QW_el->Fill(YPos_max, TY_max, QY_max);
						hXPointvsTime_Qmax_Converted_el->Fill(TX_max, XPos_max);
                        hYPointvsTime_Qmax_Converted_el->Fill(TY_max, YPos_max);
                        hHypEnergy_el->Fill(hyp_el->energy());
                        hHypMomentum_el->Fill(p_hyp_el);
                        hHypTheta_el->Fill((180./3.141592)*hyp_el->momentum().Theta()); //theta at origin
			        }
					if (i3 > 0) hnumPointsSeen_el->Fill(i3);
      		} //--END if hasMinMomentum && hyp_matchedToTOF && passedThroughTRD
			
			if (hasMinMomentum && extrapSeenByFCAL && passedThroughTRD && isEnergetic) {
                double extrap_FCAL_EP = fcalparms->dFCALShower->getEnergy() / fcal_extrapolations[0].momentum.Mag(); //previously p_hyp_el
                if (TrackMatch) {
					hFCALEP_TRD_el->Fill(extrap_FCAL_EP);
                	if (extrap_FCAL_EP>0.80 && extrap_FCAL_EP<1.25) {
                    	hFCALMatchXYDisplay_el->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
                    	hFCALEP_cut_el->Fill(extrap_FCAL_EP);
                	} //--END E/P Cut
				}
            }
    	} //--END using e+ / e- track hypothesis
		
		//-- END ELECTRON FOLDER ---------------------------------------------------- 
		//-- START PION FOLDER ------------------------------------------------------
		
		// ======================================================
    	//  BEGIN Pi+ / Pi- hypothesis, Pion_hyp subdirectory folder
    	// ======================================================
    	
		const DChargedTrackHypothesis *hyp_pi=tracks[j]->Get_Hypothesis(PiMinus);
		if (hyp_pi==nullptr) { hyp_pi=tracks[j]->Get_Hypothesis(PiPlus); }
		passedThroughTRD = false;
        TrackMatch = false;
		isEnergetic = false;
		extrapSeenByFCAL = false;
        extrap_FCALEnergy = 0.;
		extrap_existsAtECAL = false;
		extrap_existsAtTRD = false;
		extrap_existsAtFCAL = false;
		extrap_existsAtTOF = false;
		hyp_matchedToFCAL = false;
		hyp_matchedToTOF = false;
		hasMinMomentum = false;
		
		if (hyp_pi!=nullptr) {
			double p_hyp_pi=hyp_pi->momentum().Mag();
            shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_pi->Get_FCALShowerMatchParams();
			shared_ptr<const DTOFHitMatchParams>tofparms=hyp_pi->Get_TOFHitMatchParams();
			const DTrackTimeBased *time_track=hyp_pi->Get_TrackTimeBased();
            vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
			vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
			vector<DTrackFitter::Extrapolation_t> ecal_extrapolations = time_track->extrapolations.at(SYS_ECAL);
			vector<DTrackFitter::Extrapolation_t> tof_extrapolations = time_track->extrapolations.at(SYS_TOF);
			if (fcal_extrapolations.size()>0) extrap_existsAtFCAL = true;
			if (trd_extrapolations.size()>0) extrap_existsAtTRD = true;
			if (ecal_extrapolations.size()>0) extrap_existsAtECAL = true;
			if (tof_extrapolations.size()>0) extrap_existsAtTOF = true;
			if (fcalparms!=nullptr) hyp_matchedToFCAL = true;
			if (tofparms!=nullptr) { hyp_matchedToTOF = true; matchedToTOF_pi++; }
			
			if (extrap_existsAtTRD) {
				extrapToTRD_pi++;
				if (extrap_existsAtFCAL) extrapToFCAL_pi++;
				if (extrap_existsAtECAL) extrapToECAL_pi++;
				if (extrap_existsAtTOF) extrapToTOF_pi++;
				if (extrap_existsAtFCAL || extrap_existsAtECAL) {
				if (runNumber<140000) { //Spring 2025 ECAL Commisioning Period
					if ((trd_extrapolations[0].position.x() > -83.47) && (trd_extrapolations[0].position.x() < -11.47) && (trd_extrapolations[0].position.y() > -68.6) && (trd_extrapolations[0].position.y() < -32.61)) {
                    	passedThroughTRD = true;
						extrapInTRD_pi++;
                	}
					if (p_hyp_pi>=1.) {
                        hasMinMomentum = true;
						hasMinMom_pi++;
                    }
                } else { //Spring 2026 Low Energy Period
					if ((trd_extrapolations[0].position.x() > -70.39) && (trd_extrapolations[0].position.x() < -10.39) && (trd_extrapolations[0].position.y() > -51.88) && (trd_extrapolations[0].position.y() < -25.48)) {
                        passedThroughTRD = true;
						extrapInTRD_pi++;
                    }
                    if (p_hyp_pi>=0.8) {
                        hasMinMomentum = true;
						hasMinMom_pi++;
                    }
                }
				
				if (hyp_matchedToFCAL) {
					if (abs(fcal_extrapolations[0].position.x() - fcalparms->dFCALShower->getPosition().X())<5. && abs(fcal_extrapolations[0].position.y() - fcalparms->dFCALShower->getPosition().Y())<5.) {
						extrapSeenByFCAL = true;
					}
					extrap_FCALEnergy = fcalparms->dFCALShower->getEnergy();
					if (extrap_FCALEnergy>0.) {
                		isEnergetic = true;
                	}
				} //--END hyp_matchedToFCAL
				} //--END if extrap_existsAt FCAL || ECAL
			} //--END if extrap_existsAtTRD
				
			if (hasMinMomentum && hyp_matchedToTOF && passedThroughTRD) {
					extrapInTRD_goodPi++;
                    hExtrapsXY_pi->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
					hExtrapsX_pi->Fill(trd_extrapolations[0].position.x());
					hExtrapsY_pi->Fill(trd_extrapolations[0].position.y());
					int i5 = 0;
					
					float QX_max=0.;
					float XPos_max=0.;
					float QY_max=0.;
					float YPos_max=0.;
					float TX_max=0.;
					float TY_max=0.;
					float pointH_maxdE=0.;
            		float pointH_maxTime=0.;
					float pointH_maxPosX=100.;
					float pointH_maxPosY=100.;
            		float point_maxdE=0.;
            		float point_maxTime=0.;
					
					for (const auto& point : pointHits) {
						
						if (abs(trd_extrapolations[0].position.x() - point->x) <= 1.5 && abs(trd_extrapolations[0].position.y() - point->y) <= 1.8) { //if within 1.5cm
                            if (!TrackMatch) {
                                singleSeenExtrap_pi++;
                                hSeenPointsSingleXY_pi->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
                                hSeenPointsSingleX_pi->Fill(trd_extrapolations[0].position.x());
                                hSeenPointsSingleY_pi->Fill(trd_extrapolations[0].position.y());
                            }
                            i5++;
							TrackMatch = true;
                            hSeenPointsXY_pi->Fill(point->x, point->y);
                        }
						
						if (TrackMatch) {
							float QX=point->dE_x;
                        	float QY=point->dE_y;
							if (QX>QX_max && point->time>340.) {QX_max=QX; XPos_max=point->x; TX_max=point->time;}
							if (QY>QY_max && point->time>340.) {QY_max=QY; YPos_max=point->y; TY_max=point->time;}
							hXPointvsTime_pi->Fill(point->x, point->time);
	                        hYPointvsTime_pi->Fill(point->y, point->time);
							hXPointvsTime_Converted_pi->Fill(point->time, point->x);
                            hYPointvsTime_Converted_pi->Fill(point->time, point->y);
							hXPointvsTime_QW_pi->Fill(point->x, point->time, point->dE_x);
                    	    hYPointvsTime_QW_pi->Fill(point->y, point->time, point->dE_y);
			            }
                    	if (point->dE > pointH_maxdE) {
                            pointH_maxdE = point->dE;
                            pointH_maxTime = point->time;
							pointH_maxPosX = point->x;
							pointH_maxPosY = point->y;
                        }
                    } //END Point_Hits loop
                    if (pointH_maxdE > 0.) {
						hPointH_TimeVsdE_Max_pi->Fill(pointH_maxTime, pointH_maxdE);
						if (TrackMatch) {
							hExtrapXMaxPointDiff_pi->Fill(trd_extrapolations[0].position.x() - pointH_maxPosX);
                            hExtrapYMaxPointDiff_pi->Fill(trd_extrapolations[0].position.y() - pointH_maxPosY);
                            hExtrapXYMaxPointDiff_pi->Fill(trd_extrapolations[0].position.x() - pointH_maxPosX, trd_extrapolations[0].position.y() - pointH_maxPosY);
						}
					}
                    for (const auto& point : points) {
                        if (point->dE > point_maxdE) {
                            point_maxdE = point->dE;
                            point_maxTime = point->time;
                        }
                    }
                    if (point_maxdE > 0.) {
						hPoint_TimeVsdE_Max_pi->Fill(point_maxTime, point_maxdE);
					}
					
					if (TrackMatch) {
				        hXPointvsTime_Qmax_pi->Fill(XPos_max, TX_max);
                    	hYPointvsTime_Qmax_pi->Fill(YPos_max, TY_max);
						hXPointvsTime_Qmax_QW_pi->Fill(XPos_max, TX_max, QX_max);
                    	hYPointvsTime_Qmax_QW_pi->Fill(YPos_max, TY_max, QY_max);
						hXPointvsTime_Qmax_Converted_pi->Fill(TX_max, XPos_max);
                        hYPointvsTime_Qmax_Converted_pi->Fill(TY_max, YPos_max);
						hHypEnergy_pi->Fill(hyp_pi->energy());
                        hHypMomentum_pi->Fill(p_hyp_pi);
                        hHypTheta_pi->Fill((180./3.141592)*hyp_pi->momentum().Theta());
			        }
					if (i5 > 0) hnumPointsSeen_pi->Fill(i5);
            } //--END if hasMinMomentum && hyp_matchedToTOF && passedThroughTRD
			
			if (hasMinMomentum && extrapSeenByFCAL && passedThroughTRD && isEnergetic) {
                double extrap_FCAL_EP = fcalparms->dFCALShower->getEnergy() / fcal_extrapolations[0].momentum.Mag(); // previously p_hyp_pi
                if (TrackMatch) { 
					hFCALEP_TRD_pi->Fill(extrap_FCAL_EP);
                	if (extrap_FCAL_EP<=0.6) {
                    	hFCALMatchXYDisplay_pi->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
                    	hFCALEP_cut_pi->Fill(extrap_FCAL_EP);
                	} //--END E/P Cut
            	}
			}
        } //--END pi+ / pi- hyp
		// ======================================================
        //  END Pi+ / Pi- hypothesis, Pion subdirectory folder
        // ======================================================
  	}
	
	// ======================================================
    //          END Loop over tracks
    // ======================================================

	if (numTracks>0) { Count("nTrack"); Count_el("nTrack"); Count_pi("nTrack"); }
	
	//--Best Hyp
	//if (goodFCALTrack>0) Count("goodTrackThruFCAL");
	if (extrapToTRD>0) Count("extrapToTRDz");
	if (extrapToTOF>0) Count("extrapToTOFz");
	if (extrapToFCAL>0) Count("extrapToFCALz");
	if (extrapToECAL>0) Count("extrapToECALz");
	if (matchedToTOF>0) Count("matchedToTOF");
	if (hasMinMom>0) Count("minMomOK");
	if (extrapInTRD>0) Count("ExtrapThruTRD");
	if (extrapInTRD_good>0) Count("GoodExtrapThruTRD");
	if (singleSeenExtrap>0) Count("GoodExtrapSeenByTRD");
	//if (singleSeenExtrapHit>0) Count("GoodExtrapSeenByTRD_Hits");
	
	//--e+/e- Hyp
	if (extrapToTRD_el>0) Count_el("extrapToTRDz");
    if (extrapToTOF_el>0) Count_el("extrapToTOFz");
    if (extrapToFCAL_el>0) Count_el("extrapToFCALz");
	if (extrapToECAL_el>0) Count_el("extrapToECALz");
	if (matchedToTOF_el>0) Count_el("matchedToTOF");
	if (hasMinMom_el>0) Count_el("minMomOK");
	if (extrapInTRD_el>0) Count_el("ExtrapThruTRD");
	if (extrapInTRD_goodEl>0) Count_el("GoodExtrapThruTRD");
	if (singleSeenExtrap_el>0) Count_el("GoodExtrapSeenByTRD");
	
	//--pi+/pi- Hyp
	if (extrapToTRD_pi>0) Count_pi("extrapToTRDz");
    if (extrapToTOF_pi>0) Count_pi("extrapToTOFz");
    if (extrapToFCAL_pi>0) Count_pi("extrapToFCALz");
	if (extrapToECAL_pi>0) Count_pi("extrapToECALz");
	if (matchedToTOF_pi>0) Count_pi("matchedToTOF");
	if (hasMinMom_pi>0) Count_pi("minMomOK");
	if (extrapInTRD_pi>0) Count_pi("ExtrapThruTRD");
	if (extrapInTRD_goodPi>0) Count_pi("GoodExtrapThruTRD");
    if (singleSeenExtrap_pi>0) Count_pi("GoodExtrapSeenByTRD");
	
  	lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_TRDTrack::EndRun()
{
hEfficiencyFitsX->Write();
hEfficiencyFitsY->Write();
hFluxFits->Write();
hPulseConstFitsX->Write();
hPulseConstFitsY->Write();
hPulseMPVFitsX->Write();
hPulseMPVFitsY->Write();
hPulseSigmaFitsX->Write();
hPulseSigmaFitsY->Write();
}

//------------------
// Finish
//------------------
void JEventProcessor_TRDTrack::Finish()
{
}

