// $Id$
//
//    File: JEventProcessor_TRDTrack.cc
// Created: Thu Mar 27 02:20:19 PM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

#include "JEventProcessor_TRDTrack.h"
#include <PID/DChargedTrack.h>
#include <TDirectory.h>

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
	hnumTracks=new TH1D("hnumTracks","Num Charged Tracks;Num Tracks",12,0.5,12.5);
	hnumTracksGoodExtrap=new TH1D("hnumTracksGoodExtrap","Num Charged Tracks with Extrap at TRD and FCAL Plane;Num Tracks",12,0.5,12.5);
	hnumTracksInTRD=new TH1D("hnumTracksInTRD","Num Charged Tracks with Extrap Inside TRD Active Area;Num Tracks",12,0.5,12.5);
	//hnumTrackMatches=new TH1D("hnumTrackMatches","Num Tracks Matched with TRD;Num Tracks",3,-0.5,2.5);
	//hExtrapXDiff=new TH1D("ExtrapXDiff","Track Extrapolation X Difference from TRD Points;x(Extrap) - x(TRD Point) [cm]",500,-5.,5.);
    //hExtrapYDiff=new TH1D("ExtrapYDiff","Track Extrapolation Y Difference from TRD Points;y(Extrap) - y(TRD Point) [cm]",500,-5.,5.);
    //hExtrapXYDiff=new TH2D("ExtrapXYDiff","Track Extrapolation XY Difference from TRD Points;x(Extrap) - x(TRD Point) [cm];y(Extrap) - y(TRD Point) [cm]",500,-5.,5.,500,-5.,5.);
    hExtrapXHitDiff=new TH1D("ExtrapXHitDiff","Track Extrapolation X Difference from TRD Point_Hits;x(Extrap) - x(TRD Point_Hit) [cm]",500,-5.,5.);
    hExtrapYHitDiff=new TH1D("ExtrapYHitDiff","Track Extrapolation Y Difference from TRD Point_Hits;y(Extrap) - y(TRD Point_Hit) [cm]",500,-5.,5.);
    hExtrapXYHitDiff=new TH2D("ExtrapXYHitDiff","Track Extrapolation XY Difference from TRD Point_Hits;x(Extrap) - x(TRD Point_Hit) [cm];y(Extrap) - y(TRD Point_Hit) [cm]",500,-5.,5.,500,-5.,5.);
	hExtrapXHitDiffvsTime=new TH2D("ExtrapXHitDiffvsTime","Track Extrapolation X Difference from TRD Point_Hits vs Drift Time;x(Extrap) - x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",500,-5.,5.,400,0.,200.*8);
	hExtrapYHitDiffvsTime=new TH2D("ExtrapYHitDiffvsTime","Track Extrapolation Y Difference from TRD Point_Hits vs Drift Time;y(Extrap) - y(TRD Point_Hit)     [cm]; 8*(Peak Time) [ns]",500,-5.,5.,400,0.,200.*8);
	//hSegmentExtrapXDiff=new TH1D("SegmentExtrapXDiff","Track Extrapolation X Difference from TRD Segment;x(Extrap) - x(TRD Segment) [cm]",500,-5.,5.);
    //hSegmentExtrapYDiff=new TH1D("SegmentExtrapYDiff","Track Extrapolation Y Difference from TRD Segment;y(Extrap) - y(TRD Segment) [cm]",500,-5.,5.);
    //hSegmentExtrapXYDiff=new TH2D("SegmentExtrapXYDiff","Track Extrapolation XY Difference from TRD Segment;x(Extrap) - x(TRD Segment) [cm];y(Extrap) - y(TRD Segment) [cm]",500,-5.,5.,500,-5.,5.);
	hTRDExtrapXY=new TH2D("TRDExtrapXY","Track Extrapolation XY Display at TRD Plane;x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
    hTRDExtrapPx=new TH1D("TRDExtrapPx","TRD Track Extrapolation X Mom.;Px [GeV/c]",200,-3.,3.);
    hTRDExtrapPy=new TH1D("TRDExtrapPy","TRD Track Extrapolation Y Mom.;Py [GeV/c]",200,-3.,3.);
    hTRDExtrapPz=new TH1D("TRDExtrapPz","TRD Track Extrapolation Z Mom.;Pz [GeV/c]",240,-0.1,11.9);
	
	hFCALExtrapXY=new TH2D("FCALExtrapXY","Track Extrapolation XY Display at FCAL Plane;x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
    hFCALExtrapPx=new TH1D("FCALExtrapPx","FCAL Track Extrapolation X Mom.;Px [GeV/c]",200,-3.,3.);
    hFCALExtrapPy=new TH1D("FCALExtrapPy","FCAL Track Extrapolation Y Mom.;Py [GeV/c]",200,-5.,5.);
    hFCALExtrapPz=new TH1D("FCALExtrapPz","FCAL Track Extrapolation Z Mom.;Pz [GeV/c]",240,-0.1,11.9);
	hExtrapXDiff_FCAL=new TH1D("ExtrapXDiff_FCAL","Track Extrapolation X Difference between TRD and FCAL;x(FCAL) - x(TRD Point_Hit) [cm]",500,-5.,5.);
    hExtrapYDiff_FCAL=new TH1D("ExtrapYDiff_FCAL","Track Extrapolation Y Difference between TRD and FCAL;y(FCAL) - y(TRD Point_Hit) [cm]",500,-5.,5.);
    hExtrapXYDiff_FCAL=new TH2D("ExtrapXYDiff_FCAL","Track Extrapolation XY Difference between TRD and FCAL;x(FCAL) - x(TRD Point_Hit) [cm];y(FCAL) - y(TRD Point_Hit) [cm]",500,-5.,5.,500,-5.,5.);
	hFCALExtrapE=new TH1D("FCALExtrapE","FCAL Track Extrapolation Energy;E [GeV]",240,-0.1,11.9);
	hFCALExtrapEP=new TH1D("FCALExtrapEP","FCAL Track Extrapolation E / P;E / P [GeV/c]",600,-0.1,2.9);
	hFCALExtrapEPvsP=new TH2D("FCALExtrapEPvsP","FCAL Track Extrapolation E / P vs P;E / P [GeV/c]; P [GeV/c]",600,-0.1,2.9,480,-0.1,11.9);
	hFCALShowerDisplay=new TH2D("FCALShowerDisplay","fCAL Shower Extrapolation XY Display;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
	hExtrapTheta=new TH1D("ExtrapTheta","FCAL Track Extrapolation Angle;Theta [Degrees]",350,-0.1,34.9);
	hExtrapThetavsP=new TH2D("ExtrapThetavsP","FCAL Track Extrapolation Angle vs P;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hExtrapThetavsP_Selected=new TH2D("ExtrapThetavsP_Selected","FCAL Track Extrapolation Angle vs P for Selected EP vs P Corr.;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hTRDXCorr=new TH2D("TRDXCorr","X Corr. Between Track Extrapolation and TRD Point_Hits;x(Track Extrap.) [cm];x(TRD Point_Hit) [cm]",750,-85.,-10.,750,-85.,-10.);
	hTRDYCorr=new TH2D("TRDYCorr","Y Corr. Between Track Extrapolation and TRD Point_Hits;y(Track Extrap.) [cm];y(TRD Point_Hit) [cm]",400,-70.,-30.,400,-70.,-30.);
	hFCALExtrapXY_TRD=new TH2D("FCALExtrapXY_TRD","Track Extrapolation XY Display at FCAL Plane;x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
	hExtrapXDiff_FCAL_TRD=new TH1D("ExtrapXDiff_FCAL_TRD","Track Extrapolation X Difference between TRD and FCAL;x(FCAL) - x(TRD Point_Hit) [cm]",500,-5.,5.);
    hExtrapYDiff_FCAL_TRD=new TH1D("ExtrapYDiff_FCAL_TRD","Track Extrapolation Y Difference between TRD and FCAL;y(FCAL) - y(TRD Point_Hit) [cm]",500,-5.,5.);
    hExtrapXYDiff_FCAL_TRD=new TH2D("ExtrapXYDiff_FCAL_TRD","Track Extrapolation XY Difference between TRD and FCAL;x(FCAL) - x(TRD Point_Hit) [cm];y(FCAL) - y(TRD Point_Hit) [cm]",500,-5.,5.,500,-5.,5.);
    hFCALExtrapE_TRD=new TH1D("FCALExtrapE_TRD","FCAL Track Extrapolation Energy;E [GeV]",240,-0.1,11.9);
    hFCALExtrapEP_TRD=new TH1D("FCALExtrapEP_TRD","FCAL Track Extrapolation E / P;E / P [GeV/c]",600,-0.1,2.9);
    hFCALExtrapEPvsP_TRD=new TH2D("FCALExtrapEPvsP_TRD","FCAL Track Extrapolation E / P vs P;E / P [GeV/c]; P [GeV/c]",600,-0.1,2.9,480,-0.1,11.9);
	hExtrapTheta_TRD=new TH1D("ExtrapTheta_TRD","FCAL Track Extrapolation Angle;Theta [Degrees]",350,-0.1,34.9);
    hExtrapThetavsP_TRD=new TH2D("ExtrapThetavsP_TRD","FCAL Track Extrapolation Angle vs P;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
    hExtrapThetavsP_Selected_TRD=new TH2D("ExtrapThetavsP_Selected_TRD","FCAL Track Extrapolation Angle vs P for Selected EP vs P Corr.;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hnumSeenExtrap=new TH1D("numSeenExtrap","Num Track Extrapolations Seen in TRD; # Track Extrap.",3,-0.5,2.5);
	hnumPointsSeen=new TH1D("numPointsSeen","Num Points Counted for a Track Extrapolation Seen in TRD; # Point_Hits",35,-0.5,34.5);
	hSeenPointsXY=new TH2D("seenPointsXY","2D Point Display for Track Extrapolations Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
	hSeenPointsSingleXY=new TH2D("seenPointsSingleXY","2D Single Point Display for Track Extrapolations Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
	hSeenPointsSingleX=new TH1D("seenPointsSingleX","Single Point X Display for Track Extrapolations Seen in TRD; x(Point_Hits) [cm]",75,-85.,-10.);
	hSeenPointsSingleY=new TH1D("seenPointsSingleY","Single Point Y Display for Track Extrapolations Seen in TRD;y(Point_Hits) [cm]",40,-70.,-30.);
	hnumExtrap=new TH1D("numExtrap","Num Track Extrapolations that Enter TRD; # Track Extrap.",3,-0.5,2.5);
	hExtrapsXY=new TH2D("ExtrapsXY","2D Display for Track Extrapolations that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",75,-85.,-10.,40,-70.,-30.);
	hExtrapsX=new TH1D("ExtrapsX","Track X Extrapolations that Pass Through TRD; x(Track Extrap.) [cm]",75,-85.,-10.);
    hExtrapsY=new TH1D("ExtrapsY","Track Y Extrapolations that Pass Through TRD; y(Track Extrap.) [cm]",40,-70.,-30.);
	hnumSeenExtrapFCAL=new TH1D("numSeenExtrapFCAL","Num Track Extrapolations Seen at FCAL; # Track Extrap.",3,-0.5,2.5);
    hnumPointsSeenFCAL=new TH1D("numPointsSeenFCAL","Num Points Counted for a Track Extrapolation Seen at FCAL; # Point_Hits",35,-0.5,34.5);
    hSeenPointsFCALXY=new TH2D("seenPointsFCALXY","2D Point Display for Track Extrapolations Seen at FCAL; x(Point_Hits) [cm]; y(Point_Hits) [cm]",750,-85.,-10.,400,-70.,-30.);
    hExtrapsFCALXY=new TH2D("ExtrapsFCALXY","2D Display for Track Extrapolations Seen at FCAL; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",750,-85.,-10.,400,-70.,-30.);
	
	
	//--Electron Subdirectory
	gDirectory->mkdir("Electron")->cd();
	hnumElTracks=new TH1D("numElTracks","Num Electron Hyp. Tracks; # Tracks",12,-0.5,12-0.5);
    hTRDEnergy_el=new TH1D("TRDEnergy_el","Electron Hyp. Track Energy;E [GeV]",240,-0.1,11.9);
    hTRDEnergyDiff_el=new TH1D("TRDEnergyDiff_el","fCAL Energy - Electron Hyp. Track Energy;(fCAL E) - (El Hyp. Track E) [GeV]",600,-3.,3.);
    hTRDMomentum_el=new TH1D("TRDMomentum_el","Electron Hyp. Track Mom.;P [GeV/c]",240,-0.1,11.9);
    hTRDTheta_el=new TH1D("TRDTheta_el","Electron Hyp. Track Theta;Theta [Degrees]",350,-0.1,34.9);
	
	hfCALEP_TRD_el=new TH1D("fCALEP_TRD_el","fCAL Track Projection E/P with Extrap in TRD;(fCAL E) / (Measured Track P)",600,-0.1,2.9);
	hfCALEP_cut_el=new TH1D("fCALEP_cut_el","fCAL Track Projection E/P with Extrap in TRD and Cut >0.85;(fCAL E) / (Measured Track P)",600,-0.1,2.9);
	hfCALEP_el=new TH1D("fCALEP_el","fCAL Track Projection E/P (Electron Hyp.);(fCAL E) / (Track P)",600,-0.1,2.9);
	hfCALShower_el=new TH1D("fCALShower_el","fCAL Shower Energy;E [GeV]",240,-0.1,11.9);
	hfCALMatchXDiff_el=new TH1D("fCALMatchXDiff_el","fCAL Track Match X Difference from TRD;x(fCAL) - x(TRD Point_Hits) [cm]",270,-22.,5.);
	hfCALMatchYDiff_el=new TH1D("fCALMatchYDiff_el","fCAL Track Match Y Difference from TRD;y(fCAL) - y(TRD Point_Hits) [cm]",270,-22.,5.);
	hfCALXY_el=new TH2D("fCALXY_el","2D fCAL Track Match Display with Extrap in TRD ;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
	hfCALMatchXYDisplay_el=new TH2D("fCALMatchXYDisplay_el","2D fCAL Track Match Display with Extrap in TRD and Cut >0.85;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
	hfCALMatchXYDiff_el=new TH2D("fCALMatchXYDiff_el","2D fCAL Track Match Difference from TRD;x(fCAL) - x(TRD Point_Hits) [cm];y(fCAL) - y(TRD Point_Hits) [cm]",270,-22.,5.,270,-22.,5.);
	
	//hProjectionXDiff_el=new TH1D("ProjectionXDiff_el","Track Projection X Difference from TRD Points;x(Proj) - x(TRD Point) [cm]",500,-5.,5.);
	//hProjectionYDiff_el=new TH1D("ProjectionYDiff_el","Track Projection Y Difference from TRD Points;y(Proj) - y(TRD Point) [cm]",500,-5.,5.);
	//hProjectionXYDiff_el=new TH2D("ProjectionXYDiff_el","Track Projection XY Difference from TRD Points;x(Proj) - x(TRD Point) [cm];y(Proj) - y(TRD Point) [cm]",500,-5.,5.,500,-5.,5.);
	hProjectionXHitDiff_el=new TH1D("ProjectionXHitDiff_el","Track Projection X Difference from TRD Point_Hits;x(Proj) - x(TRD Point_Hit) [cm]",500,-5.,5.);
    hProjectionYHitDiff_el=new TH1D("ProjectionYHitDiff_el","Track Projection Y Difference from TRD Point_Hits;y(Proj) - y(TRD Point_Hit) [cm]",500,-5.,5.);
    hProjectionXYHitDiff_el=new TH2D("ProjectionXYHitDiff_el","Track Projection XY Difference from TRD Point_Hits;x(Proj) - x(TRD Point_Hit) [cm];y(Proj) - y(TRD Point_Hit) [cm]",500,-5.,5.,500,-5.,5.);
	hProjectionXHitvsTime_el=new TH2D("ProjectionXHitvsTime_el","TRD Point_Hits vs Drift Time;x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
	hProjectionYHitvsTime_el=new TH2D("ProjectionYHitvsTime_el","TRD Point_Hits vs Drift Time;y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
	hProjectionXHitvsTime_QW_el=new TH2D("ProjectionXHitvsTime_QW_el","TRD Point_Hits vs Drift Time (q-weighted);x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
	hProjectionYHitvsTime_QW_el=new TH2D("ProjectionYHitvsTime_QW_el","TRD Point_Hits vs Drift Time (q-weighted);y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
	hTRDXCorr_el=new TH2D("TRDXCorr_el","X Corr. Between Track Projection and TRD Point_Hits;x(Track Proj) [cm];x(TRD Point_Hit) [cm]",750,-85.,-10.,750,-85.,-10.);
    hTRDYCorr_el=new TH2D("TRDYCorr_el","Y Corr. Between Track Projection and TRD Point_Hits;y(Track Proj) [cm];y(TRD Point_Hit) [cm]",400,-70.,-30.,400,-70.,-30.);
	hnumSeenExtrap_el=new TH1D("numSeenExtrap_el","Num Track Extrapolations Seen in TRD; # Track Extrap.",3,-0.5,2.5);
    hnumPointsSeen_el=new TH1D("numPointsSeen_el","Num Points Counted for a Track Extrapolation Seen in TRD; # Point_Hits",35,-0.5,34.5);
	hnumPointsSeenFCAL_el=new TH1D("numPointsSeenFCAL_el","Num Points Counted for a Track Extrapolation Seen at FCAL; # Point_Hits",35,-0.5,34.5);
    hSeenPointsXY_el=new TH2D("seenPointsXY_el","2D Point Display for Track Extrapolations Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenPointsSingleXY_el=new TH2D("seenPointsSingleXY_el","2D Single Point Display for Track Extrapolations Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
	hSeenPointsSingleX_el=new TH1D("seenPointsSingleX_el","Single Point X Display for Track Extrapolations Seen in TRD; x(Point_Hits) [cm]",75,-85.,-10.);
	hSeenPointsSingleY_el=new TH1D("seenPointsSingleY_el","Single Point Y Display for Track Extrapolations Seen in TRD; y(Point_Hits) [cm]",40,-70.,-30.);
    hnumExtrap_el=new TH1D("numExtrap_el","Num Track Extrapolations that Enter TRD; # Track Extrap.",3,-0.5,2.5);
    hExtrapsXY_el=new TH2D("ExtrapsXY_el","2D Display for Track Extrapolations that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",75,-85.,-10.,40,-70.,-30.);
	hExtrapsX_el=new TH1D("ExtrapsX_el","Track X Extrapolations that Pass Through TRD; x(Track Extrap.) [cm]",75,-85.,-10.);
	hExtrapsY_el=new TH1D("ExtrapsY_el","Track Y Extrapolations that Pass Through TRD; y(Track Extrap.) [cm]",40,-70.,-30.);
    hnumSeenExtrapFCAL_el=new TH1D("numSeenExtrapFCAL_el","Num Track Extrapolations Seen at FCAL; # Track Extrap.",3,-0.5,2.5);
    hSeenPointsFCALXY_el=new TH2D("seenPointsFCALXY_el","2D Point Display for Track Extrapolations Seen at FCAL; x(Point_Hits) [cm]; y(Point_Hits) [cm]",750,-85.,-10.,400,-70.,-30.);
	
	
	//--Pion Subdirectory
	trdDir->cd();
	gDirectory->mkdir("Pion")->cd();
	hnumPiTracks=new TH1D("numPiTracks","Num Pion Hyp. Tracks; # Tracks",12,-0.5,12-0.5);
    hTRDEnergy_pi=new TH1D("TRDEnergy_pi","Pi- Hyp. Track Energy;E [GeV]",240,-0.1,11.9);
    hTRDEnergyDiff_pi=new TH1D("TRDEnergyDiff_pi","fCAL Energy - Pi- Hyp. Track Energy;(fCAL E) - (Pi Hyp. Track E) [GeV]",600,-3.,3.);
    hTRDMomentum_pi=new TH1D("TRDMomentum_pi","Pi- Hyp. Track Mom.;P [GeV/c]",240,-0.1,11.9);
    hTRDTheta_pi=new TH1D("TRDTheta_pi","Pi- Hyp. Track Theta;Theta [Degrees]",350,-0.1,34.9);

    hfCALEP_TRD_pi=new TH1D("fCALEP_TRD_pi","fCAL Track Projection E/P with Extrap in TRD;(fCAL E) / (Measured Track P)",600,-0.1,2.9);
	hfCALEP_cut_pi=new TH1D("fCALEP_cut_pi","fCAL Track Projection E/P with Extrap in TRD and Cut <0.8;(fCAL E) / (Measured Track P)",600,-0.1,2.9);
	hfCALEP_pi=new TH1D("fCALEP_pi","fCAL Track Projection E/P (Pi- Hyp.);(fCAL E) / (Track P)",600,-0.1,2.9);
    hfCALShower_pi=new TH1D("fCALShower_pi","fCAL Shower Energy;E [GeV]",240,-0.1,11.9);
    hfCALMatchXDiff_pi=new TH1D("fCALMatchXDiff_pi","fCAL Track Match X Difference from TRD;x(fCAL) - x(TRD Point_Hits) [cm]",270,-22.,5.);
    hfCALMatchYDiff_pi=new TH1D("fCALMatchYDiff_pi","fCAL Track Match Y Difference from TRD;y(fCAL) - y(TRD Point_Hits) [cm]",270,-22.,5.);
    hfCALXY_pi=new TH2D("fCALXY_pi","2D fCAL Track Match Display with Extrap in TRD;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
    hfCALMatchXYDisplay_pi=new TH2D("fCALMatchXYDisplay_pi","2D fCAL Track Match Display with Extrap in TRD and Cut < 0.8;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
    hfCALMatchXYDiff_pi=new TH2D("fCALMatchXYDiff_pi","2D fCAL Track Match Difference from TRD;x(fCAL) - x(TRD Point_Hits) [cm];y(fCAL) - y(TRD Point_Hits) [cm]",270,-22.,5.,270,-22.,5.);
    
    //hProjectionXDiff_pi=new TH1D("ProjectionXDiff_pi","Track Projection X Difference from TRD Points;x(Proj) - x(TRD Point) [cm]",500,-5.,5.);
    //hProjectionYDiff_pi=new TH1D("ProjectionYDiff_pi","Track Projection Y Difference from TRD Points;y(Proj) - y(TRD Point) [cm]",500,-5.,5.);
    //hProjectionXYDiff_pi=new TH2D("ProjectionXYDiff_pi","Track Projection XY Difference from TRD Points;x(Proj) - x(TRD Point) [cm];y(Proj) - y(TRD Point) [cm]",500,-5.,5.,500,-5.,5.);
	hProjectionXHitDiff_pi=new TH1D("ProjectionXHitDiff_pi","Track Projection X Difference from TRD Point_Hits;x(Proj) - x(TRD Point_Hit) [cm]",500,-5.,5.);
    hProjectionYHitDiff_pi=new TH1D("ProjectionYHitDiff_pi","Track Projection Y Difference from TRD Point_Hits;y(Proj) - y(TRD Point_Hit) [cm]",500,-5.,5.);
    hProjectionXYHitDiff_pi=new TH2D("ProjectionXYHitDiff_pi","Track Projection XY Difference from TRD Point_Hits;x(Proj) - x(TRD Point_Hit) [cm];y(Proj) - y(TRD Point_Hit) [cm]",500,-5.,5.,500,-5.,5.);
    hProjectionXHitvsTime_pi=new TH2D("ProjectionXHitvsTime_pi","TRD Point_Hits vs Drift Time;x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
    hProjectionYHitvsTime_pi=new TH2D("ProjectionYHitvsTime_pi","TRD Point_Hits vs Drift Time;y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
    hProjectionXHitvsTime_QW_pi=new TH2D("ProjectionXHitvsTime_QW_pi","TRD Point_Hits vs Drift Time (q-weighted);x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",750,-85.,-10.,400,0.,200.*8);
    hProjectionYHitvsTime_QW_pi=new TH2D("ProjectionYHitvsTime_QW_pi","TRD Point_Hits vs Drift Time (q-weighted);y(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",400,-70.,-30.,400,0.,200.*8);
    hTRDXCorr_pi=new TH2D("TRDXCorr_pi","X Corr. Between Track Projection and TRD Point_Hits;x(Track Proj) [cm];x(TRD Point_Hit) [cm]",750,-85.,-10.,750,-85.,-10.);
    hTRDYCorr_pi=new TH2D("TRDYCorr_pi","Y Corr. Between Track Projection and TRD Point_Hits;y(Track Proj) [cm];y(TRD Point_Hit) [cm]",400,-70.,-30.,400,-70.,-30.);
	hnumSeenExtrap_pi=new TH1D("numSeenExtrap_pi","Num Track Extrapolations Seen in TRD; # Track Extrap.",3,-0.5,2.5);
    hnumPointsSeen_pi=new TH1D("numPointsSeen_pi","Num Points Counted for a Track Extrapolation Seen in TRD; # Point_Hits",35,-0.5,34.5);
	hnumPointsSeenFCAL_pi=new TH1D("numPointsSeenFCAL_pi","Num Points Counted for a Track Extrapolation Seen at FCAL; # Point_Hits",35,-0.5,34.5);
    hSeenPointsXY_pi=new TH2D("seenPointsXY_pi","2D Point Display for Track Extrapolations Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenPointsSingleXY_pi=new TH2D("seenPointsSingleXY_pi","2D Single Point Display for Track Extrapolations Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenPointsSingleX_pi=new TH1D("seenPointsSingleX_pi","Single Point X Display for Track Extrapolations Seen in TRD; x(Point_Hits) [cm]",75,-85.,-10.);
    hSeenPointsSingleY_pi=new TH1D("seenPointsSingleY_pi","Single Point Y Display for Track Extrapolations Seen in TRD; y(Point_Hits) [cm]",40,-70.,-30.);
    hnumExtrap_pi=new TH1D("numExtrap_pi","Num Track Extrapolations that Enter TRD; # Track Extrap.",3,-0.5,2.5);
    hExtrapsXY_pi=new TH2D("ExtrapsXY_pi","2D Display for Track Extrapolations that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",75,-85.,-10.,40,-70.,-30.);
    hExtrapsX_pi=new TH1D("ExtrapsX_pi","Track X Extrapolations that Pass Through TRD; x(Track Extrap.) [cm]",75,-85.,-10.);
    hExtrapsY_pi=new TH1D("ExtrapsY_pi","Track Y Extrapolations that Pass Through TRD; y(Track Extrap.) [cm]",40,-70.,-30.);
    hnumSeenExtrapFCAL_pi=new TH1D("numSeenExtrapFCAL_pi","Num Track Extrapolations Seen at FCAL; # Track Extrap.",3,-0.5,2.5);
    hSeenPointsFCALXY_pi=new TH2D("seenPointsFCALXY_pi","2D Point Display for Track Extrapolations Seen at FCAL; x(Point_Hits) [cm]; y(Point_Hits) [cm]",750,-85.,-10.,400,-70.,-30.);

	
	mainDir->cd();
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
	//vector<const DTRDHit*>hits; 
    //event->Get(hits);
	//vector<const DTRDStripCluster*>clusters; 
    //event->Get(clusters);
	vector<const DTRDPoint*>pointHits; 
    event->Get(pointHits,"Hit");
	//vector<const DTRDPoint*>points;
	//event->Get(points);
	//vector<const DTRDSegment*>segments; 
    //event->Get(segments);
	vector<const DChargedTrack*>tracks; 
    event->Get(tracks);
	vector<const DFCALShower*> FCALshowers;
    event->Get(FCALshowers);

  	lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
	
	hnumTracks->Fill(tracks.size());
	
  	for (unsigned int j=0; j<tracks.size(); j++) {
    	const DChargedTrackHypothesis *hyp_el=tracks[j]->Get_Hypothesis(Electron);
		bool inTRD = false;
    	if (hyp_el!=nullptr) {
			double p=hyp_el->momentum().Mag();
			hnumElTracks->Fill(tracks.size());
      		//shared_ptr<const DTRDMatchParams>trdparms=hyp_el->Get_TRDMatchParams();
			shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_el->Get_FCALShowerMatchParams();
			
			const DTrackTimeBased *time_track=hyp_el->Get_TrackTimeBased();
			vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
        	vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
			
			if (trd_extrapolations.size()>0 && fcal_extrapolations.size()>0) {
				hnumTracksGoodExtrap->Fill(tracks.size());
				int i = 0;
				int i2 = 0;
            	hTRDExtrapPx->Fill(trd_extrapolations[0].momentum.x());
            	hTRDExtrapPy->Fill(trd_extrapolations[0].momentum.y());
            	hTRDExtrapPz->Fill(trd_extrapolations[0].momentum.z());
            	hTRDExtrapXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
            	if ((trd_extrapolations[0].position.x() > -83.47) && (trd_extrapolations[0].position.x() < -11.47) && (trd_extrapolations[0].position.y() > -68.6) && (trd_extrapolations[0].position.y() < -32.61)) {
					inTRD = true;
					hnumTracksInTRD->Fill(tracks.size());
				}
				
				double extrap_FCALEnergy = 0.;
            	double p_track=fcal_extrapolations[0].momentum.Mag();
                for (const auto& shower : FCALshowers) {
                    hFCALShowerDisplay->Fill(shower->getPosition().X(), shower->getPosition().Y());
                    if (abs(shower->getPosition().X() - fcal_extrapolations[0].position.x()) < 5) {
                        extrap_FCALEnergy = shower->getEnergy();
                        break;
                    }
                }
                if (extrap_FCALEnergy>0. && p_track>1.) {
				
				
				if (inTRD) { 
					hnumExtrap->Fill(1);
					hExtrapsXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
					hExtrapsX->Fill(trd_extrapolations[0].position.x());
					hExtrapsY->Fill(trd_extrapolations[0].position.y());
					hExtrapsFCALXY->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
				}
				
				for (const auto& point : pointHits) {
					if (abs(trd_extrapolations[0].position.x() - point->x < 1.5) && abs(trd_extrapolations[0].position.y() - point->y < 1.5)) { //if within 1.5cm
						if (i==0) {
							hnumSeenExtrap->Fill(1);
							hSeenPointsSingleXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
							hSeenPointsSingleX->Fill(trd_extrapolations[0].position.x());
                            hSeenPointsSingleY->Fill(trd_extrapolations[0].position.y());
						}
						i++;
						hSeenPointsXY->Fill(point->x, point->y);
					}
					if (abs(fcal_extrapolations[0].position.x() - point->x < 1.5) && abs(fcal_extrapolations[0].position.y() - point->y < 1.5)) { //if within 1.5cm
                        if (i2==0) hnumSeenExtrapFCAL->Fill(1);
                        i2++;
                        hSeenPointsFCALXY->Fill(point->x, point->y);
                    }
					
                	hExtrapXHitDiff->Fill(trd_extrapolations[0].position.x() - (point->x));
                	hExtrapYHitDiff->Fill(trd_extrapolations[0].position.y() - (point->y));
					hExtrapXHitDiffvsTime->Fill(trd_extrapolations[0].position.x() - (point->x), point->time);
					hExtrapYHitDiffvsTime->Fill(trd_extrapolations[0].position.y() - (point->y), point->time);
                	hExtrapXYHitDiff->Fill(trd_extrapolations[0].position.x() - (point->x), trd_extrapolations[0].position.y() - (point->y));
                	hTRDXCorr->Fill(trd_extrapolations[0].position.x(), point->x);
                	hTRDYCorr->Fill(trd_extrapolations[0].position.y(), point->y);
            	}
				if (i > 0) hnumPointsSeen->Fill(i);
				if (i2 > 0) hnumPointsSeenFCAL->Fill(i2);
				} //--END  if (extrap_FCALEnergy>0. && p_track>1.)
				/*
            	for (const auto& point : points) {
                	hExtrapXDiff->Fill(trd_extrapolations[0].position.x() - (point->x));
                	hExtrapYDiff->Fill(trd_extrapolations[0].position.y() - (point->y));
	                hExtrapXYDiff->Fill(trd_extrapolations[0].position.x() - (point->x), trd_extrapolations[0].position.y() - (point->y));
    	        }
				
        	    for (const auto& segment : segments) {
            	    hSegmentExtrapXDiff->Fill(trd_extrapolations[0].position.x() - (segment->x));
                	hSegmentExtrapYDiff->Fill(trd_extrapolations[0].position.y() - (segment->y));
	                hSegmentExtrapXYDiff->Fill(trd_extrapolations[0].position.x() - (segment->x), trd_extrapolations[0].position.y() - (segment->y));
    	        }
				*/
            	//hnumTrackMatches->Fill(1);
              	hFCALExtrapPx->Fill(fcal_extrapolations[0].momentum.x());
	            hFCALExtrapPy->Fill(fcal_extrapolations[0].momentum.y());
    	        hFCALExtrapPz->Fill(fcal_extrapolations[0].momentum.z());
                hFCALExtrapXY->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
           	    hExtrapXDiff_FCAL->Fill(fcal_extrapolations[0].position.x() - trd_extrapolations[0].position.x());
              	hExtrapYDiff_FCAL->Fill(fcal_extrapolations[0].position.y() - trd_extrapolations[0].position.y());
               	hExtrapXYDiff_FCAL->Fill(fcal_extrapolations[0].position.x() - trd_extrapolations[0].position.x(), fcal_extrapolations[0].position.y() - trd_extrapolations[0].position.y());
				hExtrapThetavsP->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
				hExtrapTheta->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta());
               	extrap_FCALEnergy = 0.;
	            for (const auto& shower : FCALshowers) {
       	            if (abs(shower->getPosition().X() - fcal_extrapolations[0].position.x()) < 5) {
            	        extrap_FCALEnergy = shower->getEnergy();
                	    break;
                   	}
	            }

				bool ext_electron = false;
  				for (unsigned int i=0; i<tracks.size(); i++) {
    				const DChargedTrackHypothesis *hyp_el=tracks[i]->Get_Hypothesis(Electron);
    				if (hyp_el!=nullptr) {
      					//shared_ptr<const DTRDMatchParams>trdparms=hyp_el->Get_TRDMatchParams();
						shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_el->Get_FCALShowerMatchParams();
						
						const DTrackTimeBased *time_track=hyp_el->Get_TrackTimeBased();
						vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
        				vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
						
						if (trd_extrapolations.size()>0 && fcal_extrapolations.size()>0 && fcalparms!=nullptr) {
							double extrap_FCALEnergy=fcalparms->dFCALShower->getEnergy();	
    	        			if (extrap_FCALEnergy>0.) {
								hFCALExtrapE->Fill(extrap_FCALEnergy);
        	        			hFCALExtrapEP->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag());
								hFCALExtrapEPvsP->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag(), fcal_extrapolations[0].momentum.Mag());
								if (fcal_extrapolations[0].momentum.Mag() >= 0.5 && (extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag()) >= 0.68 && (extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag()) <= 1.2) {
									hExtrapThetavsP_Selected->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
									if(i!=j)ext_electron = true;
								}
							}
						}
					}
				}
				
				
				if (inTRD) {
					hFCALExtrapXY_TRD->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
                    hExtrapXDiff_FCAL_TRD->Fill(fcal_extrapolations[0].position.x() - trd_extrapolations[0].position.x());
                    hExtrapYDiff_FCAL_TRD->Fill(fcal_extrapolations[0].position.y() - trd_extrapolations[0].position.y());
                    hExtrapXYDiff_FCAL_TRD->Fill(fcal_extrapolations[0].position.x() - trd_extrapolations[0].position.x(), fcal_extrapolations[0].position.y() - trd_extrapolations[0].position.y());
                    hExtrapThetavsP_TRD->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
                    hExtrapTheta_TRD->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta());
                    double extrap_FCALEnergy = 0.;
                    for (const auto& shower : FCALshowers) {
                        hFCALShowerDisplay->Fill(shower->getPosition().X(), shower->getPosition().Y());
                        if (abs(shower->getPosition().X() - fcal_extrapolations[0].position.x()) < 5) {
                            extrap_FCALEnergy = shower->getEnergy();
                            break;
                        }
                    }
                    if (extrap_FCALEnergy>0.) {
                        hFCALExtrapE_TRD->Fill(extrap_FCALEnergy);
                        hFCALExtrapEP_TRD->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag());
                        hFCALExtrapEPvsP_TRD->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag(), fcal_extrapolations[0].momentum.Mag());
						if (ext_electron == true) hExtrapThetavsP_Selected_TRD->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
                    }
				} //--END (inTRD) condition
        	} //--END if (trd_extrapolations && fcal_extrapolations)
		
			
			
			//--trdparms NOT YET USEFUL until segment building is fixed..........
			
			if (fcalparms!=nullptr && inTRD) {
				hfCALEP_TRD_el->Fill(fcalparms->dFCALShower->getEnergy() / p);
				if ((fcalparms->dFCALShower->getEnergy() / p) > 1.) {
					hfCALEP_cut_el->Fill(fcalparms->dFCALShower->getEnergy() / p);
					hTRDEnergy_el->Fill(hyp_el->energy());
					hTRDMomentum_el->Fill(hyp_el->momentum().Mag());
					hTRDTheta_el->Fill((180./3.141592)*hyp_el->momentum().Theta());
					hnumExtrap_el->Fill(1);
                    hExtrapsXY_el->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
					hExtrapsX_el->Fill(trd_extrapolations[0].position.x());
					hExtrapsY_el->Fill(trd_extrapolations[0].position.y());
					int i3 = 0;
                	int i4 = 0;
					
					for (const auto& point : pointHits) {
    	            	hProjectionXHitDiff_el->Fill(trd_extrapolations[0].position.x() - point->x);
        	        	hProjectionYHitDiff_el->Fill(trd_extrapolations[0].position.y() - point->y);
            	    	hProjectionXYHitDiff_el->Fill(trd_extrapolations[0].position.x() - point->x, trd_extrapolations[0].position.y() - point->y);
						
						hProjectionXHitvsTime_el->Fill(point->x, point->time);
                    	hProjectionYHitvsTime_el->Fill(point->y, point->time);
						hProjectionXHitvsTime_QW_el->Fill(point->x, point->time, point->dE_x);
                    	hProjectionYHitvsTime_QW_el->Fill(point->y, point->time, point->dE_y);
                    	hTRDXCorr_el->Fill(trd_extrapolations[0].position.x(), point->x);
                    	hTRDYCorr_el->Fill(trd_extrapolations[0].position.y(), point->y);
						hfCALMatchXDiff_el->Fill(fcalparms->dFCALShower->getPosition().X() - point->x);
						hfCALMatchYDiff_el->Fill(fcalparms->dFCALShower->getPosition().Y() - point->y);
						hfCALMatchXYDiff_el->Fill(fcalparms->dFCALShower->getPosition().X() - point->x, fcalparms->dFCALShower->getPosition().Y() - point->y);
						
						if (abs(trd_extrapolations[0].position.x() - point->x < 1.5) && abs(trd_extrapolations[0].position.y() - point->y < 1.5)) { //if within 1.5cm
                        	if (i3==0) {
                            	hnumSeenExtrap_el->Fill(1);
                            	hSeenPointsSingleXY_el->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
								hSeenPointsSingleX_el->Fill(trd_extrapolations[0].position.x());
								hSeenPointsSingleY_el->Fill(trd_extrapolations[0].position.y());
                        	}
                        	i3++;
                        	hSeenPointsXY_el->Fill(point->x, point->y);
                    	}
                    	if (abs(fcalparms->dFCALShower->getPosition().X() - point->x < 1.5) && abs(fcalparms->dFCALShower->getPosition().Y() - point->y < 1.5)) { //if within 1.5cm
                        	if (i4==0) hnumSeenExtrapFCAL_el->Fill(1);
                        	i4++;
                        	hSeenPointsFCALXY_el->Fill(point->x, point->y);
                    	}
            		}
					if (i3 > 0) hnumPointsSeen_el->Fill(i3);
                	if (i4 > 0) hnumPointsSeenFCAL_el->Fill(i4);
					/*
	            	for (const auto& point : points) {
    	            	hProjectionXDiff_el->Fill(trd_extrapolations[0].position.x() - point->x);
        	        	hProjectionYDiff_el->Fill(trd_extrapolations[0].position.y() - point->y);
            	    	hProjectionXYDiff_el->Fill(trd_extrapolations[0].position.x() - point->x, trd_extrapolations[0].position.y() - point->y);
            		}
					*/
					hfCALMatchXYDisplay_el->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
					hTRDEnergyDiff_el->Fill(fcalparms->dFCALShower->getEnergy() - hyp_el->energy());
				} //--END E/P > 0.85 cut
      		}
			if (fcalparms!=nullptr){
				hfCALShower_el->Fill(fcalparms->dFCALShower->getEnergy());
				hfCALXY_el->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
				hfCALEP_el->Fill(fcalparms->dFCALShower->getEnergy() / p);
			}
    	} // End Electron Hypothesis
		
		const DChargedTrackHypothesis *hyp_pi=tracks[j]->Get_Hypothesis(PiMinus);
		if (hyp_pi!=nullptr) {
            hnumPiTracks->Fill(tracks.size());
			double p=hyp_pi->momentum().Mag();
            //shared_ptr<const DTRDMatchParams>trdparms=hyp_pi->Get_TRDMatchParams();
            shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_pi->Get_FCALShowerMatchParams();
			const DTrackTimeBased *time_track=hyp_pi->Get_TrackTimeBased();
            vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
			vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
			
			inTRD = false;
			if (trd_extrapolations.size()>0 && fcal_extrapolations.size()>0) {
				if ((trd_extrapolations[0].position.x() > -83.47) && (trd_extrapolations[0].position.x() < -11.47) && (trd_extrapolations[0].position.y() > -68.6) && (trd_extrapolations[0].position.y() < -32.61)) {
                    inTRD = true;
                }
			}
			
            if (fcalparms!=nullptr && inTRD) {
                hfCALEP_TRD_pi->Fill(fcalparms->dFCALShower->getEnergy() / p);
                if ((fcalparms->dFCALShower->getEnergy() / p) > 0. && (fcalparms->dFCALShower->getEnergy() / p) < 0.5){
					hfCALEP_cut_pi->Fill(fcalparms->dFCALShower->getEnergy() / p);
                    hTRDEnergy_pi->Fill(hyp_pi->energy());
                    hTRDMomentum_pi->Fill(hyp_pi->momentum().Mag());
                    hTRDTheta_pi->Fill((180./3.141592)*hyp_pi->momentum().Theta());
					hnumExtrap_pi->Fill(1);
                    hExtrapsXY_pi->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
					hExtrapsX_pi->Fill(trd_extrapolations[0].position.x());
					hExtrapsY_pi->Fill(trd_extrapolations[0].position.y());
					int i5 = 0;
	                int i6 = 0;

					
                    for (const auto& point : pointHits) {
                        hProjectionXHitDiff_pi->Fill(trd_extrapolations[0].position.x() - point->x);
                        hProjectionYHitDiff_pi->Fill(trd_extrapolations[0].position.y() - point->y);
                        hProjectionXYHitDiff_pi->Fill(trd_extrapolations[0].position.x() - point->x, trd_extrapolations[0].position.y() - point->y);
						hProjectionXHitvsTime_pi->Fill(point->x, point->time);
                        hProjectionYHitvsTime_pi->Fill(point->y, point->time);
						hProjectionXHitvsTime_QW_pi->Fill(point->x, point->time, point->dE_x);
                        hProjectionYHitvsTime_QW_pi->Fill(point->y, point->time, point->dE_y);
                        hTRDXCorr_pi->Fill(trd_extrapolations[0].position.x(), point->x);
                        hTRDYCorr_pi->Fill(trd_extrapolations[0].position.y(), point->y);
						hfCALMatchXDiff_pi->Fill(fcalparms->dFCALShower->getPosition().X() - point->x);
                    	hfCALMatchYDiff_pi->Fill(fcalparms->dFCALShower->getPosition().Y() - point->y);
						hfCALMatchXYDiff_pi->Fill(fcalparms->dFCALShower->getPosition().X() - point->x, fcalparms->dFCALShower->getPosition().Y() - point->y);
						
						if (abs(trd_extrapolations[0].position.x() - point->x < 1.5) && abs(trd_extrapolations[0].position.y() - point->y < 1.5)) { //if within 1.5cm
                            if (i5==0) {
                                hnumSeenExtrap_pi->Fill(1);
                                hSeenPointsSingleXY_pi->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
								hSeenPointsSingleX_pi->Fill(trd_extrapolations[0].position.x());
                                hSeenPointsSingleY_pi->Fill(trd_extrapolations[0].position.y());
                            }
                            i5++;
                            hSeenPointsXY_pi->Fill(point->x, point->y);
                        }
                        if (abs(fcalparms->dFCALShower->getPosition().X() - point->x < 1.5) && abs(fcalparms->dFCALShower->getPosition().Y() - point->y < 1.5)) { //if within 1.5cm
                            if (i6==0) hnumSeenExtrapFCAL_pi->Fill(1);
                            i6++;
                            hSeenPointsFCALXY_pi->Fill(point->x, point->y);
                        }
                    }
					if (i5 > 0) hnumPointsSeen_pi->Fill(i5);
                	if (i6 > 0) hnumPointsSeenFCAL_pi->Fill(i6);
					/*
                    for (const auto& point : points) {
                        hProjectionXDiff_pi->Fill(trd_extrapolations[0].position.x() - point->x);
                        hProjectionYDiff_pi->Fill(trd_extrapolations[0].position.y() - point->y);
                        hProjectionXYDiff_pi->Fill(trd_extrapolations[0].position.x() - point->x, trd_extrapolations[0].position.y() - point->y);
                    }
					*/
                    hfCALMatchXYDisplay_pi->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
                    hTRDEnergyDiff_pi->Fill(fcalparms->dFCALShower->getEnergy() - hyp_pi->energy());
                } //--END E/P < 0.8 cut
            }
            if (fcalparms!=nullptr) {
                hfCALShower_pi->Fill(fcalparms->dFCALShower->getEnergy());
                hfCALXY_pi->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
				hfCALEP_pi->Fill(fcalparms->dFCALShower->getEnergy() / p);
            }
        } // End Pion Hypothesis
		
  	}
	
  	lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_TRDTrack::EndRun()
{
}

//------------------
// Finish
//------------------
void JEventProcessor_TRDTrack::Finish()
{
}

