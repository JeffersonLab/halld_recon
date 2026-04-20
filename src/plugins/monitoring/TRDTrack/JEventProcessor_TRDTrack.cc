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
	
	hCount = new TH1D("hCount","Count",3,0,3);
  	hCount->SetStats(0); hCount->SetFillColor(38);  hCount->SetMinimum(1.);
  	#if ROOT_VERSION_CODE > ROOT_VERSION(6,0,0)
    	hCount->SetCanExtend(TH1::kXaxis);
  	#else
    	hCount->SetBit(TH1::kCanRebin);
  	#endif
	hTrackMult=new TH1D("TrackMult","Charged Track Multiplicity;Num Tracks",12,0.5,12.5);
	hHypMomentumDiff=new TH1D("HypMomentumDiff","FCAL Extrap vs Electron Hyp. Momentum Difference;(El Hyp. Track P) - (FCAL Track Extrap P) [GeV/c]",200,-5.,5.);
	hTrackingFOMChisq=new TH1D("TrackingFOMChisq","Charged Track FOM Chisq;ChiSq",100,-0.5,99.5);
	hTrackingFOMNdof=new TH1D("TrackingFOMNdof","Charged Track FOM NDoF;NDoF",20,-0.5,19.5);
    hExtrapXPointDiff=new TH1D("ExtrapXPointDiff","Track Extrap X Difference from TRD Point_Hits;x(Extrap) - x(TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapYPointDiff=new TH1D("ExtrapYPointDiff","Track Extrap Y Difference from TRD Point_Hits;y(Extrap) - y(TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapXYPointDiff=new TH2D("ExtrapXYPointDiff","Track Extrap XY Difference from TRD Point_Hits;x(Extrap) - x(TRD Point_Hit) [cm];y(Extrap) - y(TRD Point_Hit) [cm]",250,-5.5,5.5,250,-5.5,5.5);
	hExtrapXPointDiffvsTime=new TH2D("ExtrapXPointDiffvsTime","Track Extrap X Difference from TRD Point_Hits vs Drift Time;x(Extrap) - x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,400,0.,200.*8);
    hExtrapYPointDiffvsTime=new TH2D("ExtrapYPointDiffvsTime","Track Extrap Y Difference from TRD Point_Hits vs Drift Time;y(Extrap) - y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,400,0.,200.*8);
	hExtrapXPointDiffvsX=new TH2D("ExtrapXPointDiffvsX","Track Extrap X Difference from TRD Point_Hits vs X;x(Extrap) - x(TRD Point_Hit) [cm]; x(TRD Point_Hit) [cm]",250,-5.5,5.5,750,-85.,-10.);
    hExtrapYPointDiffvsY=new TH2D("ExtrapYPointDiffvsY","Track Extrap Y Difference from TRD Point_Hits vs Y;y(Extrap) - y(TRD Point_Hit);y(TRD Point_Hit) [cm]",250,-5.5,5.5,400,-70.,-30.);
	hExtrapXHitDiffvsTime=new TH2D("ExtrapXHitDiffvsTime","Track Extrap X Difference from TRD Hits vs Drift Time;x(Extrap) - x(TRD Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,400,0.,200.*8);
	hExtrapYHitDiffvsTime=new TH2D("ExtrapYHitDiffvsTime","Track Extrap Y Difference from TRD Hits vs Drift Time;y(Extrap) - y(TRD Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,200,0.,200.*8);
	hResXHitDiffvsTime=new TH2D("ResXHitDiffvsTime","Track Extrap X Difference from TRD Hits vs Drift Time;x(Extrap) - x(TRD Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,200,0.,200.*8);
    hResYHitDiffvsTime=new TH2D("ResYHitDiffvsTime","Track Extrap Y Difference from TRD Hits vs Drift Time;y(Extrap) - y(TRD Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,200,0.,200.*8);
	hResXHitDiff_Corrected=new TH1D("ResXHitDiff_Corrected","Corrected Track Extrap X Difference from TRD Hits;x(Extrap) - x(TRD Hit) [cm]",250,-5.5,5.5);
	hResYHitDiff_Corrected=new TH1D("ResYHitDiff_Corrected","Corrected Track Extrap Y Difference from TRD Hits;y(Extrap) - y(TRD Hit) [cm]",250,-5.5,5.5);
	
	hTRDExtrapXY=new TH2D("TRDExtrapXY","2D Display for Track Extrap at TRD Plane;x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
    hExtrapPx=new TH1D("ExtrapPx","Track Extrap at TRD Plane X Mom.;Extrap Px [GeV/c]",200,-3.,3.);
    hExtrapPy=new TH1D("ExtrapPy","Track Extrap at TRD Plane Y Mom.;Extrap Py [GeV/c]",200,-3.,3.);
    hExtrapPz=new TH1D("ExtrapPz","Track Extrap at TRD Plane Z Mom.;Extrap Pz [GeV/c]",240,-0.1,11.9);
	hTRDXCorr=new TH2D("TRDXCorr","X Corr. Between Track Extrap and TRD Point_Hits;x(Track Extrap.) [cm];x(TRD Point_Hit) [cm]",750,-85.,-10.,750,-85.,-10.);
    hTRDYCorr=new TH2D("TRDYCorr","Y Corr. Between Track Extrap and TRD Point_Hits;y(Track Extrap.) [cm];y(TRD Point_Hit) [cm]",400,-70.,-30.,400,-70.,-30.);
	hTRDXCorr_Cut=new TH2D("TRDXCorr_Cut","X Corr. Between Track Extrap and TRD Point_Hits with Track Found;x(Track Extrap.) [cm];x(TRD Point_Hit) [cm]",750,-85.,-10.,750,-85.,-10.);
    hTRDYCorr_Cut=new TH2D("TRDYCorr_Cut","Y Corr. Between Track Extrap and TRD Point_Hits with Track Found;y(Track Extrap.) [cm];y(TRD Point_Hit) [cm]",400,-70.,-30.,400,-70.,-30.);
	
    hFCALExtrapPx=new TH1D("FCALExtrapPx","Track Extrap at FCAL Plane X Mom.;Extrap Px [GeV/c]",200,-3.,3.);
    hFCALExtrapPy=new TH1D("FCALExtrapPy","Track Extrap at FCAL Plane Y Mom.;Extrap Py [GeV/c]",200,-3.,3.);
    hFCALExtrapPz=new TH1D("FCALExtrapPz","Track Extrap at FCAL Plane Z Mom.;Extrap Pz [GeV/c]",240,-0.1,11.9);
	
	hFCALExtrapXY=new TH2D("FCALExtrapXY","2D Display for Track Extrap at FCAL Plane (all p);x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALShowerXY=new TH2D("FCALShowerXY","2D Display for FCAL Shower (all p);x(FCAL Shower) [cm];y(FCAL Shower) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALExtrapE=new TH1D("FCALExtrapE","FCAL Track Extrap Energy;E [GeV]",240,-0.1,11.9);
	hFCALExtrapEP=new TH1D("FCALExtrapEP","FCAL Track Extrap E / P;E / P [GeV/c]",600,-0.1,2.9);
	hFCALExtrapEPvsP=new TH2D("FCALExtrapEPvsP","FCAL Track Extrap E / P vs P;E / P [GeV/c]; P [GeV/c]",600,-0.1,2.9,480,-0.1,11.9);
	hFCALExtrapTheta=new TH1D("FCALExtrapTheta","FCAL Track Extrap Angle at FCAL Z Plane;Theta [Degrees]",350,-0.1,34.9);
	hFCALExtrapThetavsP=new TH2D("FCALExtrapThetavsP","FCAL Track Extrap Angle (at FCAL Z Plane) vs P;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hFCALExtrapThetavsP_Selected=new TH2D("FCALExtrapThetavsP_Selected","FCAL Track Extrap Angle (at FCAL Z Plane) vs P for Selected E/P vs P;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hFCALXCorr=new TH2D("FCALXCorr","X Corr. Between FCAL Shower and Track Extrap at FCAL Plane;x(Track Extrap) [cm];x(FCAL Shower) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALYCorr=new TH2D("FCALYCorr","Y Corr. Between FCAL Shower and Track Extrap at FCAL Plane;y(Track Extrap) [cm];y(FCAL Shower) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALXDiff=new TH1D("FCALXDiff","X Position Difference Between FCAL Shower and Track Extrap at FCAL Plane;x(Track Extrap) - x(FCAL Shower) [cm]",320,-16.,16.);
	hFCALYDiff=new TH1D("FCALYDiff","Y Position Difference Between FCAL Shower and Track Extrap at FCAL Plane;y(Track Extrap) - y(FCAL Shower) [cm]",320,-16.,16.);
	hFCALShowerTime=new TH1D("FCALShowerTime","FCAL Shower Time;Time [ns]",210,-6.,99.);
	hFCALExtrapTime=new TH1D("FCALExtrapTime","FCAL Plane Track Extrap Time;Time [ns]",210,-6.,99.);
	hFCALFlightTime=new TH1D("FCALFlightTime","FCAL Match Flight Time;Time [ns]",210,-6.,99.);
	hFCALTimeCorr=new TH2D("FCALTimeCorr","Time Corr. Between FCAL Shower and Track Flight Time;Time (FCAL Shower) [ns];Time (Track Flight Time) [ns]",210,-6.,99.,210,-6.,99.);
	hFCALExtrapXY_TRD=new TH2D("FCALExtrapXY_TRD","2D Display for Track Extrap at FCAL Plane with Extrap in TRD (all p);x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALShowerXY_TRD=new TH2D("FCALShowerXY_TRD","2D Display for FCAL Shower with Track Extrap in TRD (all p);x(FCAL Shower) [cm];y(FCAL Shower) [cm]",280,-140.,140.,140,-140.,0.);
	//hFCALExtrapXY_p1_TRD=new TH2D("FCALExtrapXY_p1_TRD","2D Display for FCAL Shower with Track Extrap in TRD (p>1); x(FCAL Shower) [cm]; y(FCAL Shower) [cm]",280,-140.,140.,140,-140.,0.);
	hFCALExtrapE_TRD=new TH1D("FCALExtrapE_TRD","FCAL Shower Energy w Extrap in TRD;E [GeV]",240,-0.1,11.9);
    hFCALExtrapEP_TRD=new TH1D("FCALExtrapEP_TRD","FCAL Track Extrap E/P with Extrap in TRD;(FCAL Shower E) / (Track Hyp. P)",600,-0.1,2.9);
    hFCALExtrapEPvsP_TRD=new TH2D("FCALExtrapEPvsP_TRD","FCAL Track Extrap E / P vs P with Extrap in TRD;E / P [GeV/c]; P [GeV/c]",600,-0.1,2.9,480,-0.1,11.9);
	hFCALExtrapTheta_TRD=new TH1D("FCALExtrapTheta_TRD","FCAL Track Extrap Angle at FCAL Z Plane (with Extrap in TRD);Theta [Degrees]",350,-0.1,34.9);
    hFCALExtrapThetavsP_TRD=new TH2D("FCALExtrapThetavsP_TRD","FCAL Track Extrap Angle (at FCAL Z Plane) vs P (with Extrap in TRD);Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hFCALExtrapE_ext=new TH1D("FCALExtrapE_ext","FCAL Track Extrap Energy with External Electron Req.;E [GeV]",240,-0.1,11.9);
    hFCALExtrapEP_ext=new TH1D("FCALExtrapEP_ext","FCAL Track Extrap E / P with External Electron Req.;E / P [GeV/c]",600,-0.1,2.9);
    hFCALExtrapEPvsP_ext=new TH2D("FCALExtrapEPvsP_ext","FCAL Track Extrap E / P vs P with External Electron Req.;E / P [GeV/c]; P [GeV/c]",600,-0.1,2.9,480,-0.1,11.9);
	hFCALExtrapThetavsP_Selected_ext=new TH2D("FCALExtrapThetavsP_Selected_ext","FCAL Track Extrap Angle vs P for Selected E/P vs P w Ext. Electron Req.;Theta [Degrees];P     [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	
	hnumPointsSeen=new TH1D("numPointsSeen","Num Points Counted for a Track Extrap Seen in TRD; # Point_Hits",35,-0.5,34.5);
	hSeenPointsXY=new TH2D("seenPointsXY","2D Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
	hSeenPointsSingleXY=new TH2D("seenPointsSingleXY","2D Single Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
	hSeenPointsSingleX=new TH1D("seenPointsSingleX","Single Point X Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]",75,-85.,-10.);
	hSeenPointsSingleY=new TH1D("seenPointsSingleY","Single Point Y Display for Track Extraps Seen in TRD;y(Point_Hits) [cm]",40,-70.,-30.);
	hExtrapsXY=new TH2D("ExtrapsXY","2D Display for Track Extraps that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",75,-85.,-10.,40,-70.,-30.);
	hExtrapsX=new TH1D("ExtrapsX","Track X Extraps that Pass Through TRD; x(Track Extrap.) [cm]",75,-85.,-10.);
    hExtrapsY=new TH1D("ExtrapsY","Track Y Extraps that Pass Through TRD; y(Track Extrap.) [cm]",40,-70.,-30.);
    hSeenHitsSingleXY=new TH2D("seenHitsSingleXY","2D Single Hit Display for Track Extraps Seen in TRD; x(Hits) [cm]; y(Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenHitsSingleX=new TH1D("seenHitsSingleX","Single Hit X Display for Track Extraps Seen in TRD;x(Hits) [cm]",75,-85.,-10.);
    hSeenHitsSingleY=new TH1D("seenHitsSingleY","Single Hit Y Display for Track Extraps Seen in TRD;y(Hits) [cm]",40,-70.,-30.);
	
	
	//--Electron Subdirectory
	gDirectory->mkdir("Electron")->cd();
    hHypEnergy_el=new TH1D("HypEnergy_el","Electron Hyp. Track Energy;E [GeV]",240,-0.1,11.9);
    hHypEnergyDiff_el=new TH1D("HypEnergyDiff_el","FCAL Shower Energy - Electron Hyp. Track Energy;(El Hyp. Track E) - (FCAL E) [GeV]",200,-5.,5.);
    hHypMomentum_el=new TH1D("HypMomentum_el","Electron Hyp. Track Mom.;P [GeV/c]",240,-0.1,11.9);
    hHypTheta_el=new TH1D("HypTheta_el","Electron Hyp. Track Theta (at origin);Theta [Degrees]",350,-0.1,34.9);
	
	hFCALEP_TRD_el=new TH1D("FCALEP_TRD_el","FCAL Track Extrap E/P with Extrap in TRD;(FCAL E) / (Track Extrap P)",600,-0.1,2.9);
	hFCALEP_cut_el=new TH1D("FCALEP_cut_el","FCAL Track Extrap E/P with Extrap in TRD and 0.8<p<1.25;(FCAL E) / (Track Extrap P)",600,-0.1,2.9);
	hFCALMatchXYDisplay_el=new TH2D("FCALMatchXYDisplay_el","2D FCAL Track Match Display with Extrap in TRD and 0.8<p<1.25;x(FCAL) [cm];y(FCAL) [cm]",280,-140.,140.,140,-140.,0.);
	
	hExtrapXHitDiff_el=new TH1D("ExtrapXHitDiff_el","Track Extrap X Difference from TRD Point_Hits;x(Track Extrap) - x(TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapYHitDiff_el=new TH1D("ExtrapYHitDiff_el","Track Extrap Y Difference from TRD Point_Hits;y(Track Extrap) - y(TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapXYHitDiff_el=new TH2D("ExtrapXYHitDiff_el","Track Extrap XY Difference from TRD Point_Hits;x(Track Extrap) - x(TRD Point_Hit) [cm];y(Track Extrap) - y(TRD Point_Hit) [cm]",250,-5.5,5.5,250,-5.5,5.5);
	hXHitvsTime_el=new TH2D("XHitvsTime_el","TRD X Plane Point_Hits vs Drift Time for Matched Extrap;x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
	hYHitvsTime_el=new TH2D("YHitvsTime_el","TRD Y Plane Point_Hits vs Drift Time for Matched Extrap;y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
	hXHitvsTime_QW_el=new TH2D("XHitvsTime_QW_el","TRD X Plane Point_Hits vs Drift Time for Matched Extrap (q-weighted);x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
	hYHitvsTime_QW_el=new TH2D("YHitvsTime_QW_el","TRD Y Plane Point_Hits vs Drift Time for Matched Extrap (q-weighted);y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
	hXHitvsTime_Qmax_el=new TH2D("XHitvsTime_Qmax_el","TRD X Plane Max q Point_Hit vs Drift Time for Matched Extrap ;x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
	hYHitvsTime_Qmax_el=new TH2D("YHitvsTime_Qmax_el","TRD Y Plane Max q Point_Hits vs Drift Time for Matched Extrap;y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
	hXHitvsTime_Qmax_QW_el=new TH2D("XHitvsTime_Qmax_QW_el","TRD X Plane Max q Point_Hit vs Drift Time for Matched Extrap (q-weighted);x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
	hYHitvsTime_Qmax_QW_el=new TH2D("YHitvsTime_Qmax_QW_el","TRD Y Plane Max q Point_Hit vs Drift Time for Matched Extrap (q-weighted);y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
	
	hXHitvsTime_Converted_el=new TH2D("XHitvsTime_Converted_el","TRD X Plane Point_Hits vs Drift Time for Matched Extrap;8*(Peak Time) [ns];x(TRD Point_Hit) [cm]",400,0.,200.*8,750,-85.,-10.);
    hYHitvsTime_Converted_el=new TH2D("YHitvsTime_Converted_el","TRD Y Plane Point_Hits vs Drift Time for Matched Extrap;8*(Peak Time) [ns];y(TRD Point_Hit) [cm]",400,0.,200.*8,550,-70.,-15.);
	hXHitvsTime_Qmax_Converted_el=new TH2D("XHitvsTime_Qmax_Converted_el","TRD X Plane Max q Point_Hit vs Drift Time for Matched Extrap; 8*(Peak Time) [ns];x(TRD Point_Hit) [cm]",400,0.,200.*8,750,-85.,-10.);
    hYHitvsTime_Qmax_Converted_el=new TH2D("YHitvsTime_Qmax_Converted_el","TRD Y Plane Max q Point_Hits vs Drift Time for Matched Extrap; 8*(Peak Time) [ns];y(TRD Point_Hit) [cm]",400,0.,200.*8,550,-70.,-15.);
    hnumPointsSeen_el=new TH1D("numPointsSeen_el","Num Points Counted for a Track Extrap Seen in TRD; # Point_Hits",35,-0.5,34.5);
    hSeenPointsXY_el=new TH2D("seenPointsXY_el","2D Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenPointsSingleXY_el=new TH2D("seenPointsSingleXY_el","2D Single Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
	hSeenPointsSingleX_el=new TH1D("seenPointsSingleX_el","Single Point X Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]",75,-85.,-10.);
	hSeenPointsSingleY_el=new TH1D("seenPointsSingleY_el","Single Point Y Display for Track Extraps Seen in TRD; y(Point_Hits) [cm]",40,-70.,-30.);
    hExtrapsXY_el=new TH2D("ExtrapsXY_el","2D Display for Track Extraps that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",75,-85.,-10.,40,-70.,-30.);
	hExtrapsX_el=new TH1D("ExtrapsX_el","Track X Extraps that Pass Through TRD; x(Track Extrap.) [cm]",75,-85.,-10.);
	hExtrapsY_el=new TH1D("ExtrapsY_el","Track Y Extraps that Pass Through TRD; y(Track Extrap.) [cm]",40,-70.,-30.);
	//hPoint_TimeVsdE_Max_el=new TH2D("hPoint_TimeVsdE_Max_el","TRD Max Point Charge in Time, Electrons;8*(Peak Time) [ns];Max dE [q]",150,0.,1200,500,0.,10000.);
    hPointH_TimeVsdE_Max_el=new TH2D("hPointH_TimeVsdE_Max_el","TRD Max Point_Hit Charge in Time, Electrons;8*(Peak Time) [ns];Max dE [q]",150,0.,1200.,350,0.,3500.);
	
	
	//--Pion Subdirectory
	trdDir->cd();
	gDirectory->mkdir("Pion")->cd();
    hHypEnergy_pi=new TH1D("HypEnergy_pi","Pi- Hyp. Track Energy;E [GeV]",240,-0.1,11.9);
    hHypEnergyDiff_pi=new TH1D("HypEnergyDiff_pi","FCAL Shower Energy - Pi- Hyp. Track Energy;(Pi Hyp. Track E) - (FCAL E) [GeV]",200,-5.,5.);
    hHypMomentum_pi=new TH1D("HypMomentum_pi","Pi- Hyp. Track Mom.;P [GeV/c]",240,-0.1,11.9);
    hHypTheta_pi=new TH1D("HypTheta_pi","Pi- Hyp. Track Theta (at origin);Theta [Degrees]",350,-0.1,34.9);

    hFCALEP_TRD_pi=new TH1D("FCALEP_TRD_pi","FCAL Track Extrap E/P with Extrap in TRD;(FCAL E) / (Track Extrap P)",600,-0.1,2.9);
	hFCALEP_cut_pi=new TH1D("FCALEP_cut_pi","FCAL Track Extrap E/P with Extrap in TRD and p<0.6;(FCAL E) / (Track Extrap P)",600,-0.1,2.9);
    hFCALMatchXYDisplay_pi=new TH2D("FCALMatchXYDisplay_pi","2D FCAL Track Match Display with Extrap in TRD and p<0.6;x(FCAL) [cm];y(FCAL) [cm]",280,-140.,140.,140,-140.,0.);
    
	hExtrapXHitDiff_pi=new TH1D("ExtrapXHitDiff_pi","Track Extrap X Difference from TRD Point_Hits;x(Track Extrap) - x(TRD Point_Hit) [cm]",550,-5.5,5.5);
    hExtrapYHitDiff_pi=new TH1D("ExtrapYHitDiff_pi","Track Extrap Y Difference from TRD Point_Hits;y(Track Extrap) - y(TRD Point_Hit) [cm]",550,-5.5,5.5);
    hExtrapXYHitDiff_pi=new TH2D("ExtrapXYHitDiff_pi","Track Extrap XY Difference from TRD Point_Hits;x(Track Extrap) - x(TRD Point_Hit) [cm];y(Track Extrap) - y(TRD Point_Hit) [cm]",550,-5.5,5.5,550,-5.5,5.5);
    hXHitvsTime_pi=new TH2D("XHitvsTime_pi","TRD X Plane Point_Hits vs Drift Time for Matched Extrap;x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
    hYHitvsTime_pi=new TH2D("YHitvsTime_pi","TRD Y Plane Point_Hits vs Drift Time for Matched Extrap;y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
    hXHitvsTime_QW_pi=new TH2D("XHitvsTime_QW_pi","TRD X Plane Point_Hits vs Drift Time for Matched Extrap (q-weighted);x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
    hYHitvsTime_QW_pi=new TH2D("YHitvsTime_QW_pi","TRD Y Plane Point_Hits vs Drift Time for Matched Extrap (q-weighted);y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
    hXHitvsTime_Qmax_pi=new TH2D("XHitvsTime_Qmax_pi","TRD X Plane Max q Point_Hit vs Drift Time for Matched Extrap;x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
    hYHitvsTime_Qmax_pi=new TH2D("YHitvsTime_Qmax_pi","TRD Y Plane Max q Point_Hit vs Drift Time for Matched Extrap;y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
    hXHitvsTime_Qmax_QW_pi=new TH2D("XHitvsTime_Qmax_QW_pi","TRD X Plane Max q Point_Hit vs Drift Time for Matched Extrap (q-weighted);x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
    hYHitvsTime_Qmax_QW_pi=new TH2D("YHitvsTime_Qmax_QW_pi","TRD Y Plane Max q Point_Hit vs Drift Time for Matched Extrap (q-weighted);y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
	
	hXHitvsTime_Converted_pi=new TH2D("XHitvsTime_Converted_pi","TRD X Plane Point_Hits vs Drift Time for Matched Extrap;8*(Peak Time) [ns];x(TRD Point_Hit) [cm]",400,0.,200.*8,750,-85.,-10.);
    hYHitvsTime_Converted_pi=new TH2D("YHitvsTime_Converted_pi","TRD Y Plane Point_Hits vs Drift Time for Matched Extrap;8*(Peak Time) [ns];y(TRD Point_Hit) [cm]",400,0.,200.*8,550,-70.,-15.);
	hXHitvsTime_Qmax_Converted_pi=new TH2D("XHitvsTime_Qmax_Converted_pi","TRD X Plane Max q Point_Hit vs Drift Time for Matched Extrap; 8*(Peak Time) [ns];x(TRD Point_Hit) [cm]",400,0.,200.*8,750,-85.,-10.);
    hYHitvsTime_Qmax_Converted_pi=new TH2D("YHitvsTime_Qmax_Converted_pi","TRD Y Plane Max q Point_Hits vs Drift Time for Matched Extrap; 8*(Peak Time) [ns];y(TRD Point_Hit) [cm]",400,0.,200.*8,550,-70.,-15.);
    hnumPointsSeen_pi=new TH1D("numPointsSeen_pi","Num Points Counted for a Track Extrap Seen in TRD; # Point_Hits",35,-0.5,34.5);
    hSeenPointsXY_pi=new TH2D("seenPointsXY_pi","2D Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenPointsSingleXY_pi=new TH2D("seenPointsSingleXY_pi","2D Single Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenPointsSingleX_pi=new TH1D("seenPointsSingleX_pi","Single Point X Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]",75,-85.,-10.);
    hSeenPointsSingleY_pi=new TH1D("seenPointsSingleY_pi","Single Point Y Display for Track Extraps Seen in TRD; y(Point_Hits) [cm]",40,-70.,-30.);
    hExtrapsXY_pi=new TH2D("ExtrapsXY_pi","2D Display for Track Extraps that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",75,-85.,-10.,40,-70.,-30.);
    hExtrapsX_pi=new TH1D("ExtrapsX_pi","Track X Extrapolations that Pass Through TRD; x(Track Extrap.) [cm]",75,-85.,-10.);
    hExtrapsY_pi=new TH1D("ExtrapsY_pi","Track Y Extrapolations that Pass Through TRD; y(Track Extrap.) [cm]",40,-70.,-30.);
	//hPoint_TimeVsdE_Max_pi=new TH2D("hPoint_TimeVsdE_Max_pi","TRD Max Point Charge in Time, Pions;8*(Peak Time) [ns];Max dE [q]",150,0.,1200,500,0.,10000.);
    hPointH_TimeVsdE_Max_pi=new TH2D("hPointH_TimeVsdE_Max_pi","TRD Max Point_Hit Charge in Time, Pions;8*(Peak Time) [ns];Max dE [q]",150,0.,1200.,350,0.,3500.);
	
	
	//New efficiency array plots
	trdDir->cd();
	gDirectory->mkdir("Efficiency")->cd();
	
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
    	//effDir->cd();
	    hSeenHitsSingleX_fine[i] = new TH1F(Form("seenHitsSingleX_fine_%i", i+1),
                                    Form("Single Hit X Display for Track Extraps Seen in TRD (%i - %i)",i*20000,(i+1)*20000),75,-85.,-10.);
		hSeenHitsSingleY_fine[i] = new TH1F(Form("seenHitsSingleY_fine_%i", i+1),
                                    Form("Single Hit Y Display for Track Extraps Seen in TRD (%i - %i)",i*20000,(i+1)*20000),40,-70.,-30.);
		hSeenPointsSingleX_fine[i] = new TH1F(Form("seenPointsSingleX_fine_%i", i+1),
                                    Form("Single Point X Display for Track Extraps Seen in TRD (%i - %i)",i*20000,(i+1)*20000),75,-85.,-10.);
    	hSeenPointsSingleY_fine[i] = new TH1F(Form("seenPointsSingleY_fine_%i", i+1),
                                    Form("Single Point Y Display for Track Extraps Seen in TRD (%i - %i)",i*20000,(i+1)*20000),40,-70.,-30.);
		hExtrapsX_fine[i] = new TH1F(Form("ExtrapsX_fine_%i", i+1),
                                    Form("Track X Extrapolations that Pass Through TRD (%i - %i)",i*20000,(i+1)*20000),75,-85.,-10.);
    	hExtrapsY_fine[i] = new TH1F(Form("ExtrapsY_fine_%i", i+1),
                                    Form("Track X Extrapolations that Pass Through TRD (%i - %i)",i*20000,(i+1)*20000),40,-70.,-30.);
		hHit_PulseHeight_X[i] = new TH1F(Form("Hit_PulseHeight_X%i", i+1),
                                    Form("GEMTRD X Plane Pulse Height (%i - %i); Pulse Height [fADC Units]",i*20000,(i+1)*20000),350,0.,3500.);
		hHit_PulseHeight_Y[i] = new TH1F(Form("Hit_PulseHeight_Y%i", i+1),
                                    Form("GEMTRD Y Plane Pulse Height (%i - %i); Pulse Height [fADC Units]]",i*20000,(i+1)*20000),350,0.,3500.);
		hHit_DriftTime_X[i] = new TH1F(Form("Hit_DriftTime_X%i", i+1),
                                    Form("GEMTRD X Plane Drift Time (%i - %i); Drift Time [ns]",i*20000,(i+1)*20000),100,100.,900.);
        hHit_DriftTime_Y[i] = new TH1F(Form("Hit_DriftTime_Y%i", i+1),
                                    Form("GEMTRD Y Plane Drift Time (%i - %i); Drift Time [ns]",i*20000,(i+1)*20000),100,100.,900);
	}
	
	mainDir->cd();
	
	//--Initialize Trigger Counts
	PSPair=0;
	DL1Triggers=0;
	startTime=0;
	firstGroup = true;
	NPSTriggers = 0;
	NDL1Triggers = 0;
	NEvents = 0;
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
	vector<const DTRDHit*>hits; 
    event->Get(hits);
	vector<const DChargedTrack*>tracks; 
    event->Get(tracks);
	vector<const DPSPair*>PSPair;
	event->Get(PSPair);
	vector<const DL1Trigger*>Trig;
	event->Get(Trig);
	auto eventNumber = event->GetEventNumber();
	
	if (Trig.size()<1) return;
	NEvents++;
  	lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
	
	int numTracks=0, extrapInTRD=0, goodTracks=0, extrapInTRD_p1=0, singleSeenExtrap=0,  singleSeenExtrapHit=0, extElectron=0, singleSeenExtrap_el=0, singleExtrap_el=0, singleSeenExtrap_pi=0, singleExtrap_pi=0;
	
	
	if (!PSPair.empty()) NPSTriggers += PSPair.size();
	//--NEW
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
	for (const auto& pair : PSPair) {
		//if (NEvents%20000==0) {
			const DPSPair::PSClust* flhit = pair->ee.first; //Left Hit
			const DPSPair::PSClust* frhit = pair->ee.second; //Right Hit
			double t_avg = (flhit->t + frhit->t)/2.;
        	hPSPairTime->Fill(t_avg);
		//}
    }
	
	
	hTrackMult->Fill(tracks.size());
	
  	for (unsigned int j=0; j<tracks.size(); j++) {
    	const DChargedTrackHypothesis *hyp_el=tracks[j]->Get_Hypothesis(Electron);
		if (hyp_el==nullptr) {  hyp_el=tracks[j]->Get_Hypothesis(Positron); }
		bool inTRD = false;
		bool TrackMatch = false;
		bool hitTrackMatchX = false;
		bool hitTrackMatchY = false;
		bool hitTrackMatch = false;
		bool isEnergetic = false;
		bool isGoodTrack = false;
		bool ext_electron = false;
		double extrap_FCALEnergy = 0.;
		
		const DChargedTrackHypothesis *best_hyp=tracks[j]->Get_BestTrackingFOM();
		if (best_hyp!=nullptr) {
			hTrackingFOMChisq->Fill(best_hyp->Get_TrackTimeBased()->chisq);
			hTrackingFOMNdof->Fill(best_hyp->Get_TrackTimeBased()->Ndof);
		}
		
    	if (hyp_el!=nullptr) {
			double p_hyp_el=hyp_el->momentum().Mag();
			shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_el->Get_FCALShowerMatchParams();
			const DTrackTimeBased *time_track=hyp_el->Get_TrackTimeBased();
			vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
        	vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
			
			if (trd_extrapolations.size()>0 && fcal_extrapolations.size()>0 && fcalparms!=nullptr) {
				hFCALXCorr->Fill(fcal_extrapolations[0].position.x(), fcalparms->dFCALShower->getPosition().X());
				hFCALXDiff->Fill(fcal_extrapolations[0].position.x() - fcalparms->dFCALShower->getPosition().X());
				hFCALYCorr->Fill(fcal_extrapolations[0].position.y(), fcalparms->dFCALShower->getPosition().Y());
				hFCALYDiff->Fill(fcal_extrapolations[0].position.y() - fcalparms->dFCALShower->getPosition().Y());
				hFCALShowerTime->Fill(fcalparms->dFCALShower->getTime());
				hFCALExtrapTime->Fill(fcal_extrapolations[0].t);
				hFCALFlightTime->Fill(fcalparms->dFlightTime);
				hFCALTimeCorr->Fill(fcalparms->dFCALShower->getTime(), fcalparms->dFlightTime);
				extrap_FCALEnergy = fcalparms->dFCALShower->getEnergy();
                double p_track = fcal_extrapolations[0].momentum.Mag();
                hHypMomentumDiff->Fill(p_hyp_el - p_track);
				
				
				if (abs(fcal_extrapolations[0].position.x() - fcalparms->dFCALShower->getPosition().X())<5. && abs(fcal_extrapolations[0].position.y() - fcalparms->dFCALShower->getPosition().Y())<5.) {
					isGoodTrack = true;
					goodTracks++;
					hTRDExtrapXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
				}
				
				if ((trd_extrapolations[0].position.x() > -83.47) && (trd_extrapolations[0].position.x() < -11.47) && (trd_extrapolations[0].position.y() > -68.6) && (trd_extrapolations[0].position.y() < -32.61)) {
					inTRD = true;
					extrapInTRD++;
				}
				
                if (extrap_FCALEnergy>0. && p_hyp_el>=1.) {
					isEnergetic = true;
				}
				
				if (isEnergetic && isGoodTrack) {
					if (inTRD) {
					int i1=0;
					extrapInTRD_p1++;
					
					hExtrapPx->Fill(trd_extrapolations[0].momentum.x());
                	hExtrapPy->Fill(trd_extrapolations[0].momentum.y());
                	hExtrapPz->Fill(trd_extrapolations[0].momentum.z());
					hExtrapsXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
					hExtrapsX->Fill(trd_extrapolations[0].position.x());
					hExtrapsY->Fill(trd_extrapolations[0].position.y());
					//NEW dynamic histos
					if (NEvents<2000000) {
                    	hExtrapsX_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.x());
						hExtrapsY_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.y());
					}
					hFCALExtrapE_TRD->Fill(extrap_FCALEnergy);
                    hFCALExtrapEP_TRD->Fill(extrap_FCALEnergy / p_hyp_el);
					hFCALExtrapEPvsP_TRD->Fill(extrap_FCALEnergy / p_hyp_el, p_hyp_el);
                    hFCALShowerXY_TRD->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
                    hFCALExtrapXY_TRD->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
                    hFCALExtrapThetavsP_TRD->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
                    hFCALExtrapTheta_TRD->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta());
					
					for (const auto& point : pointHits) {
						if (abs(trd_extrapolations[0].position.x() - point->x) < 1.5 && abs(trd_extrapolations[0].position.y() - point->y) < 1.5) { //if within 1.5cm
							if (!TrackMatch) {
								singleSeenExtrap++;
								hSeenPointsSingleXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
								hSeenPointsSingleX->Fill(trd_extrapolations[0].position.x());
	                            hSeenPointsSingleY->Fill(trd_extrapolations[0].position.y());
								//NEW dynamic histos
								if (NEvents<2000000) {
									hSeenPointsSingleX_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.x());
									hSeenPointsSingleY_fine[NEvents/20000]->Fill(trd_extrapolations[0].position.y());
								}
							}
							i1++;
							TrackMatch = true;
							hSeenPointsXY->Fill(point->x, point->y);
							hTRDXCorr_Cut->Fill(trd_extrapolations[0].position.x(), point->x);
                        	hTRDYCorr_Cut->Fill(trd_extrapolations[0].position.y(), point->y);
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
					
					for (const auto& hit : hits) {
						//Convert to GlueX coordinates
						double newX = 0.;
						double newY = 0.;
						if (hit->plane == 1) { //X plane
							newX = (-47.4695) + -1.*0.1*(720/2-hit->strip + 0.5);
						} else { //Y Plane
							newY = (-59.0315) + 0.1*(528/2-hit->strip + 0.5);
						}
                        if (hit->plane == 1 && abs(trd_extrapolations[0].position.x() - newX) < 1.5) { //if within 1.5cm
                            if (!hitTrackMatchX) {
                                singleSeenExtrapHit++;
                                hSeenHitsSingleXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
                                hSeenHitsSingleX->Fill(trd_extrapolations[0].position.x());
								//NEW dynamic histos
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
						if (hit->plane == 1) {
						//	hHit_PulseHeight_X[NEvents/20000]->Fill(hit->pulse_height);
						//	hHit_DriftTime_X[NEvents/20000]->Fill(hit->t);
							hExtrapXHitDiffvsTime->Fill(trd_extrapolations[0].position.x() - newX, hit->t);
							if (trd_extrapolations[0].position.x() > -20.) hResXHitDiffvsTime->Fill(trd_extrapolations[0].position.x() - newX, hit->t);
							hResXHitDiff_Corrected->Fill((trd_extrapolations[0].position.x() - newX)-((hit->t - 473.197)/165.83));
						}
						if (hit->plane == 2 && abs(trd_extrapolations[0].position.y() - newY) < 1.5) { //if within 1.5cm
                            if (!hitTrackMatchY) {
                                singleSeenExtrapHit++;
                                hSeenHitsSingleXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
                                hSeenHitsSingleY->Fill(trd_extrapolations[0].position.y());
								//NEW dynamic histos
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
						if (hit->plane == 2) {
						//	hHit_PulseHeight_Y[NEvents/20000]->Fill(hit->pulse_height);
						//	hHit_DriftTime_Y[NEvents/20000]->Fill(hit->t);
							hExtrapYHitDiffvsTime->Fill(trd_extrapolations[0].position.y() - newY, hit->t);
							if (trd_extrapolations[0].position.y() > -38.) hResYHitDiffvsTime->Fill(trd_extrapolations[0].position.y() - newY, hit->t);
							hResYHitDiff_Corrected->Fill((trd_extrapolations[0].position.y() - newY)-((hit->t - 558.016)/439.767));
						}
                        //if (hitTrackMatchX && hitTrackMatchY) hitTrackMatch = true;
					} //--END Hits Loop
					
				} //--END inTRD
				
              	hFCALExtrapPx->Fill(fcal_extrapolations[0].momentum.x());
	            hFCALExtrapPy->Fill(fcal_extrapolations[0].momentum.y());
    	        hFCALExtrapPz->Fill(fcal_extrapolations[0].momentum.z());
                hFCALExtrapXY->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
				hFCALShowerXY->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
				hFCALExtrapThetavsP->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
				hFCALExtrapTheta->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta());

				//--Loop to find additional external electron
				//bool ext_electron = false;
  				for (unsigned int i=0; i<tracks.size(); i++) {
    				const DChargedTrackHypothesis *hyp_el_2=tracks[i]->Get_Hypothesis(Electron);
					if (hyp_el_2==nullptr) { hyp_el_2=tracks[i]->Get_Hypothesis(Positron); }
    				if (hyp_el_2!=nullptr) {
						shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_el_2->Get_FCALShowerMatchParams();
						const DTrackTimeBased *time_track=hyp_el_2->Get_TrackTimeBased();
						vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
        				vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
						
						if (trd_extrapolations.size()>0 && fcal_extrapolations.size()>0 && fcalparms!=nullptr) {
							double extrap_FCALEnergy=fcalparms->dFCALShower->getEnergy();	
    	        			if (extrap_FCALEnergy>0.) {
								hFCALExtrapE->Fill(extrap_FCALEnergy);
        	        			hFCALExtrapEP->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag());
								hFCALExtrapEPvsP->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag(), fcal_extrapolations[0].momentum.Mag());
								
								if (fcal_extrapolations[0].momentum.Mag() >= 0.5 && (extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag()) >= 0.75 && (extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag()) <= 1.3) {
									hFCALExtrapThetavsP_Selected->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
									if(i!=j){ ext_electron = true; extElectron++; }
								}
							}
						}
					}
				}
				
				
				if (inTRD && ext_electron && extrap_FCALEnergy>0.) {
                    hFCALExtrapE_ext->Fill(extrap_FCALEnergy);
                    hFCALExtrapEP_ext->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag());
                    hFCALExtrapEPvsP_ext->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag(), fcal_extrapolations[0].momentum.Mag());
					hFCALExtrapThetavsP_Selected_ext->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
                }
				} //--END isGoodTrack & isEnergetic
        	} //--END if (trd_extrapolations && fcal_extrapolations && fcalparms)
			
//--START ELECTRON FOLDER ---------------			
			
			if (isGoodTrack && inTRD && isEnergetic) {
				double extrap_FCAL_EP = fcalparms->dFCALShower->getEnergy() / p_hyp_el;
				if (TrackMatch) hFCALEP_TRD_el->Fill(extrap_FCAL_EP);
				if (extrap_FCAL_EP>0.80 && extrap_FCAL_EP<1.25) {
					if (TrackMatch) {
						hFCALEP_cut_el->Fill(extrap_FCAL_EP);
						hHypEnergy_el->Fill(hyp_el->energy());
						hHypMomentum_el->Fill(p_hyp_el);
						hHypTheta_el->Fill((180./3.141592)*hyp_el->momentum().Theta()); //theta at origin
					}
					singleExtrap_el++;
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
        		    //float point_maxdE=0.;
            		//float point_maxTime=0.;
					for (const auto& point : pointHits) {
						if (TrackMatch) {
							float QX=point->dE_x;
							float QY=point->dE_y;
							if (QX>QX_max && point->time>340.) { QX_max=QX; XPos_max=point->x; TX_max=point->time; }
							if (QY>QY_max && point->time>340.) { QY_max=QY; YPos_max=point->y; TY_max=point->time; }
							hExtrapXHitDiff_el->Fill(trd_extrapolations[0].position.x() - point->x);
    	    	        	hExtrapYHitDiff_el->Fill(trd_extrapolations[0].position.y() - point->y);
        	    	    	hExtrapXYHitDiff_el->Fill(trd_extrapolations[0].position.x() - point->x, trd_extrapolations[0].position.y() - point->y);
		    	    		hXHitvsTime_el->Fill(point->x, point->time);
                	    	hYHitvsTime_el->Fill(point->y, point->time);
							hXHitvsTime_Converted_el->Fill(point->time, point->x);
                            hYHitvsTime_Converted_el->Fill(point->time, point->y);
							hXHitvsTime_QW_el->Fill(point->x, point->time, point->dE_x);
                    		hYHitvsTime_QW_el->Fill(point->y, point->time, point->dE_y);
			            }
						
						if (abs(trd_extrapolations[0].position.x() - point->x) < 1.5 && abs(trd_extrapolations[0].position.y() - point->y) < 1.5) { //if within 1.5cm
                        	if (i3==0) {
								singleSeenExtrap_el++;
                            	hSeenPointsSingleXY_el->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
								hSeenPointsSingleX_el->Fill(trd_extrapolations[0].position.x());
								hSeenPointsSingleY_el->Fill(trd_extrapolations[0].position.y());
                        	}
                        	i3++;
                        	hSeenPointsXY_el->Fill(point->x, point->y);
                    	}
						if (point->dE > pointH_maxdE) {
                            pointH_maxdE = point->dE;
                            pointH_maxTime = point->time;
                        }
                    } //END Point_Hits loop
                    if (pointH_maxdE > 0.) hPointH_TimeVsdE_Max_el->Fill(pointH_maxTime, pointH_maxdE);
					
					if (TrackMatch) {
				        hXHitvsTime_Qmax_el->Fill(XPos_max, TX_max);
                    	hYHitvsTime_Qmax_el->Fill(YPos_max, TY_max);
						hXHitvsTime_Qmax_QW_el->Fill(XPos_max, TX_max, QX_max);
                    	hYHitvsTime_Qmax_QW_el->Fill(YPos_max, TY_max, QY_max);
						hXHitvsTime_Qmax_Converted_el->Fill(TX_max, XPos_max);
                        hYHitvsTime_Qmax_Converted_el->Fill(TY_max, YPos_max);
			        }
					if (i3 > 0) hnumPointsSeen_el->Fill(i3);
					hFCALMatchXYDisplay_el->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
					hHypEnergyDiff_el->Fill(hyp_el->energy() - fcalparms->dFCALShower->getEnergy());
				} //--END E/P > 0.8 && <1.25 cut
      		} //--END if (fcalparms && inTRD && isEnergetic)
    	} // End Electron Hypothesis

//--START PION FOLDER ------------------------------------------------------
		
		const DChargedTrackHypothesis *hyp_pi=tracks[j]->Get_Hypothesis(PiMinus);
		if (hyp_pi==nullptr) { hyp_pi=tracks[j]->Get_Hypothesis(PiPlus); }
		inTRD = false;
        TrackMatch = false;
		isEnergetic = false;
		isGoodTrack = false;
        extrap_FCALEnergy = 0.;
		
		if (hyp_pi!=nullptr) {
			double p_hyp_pi=hyp_pi->momentum().Mag();
            shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_pi->Get_FCALShowerMatchParams();
			const DTrackTimeBased *time_track=hyp_pi->Get_TrackTimeBased();
            vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
			vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
			
			if (trd_extrapolations.size()>0 && fcal_extrapolations.size()>0 && fcalparms!=nullptr) {
				
				if (abs(fcal_extrapolations[0].position.x() - fcalparms->dFCALShower->getPosition().X())<5. && abs(fcal_extrapolations[0].position.y() - fcalparms->dFCALShower->getPosition().Y())<5.) {
					
					isGoodTrack = true;
				}
				if ((trd_extrapolations[0].position.x() > -83.47) && (trd_extrapolations[0].position.x() < -11.47) && (trd_extrapolations[0].position.y() > -68.6) && (trd_extrapolations[0].position.y() < -32.61)) {
                    inTRD = true;
                }
				
				extrap_FCALEnergy = fcalparms->dFCALShower->getEnergy();
				if (extrap_FCALEnergy>0. && p_hyp_pi>=1.) {
                    isEnergetic = true;
                }
				
				if (inTRD && isEnergetic && isGoodTrack) {
					for (const auto& point : pointHits) {
        		        if (abs(trd_extrapolations[0].position.x() - point->x) < 1.5 && abs(trd_extrapolations[0].position.y() - point->y) < 1.5) { //if within 1.5cm
            		        TrackMatch = true;
                		}
					}
				}
			}
			
            if (isGoodTrack && inTRD && isEnergetic) {
				double extrap_FCAL_EP = fcalparms->dFCALShower->getEnergy() / p_hyp_pi;
                if (TrackMatch) hFCALEP_TRD_pi->Fill(extrap_FCAL_EP);
                if (extrap_FCAL_EP>0. && extrap_FCAL_EP<0.6) {
					if (TrackMatch) {
						hFCALEP_cut_pi->Fill(extrap_FCAL_EP);
    	                hHypEnergy_pi->Fill(hyp_pi->energy());
        	            hHypMomentum_pi->Fill(p_hyp_pi);
            	        hHypTheta_pi->Fill((180./3.141592)*hyp_pi->momentum().Theta());
					}
					singleExtrap_pi++;
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
            		//float point_maxdE=0.;
            		//float point_maxTime=0.;
					for (const auto& point : pointHits) {
						float QX=point->dE_x;
						float QY=point->dE_y;
						if (TrackMatch) {
							if (QX>QX_max && point->time>340.) {QX_max=QX; XPos_max=point->x; TX_max=point->time;}
							if (QY>QY_max && point->time>340.) {QY_max=QY; YPos_max=point->y; TY_max=point->time;}
							
	                        hExtrapXHitDiff_pi->Fill(trd_extrapolations[0].position.x() - point->x);
    	                    hExtrapYHitDiff_pi->Fill(trd_extrapolations[0].position.y() - point->y);
        	                hExtrapXYHitDiff_pi->Fill(trd_extrapolations[0].position.x() - point->x, trd_extrapolations[0].position.y() - point->y);
							hXHitvsTime_pi->Fill(point->x, point->time);
	                        hYHitvsTime_pi->Fill(point->y, point->time);
							hXHitvsTime_Converted_pi->Fill(point->time, point->x);
                            hYHitvsTime_Converted_pi->Fill(point->time, point->y);
							hXHitvsTime_QW_pi->Fill(point->x, point->time, point->dE_x);
                    	    hYHitvsTime_QW_pi->Fill(point->y, point->time, point->dE_y);
			            }
						
						if (abs(trd_extrapolations[0].position.x() - point->x) < 1.5 && abs(trd_extrapolations[0].position.y() - point->y) < 1.5) { //if within 1.5cm
                            if (i5==0) {
								singleSeenExtrap_pi++;
                                hSeenPointsSingleXY_pi->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
								hSeenPointsSingleX_pi->Fill(trd_extrapolations[0].position.x());
                                hSeenPointsSingleY_pi->Fill(trd_extrapolations[0].position.y());
                            }
                            i5++;
                            hSeenPointsXY_pi->Fill(point->x, point->y);
                        }
                    	if (point->dE > pointH_maxdE) {
                            pointH_maxdE = point->dE;
                            pointH_maxTime = point->time;
                        }
                    } //END Point_Hits loop
                    if (pointH_maxdE > 0.) hPointH_TimeVsdE_Max_pi->Fill(pointH_maxTime, pointH_maxdE);
					
					
					if (TrackMatch) {
				        hXHitvsTime_Qmax_pi->Fill(XPos_max, TX_max);
                    	hYHitvsTime_Qmax_pi->Fill(YPos_max, TY_max);
						hXHitvsTime_Qmax_QW_pi->Fill(XPos_max, TX_max, QX_max);
                    	hYHitvsTime_Qmax_QW_pi->Fill(YPos_max, TY_max, QY_max);
						hXHitvsTime_Qmax_Converted_pi->Fill(TX_max, XPos_max);
                        hYHitvsTime_Qmax_Converted_pi->Fill(TY_max, YPos_max);
			        }
					if (i5 > 0) hnumPointsSeen_pi->Fill(i5);
                    hFCALMatchXYDisplay_pi->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
                    hHypEnergyDiff_pi->Fill(hyp_pi->energy() - fcalparms->dFCALShower->getEnergy());
                } //--END E/P < 0.5 cut
            } //--END fcalparms && inTRD
        } //--END Pion Hypothesis
		
  	} //--END Track Loop
	if (numTracks>0) Count("nTrack");
	//if (goodTracks>0) Count("nGoodTrack");
	if (extrapInTRD>0) Count("ExtrapInTRD");
	if (extrapInTRD_p1>0) Count("ExtrapInTRD_p1");
	if (singleSeenExtrap>0) Count("ExtrapSeen_p1");
	if (singleSeenExtrapHit>0) Count("ExtrapSeen_p1_Hits");
	if (extElectron>0) Count("ext_El");
	if (singleExtrap_el>0) Count("el_extrap");
	if (singleSeenExtrap_el>0) Count("el_extrapSeen");
	if (singleExtrap_pi>0) Count("pi_extrap");
    if (singleSeenExtrap_pi>0) Count("pi_extrapSeen");
	
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

