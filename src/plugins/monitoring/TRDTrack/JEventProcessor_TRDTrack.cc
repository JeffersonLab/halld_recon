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
	hTrackMult=new TH1D("hTrackMult","Charged Track Multiplicity;Num Tracks",12,0.5,12.5);
	hHypMomentumDiff=new TH1D("HypMomentumDiff","FCAL Extrap vs Electron Hyp. Momentum Difference;(El Hyp. Track P) - (FCAL Track Extrap P) [GeV/c]",200,-5.,5.);
    hExtrapXHitDiff=new TH1D("ExtrapXHitDiff","Track Extrap X Difference from TRD Point_Hits;x(Extrap) - x(TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapYHitDiff=new TH1D("ExtrapYHitDiff","Track Extrap Y Difference from TRD Point_Hits;y(Extrap) - y(TRD Point_Hit) [cm]",250,-5.5,5.5);
    hExtrapXYHitDiff=new TH2D("ExtrapXYHitDiff","Track Extrap XY Difference from TRD Point_Hits;x(Extrap) - x(TRD Point_Hit) [cm];y(Extrap) - y(TRD Point_Hit) [cm]",250,-5.5,5.5,250,-5.5,5.5);
	hExtrapXHitDiffvsTime=new TH2D("ExtrapXHitDiffvsTime","Track Extrap X Difference from TRD Point_Hits vs Drift Time;x(Extrap) - x(TRD Point_Hit) [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,400,0.,200.*8);
	hExtrapYHitDiffvsTime=new TH2D("ExtrapYHitDiffvsTime","Track Extrap Y Difference from TRD Point_Hits vs Drift Time;y(Extrap) - y(TRD Point_Hit)     [cm]; 8*(Peak Time) [ns]",250,-5.5,5.5,400,0.,200.*8);
	hTRDExtrapXY=new TH2D("TRDExtrapXY","2D Display for Track Extrap at TRD Plane;x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
    hTRDExtrapPx=new TH1D("TRDExtrapPx","TRD Track Extrap X Mom.;Px [GeV/c]",200,-3.,3.);
    hTRDExtrapPy=new TH1D("TRDExtrapPy","TRD Track Extrap Y Mom.;Py [GeV/c]",200,-3.,3.);
    hTRDExtrapPz=new TH1D("TRDExtrapPz","TRD Track Extrap Z Mom.;Pz [GeV/c]",240,-0.1,11.9);
	hTRDXCorr=new TH2D("TRDXCorr","X Corr. Between Track Extrap and TRD Point_Hits;x(Track Extrap.) [cm];x(TRD Point_Hit) [cm]",750,-85.,-10.,750,-85.,-10.);
    hTRDYCorr=new TH2D("TRDYCorr","Y Corr. Between Track Extrap and TRD Point_Hits;y(Track Extrap.) [cm];y(TRD Point_Hit) [cm]",400,-70.,-30.,400,-70.,-30.);
	
    hFCALExtrapPx=new TH1D("FCALExtrapPx","FCAL Track Extrap X Mom.;Px [GeV/c]",200,-3.,3.);
    hFCALExtrapPy=new TH1D("FCALExtrapPy","FCAL Track Extrap Y Mom.;Py [GeV/c]",200,-5.,5.);
    hFCALExtrapPz=new TH1D("FCALExtrapPz","FCAL Track Extrap Z Mom.;Pz [GeV/c]",240,-0.1,11.9);
	
	hFCALExtrapXY=new TH2D("FCALExtrapXY","2D Display for FCAL Shower (all p);x(FCAL Shower) [cm];y(FCAL Shower) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALShowerXY=new TH2D("FCALShowerXY","2D Display for FCAL Track Extrap (all p);x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALExtrapE=new TH1D("FCALExtrapE","FCAL Track Extrap Energy;E [GeV]",240,-0.1,11.9);
	hFCALExtrapEP=new TH1D("FCALExtrapEP","FCAL Track Extrap E / P;E / P [GeV/c]",600,-0.1,2.9);
	hFCALExtrapEPvsP=new TH2D("FCALExtrapEPvsP","FCAL Track Extrap E / P vs P;E / P [GeV/c]; P [GeV/c]",600,-0.1,2.9,480,-0.1,11.9);
	hFCALExtrapTheta=new TH1D("FCALExtrapTheta","FCAL Track Extrap Angle at FCAL Z Plane;Theta [Degrees]",350,-0.1,34.9);
	hFCALExtrapThetavsP=new TH2D("FCALExtrapThetavsP","FCAL Track Extrap Angle (at FCAL Z Plane) vs P;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hFCALExtrapThetavsP_Selected=new TH2D("FCALExtrapThetavsP_Selected","FCAL Track Extrap Angle (at FCAL Z Plane) vs P for Selected E/P vs P;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hFCALExtrapXY_TRD=new TH2D("FCALExtrapXY_TRD","2D Display for FCAL Track Extrap with Track Extrap. in TRD (all p);x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALShowerXY_TRD=new TH2D("FCALShowerXY_TRD","2D Display for FCAL Shower with Track Extrap. in TRD (all p);x(FCAL Shower) [cm];y(FCAL Shower) [cm]",280,-140.,140.,280,    -140.,140.);
	hFCALExtrapXY_p1_TRD=new TH2D("FCALExtrapXY_p1_TRD","2D Display for FCAL Shower with Track Extrap. in TRD (p>1); x(FCAL Shower) [cm]; y(FCAL Shower) [cm]",280,-140.,140.,280,-140.,140.);
	hFCALExtrapE_TRD=new TH1D("FCALExtrapE_TRD","FCAL Shower Energy w Extrap in TRD;E [GeV]",240,-0.1,11.9);
    hFCALExtrapEP_TRD=new TH1D("FCALExtrapEP_TRD","FCAL Track Extrap E/P with Extrap in TRD;(FCAL Shower E) / (Track Hyp. P)",600,-0.1,2.9);
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
	
	
	//--Electron Subdirectory
	gDirectory->mkdir("Electron")->cd();
    hHypEnergy_el=new TH1D("HypEnergy_el","Electron Hyp. Track Energy;E [GeV]",240,-0.1,11.9);
    hHypEnergyDiff_el=new TH1D("HypEnergyDiff_el","FCAL Shower Energy - Electron Hyp. Track Energy;(El Hyp. Track E) - (FCAL E) [GeV]",200,-5.,5.);
    hHypMomentum_el=new TH1D("HypMomentum_el","Electron Hyp. Track Mom.;P [GeV/c]",240,-0.1,11.9);
    hHypTheta_el=new TH1D("HypTheta_el","Electron Hyp. Track Theta (at origin);Theta [Degrees]",350,-0.1,34.9);
	
	hFCALEP_TRD_el=new TH1D("FCALEP_TRD_el","FCAL Track Extrap E/P with Extrap in TRD;(FCAL E) / (Track Extrap P)",600,-0.1,2.9);
	hFCALEP_cut_el=new TH1D("FCALEP_cut_el","FCAL Track Extrap E/P with Extrap in TRD and p>0.85;(FCAL E) / (Track Extrap P)",600,-0.1,2.9);
	hFCALMatchXYDisplay_el=new TH2D("FCALMatchXYDisplay_el","2D FCAL Track Match Display with Extrap in TRD and p>0.85;x(FCAL) [cm];y(FCAL) [cm]",280,-140.,140.,280,-140,140);
	
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
    hnumPointsSeen_el=new TH1D("numPointsSeen_el","Num Points Counted for a Track Extrap Seen in TRD; # Point_Hits",35,-0.5,34.5);
    hSeenPointsXY_el=new TH2D("seenPointsXY_el","2D Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenPointsSingleXY_el=new TH2D("seenPointsSingleXY_el","2D Single Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
	hSeenPointsSingleX_el=new TH1D("seenPointsSingleX_el","Single Point X Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]",75,-85.,-10.);
	hSeenPointsSingleY_el=new TH1D("seenPointsSingleY_el","Single Point Y Display for Track Extraps Seen in TRD; y(Point_Hits) [cm]",40,-70.,-30.);
    hExtrapsXY_el=new TH2D("ExtrapsXY_el","2D Display for Track Extraps that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",75,-85.,-10.,40,-70.,-30.);
	hExtrapsX_el=new TH1D("ExtrapsX_el","Track X Extraps that Pass Through TRD; x(Track Extrap.) [cm]",75,-85.,-10.);
	hExtrapsY_el=new TH1D("ExtrapsY_el","Track Y Extraps that Pass Through TRD; y(Track Extrap.) [cm]",40,-70.,-30.);
	
	
	//--Pion Subdirectory
	trdDir->cd();
	gDirectory->mkdir("Pion")->cd();
    hHypEnergy_pi=new TH1D("HypEnergy_pi","Pi- Hyp. Track Energy;E [GeV]",240,-0.1,11.9);
    hHypEnergyDiff_pi=new TH1D("HypEnergyDiff_pi","FCAL Shower Energy - Pi- Hyp. Track Energy;(Pi Hyp. Track E) - (FCAL E) [GeV]",200,-5.,5.);
    hHypMomentum_pi=new TH1D("HypMomentum_pi","Pi- Hyp. Track Mom.;P [GeV/c]",240,-0.1,11.9);
    hHypTheta_pi=new TH1D("HypTheta_pi","Pi- Hyp. Track Theta (at origin);Theta [Degrees]",350,-0.1,34.9);

    hFCALEP_TRD_pi=new TH1D("FCALEP_TRD_pi","FCAL Track Extrap E/P with Extrap in TRD;(FCAL E) / (Track Extrap P)",600,-0.1,2.9);
	hFCALEP_cut_pi=new TH1D("FCALEP_cut_pi","FCAL Track Extrap E/P with Extrap in TRD and p<0.5;(FCAL E) / (Track Extrap P)",600,-0.1,2.9);
    hFCALMatchXYDisplay_pi=new TH2D("FCALMatchXYDisplay_pi","2D FCAL Track Match Display with Extrap in TRD and p<0.5;x(FCAL) [cm];y(FCAL) [cm]",280,-140.,140.,280,-140,140);
    
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
    hnumPointsSeen_pi=new TH1D("numPointsSeen_pi","Num Points Counted for a Track Extrap Seen in TRD; # Point_Hits",35,-0.5,34.5);
    hSeenPointsXY_pi=new TH2D("seenPointsXY_pi","2D Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenPointsSingleXY_pi=new TH2D("seenPointsSingleXY_pi","2D Single Point Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]; y(Point_Hits) [cm]",75,-85.,-10.,40,-70.,-30.);
    hSeenPointsSingleX_pi=new TH1D("seenPointsSingleX_pi","Single Point X Display for Track Extraps Seen in TRD; x(Point_Hits) [cm]",75,-85.,-10.);
    hSeenPointsSingleY_pi=new TH1D("seenPointsSingleY_pi","Single Point Y Display for Track Extraps Seen in TRD; y(Point_Hits) [cm]",40,-70.,-30.);
    hExtrapsXY_pi=new TH2D("ExtrapsXY_pi","2D Display for Track Extraps that Pass Through TRD; x(Track Extrap.) [cm]; y(Track Extrap.) [cm]",75,-85.,-10.,40,-70.,-30.);
    hExtrapsX_pi=new TH1D("ExtrapsX_pi","Track X Extrapolations that Pass Through TRD; x(Track Extrap.) [cm]",75,-85.,-10.);
    hExtrapsY_pi=new TH1D("ExtrapsY_pi","Track Y Extrapolations that Pass Through TRD; y(Track Extrap.) [cm]",40,-70.,-30.);

	
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
	vector<const DTRDPoint*>pointHits; 
    event->Get(pointHits,"Hit");
	vector<const DChargedTrack*>tracks; 
    event->Get(tracks);
	vector<const DFCALShower*> FCALshowers;
    event->Get(FCALshowers);

  	lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
	
	int numTracks=0, goodTrack=0, extrapInTRD=0, extrapInTRD_p1=0, singleSeenExtrap=0, extElectron=0, singleSeenExtrap_el=0, singleExtrap_el=0, singleSeenExtrap_pi=0, singleExtrap_pi=0;
	hTrackMult->Fill(tracks.size());
	
  	for (unsigned int j=0; j<tracks.size(); j++) {
    	const DChargedTrackHypothesis *hyp_el=tracks[j]->Get_Hypothesis(Electron);
		bool inTRD = false;
		bool TrackMatch = false;
		double extrap_FCALEnergy = 0.;
		
    	if (hyp_el!=nullptr) {
			double p_hyp=hyp_el->momentum().Mag();
			shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_el->Get_FCALShowerMatchParams();
			const DTrackTimeBased *time_track=hyp_el->Get_TrackTimeBased();
			vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
        	vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
			
			if (trd_extrapolations.size()>0 && fcal_extrapolations.size()>0 && fcalparms!=nullptr) {
				goodTrack++;
				int i1 = 0;
            	hTRDExtrapPx->Fill(trd_extrapolations[0].momentum.x());
            	hTRDExtrapPy->Fill(trd_extrapolations[0].momentum.y());
            	hTRDExtrapPz->Fill(trd_extrapolations[0].momentum.z());
            	hTRDExtrapXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
            	if ((trd_extrapolations[0].position.x() > -83.47) && (trd_extrapolations[0].position.x() < -11.47) && (trd_extrapolations[0].position.y() > -68.6) && (trd_extrapolations[0].position.y() < -32.61)) {
					inTRD = true;
					extrapInTRD++;
				}
				
				extrap_FCALEnergy = fcalparms->dFCALShower->getEnergy();
            	double p_track = fcal_extrapolations[0].momentum.Mag();
				hHypMomentumDiff->Fill(p_hyp - p_track);
                if (extrap_FCALEnergy>0. && p_hyp>=1.) {
				
				if (inTRD) {
					extrapInTRD_p1++;
					hExtrapsXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
					hExtrapsX->Fill(trd_extrapolations[0].position.x());
					hExtrapsY->Fill(trd_extrapolations[0].position.y());
					hFCALExtrapXY_p1_TRD->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
				}
				
				for (const auto& point : pointHits) {
					if (abs(trd_extrapolations[0].position.x() - point->x) < 1.5 && abs(trd_extrapolations[0].position.y() - point->y) < 1.5) { //if within 1.5cm
						if (i1==0) {
							singleSeenExtrap++;
							hSeenPointsSingleXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
							hSeenPointsSingleX->Fill(trd_extrapolations[0].position.x());
                            hSeenPointsSingleY->Fill(trd_extrapolations[0].position.y());
						}
						i1++;
						TrackMatch = true;
						hSeenPointsXY->Fill(point->x, point->y);
					}
					
                	hExtrapXHitDiff->Fill(trd_extrapolations[0].position.x() - point->x);
                	hExtrapYHitDiff->Fill(trd_extrapolations[0].position.y() - point->y);
					hExtrapXHitDiffvsTime->Fill(trd_extrapolations[0].position.x() - point->x, point->time);
					hExtrapYHitDiffvsTime->Fill(trd_extrapolations[0].position.y() - point->y, point->time);
                	hExtrapXYHitDiff->Fill(trd_extrapolations[0].position.x() - point->x, trd_extrapolations[0].position.y() - point->y);
                	hTRDXCorr->Fill(trd_extrapolations[0].position.x(), point->x);
                	hTRDYCorr->Fill(trd_extrapolations[0].position.y(), point->y);
            	} //--END Point_Hits loop
				if (i1 > 0) hnumPointsSeen->Fill(i1);
				} //--END  if (extrap_FCALEnergy>0. && p>=1.)

              	hFCALExtrapPx->Fill(fcal_extrapolations[0].momentum.x());
	            hFCALExtrapPy->Fill(fcal_extrapolations[0].momentum.y());
    	        hFCALExtrapPz->Fill(fcal_extrapolations[0].momentum.z());
                hFCALExtrapXY->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
				hFCALShowerXY->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
				hFCALExtrapThetavsP->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
				hFCALExtrapTheta->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta());

				//--Loop to find additional external electron
				bool ext_electron = false;
  				for (unsigned int i=0; i<tracks.size(); i++) {
    				const DChargedTrackHypothesis *hyp_el=tracks[i]->Get_Hypothesis(Electron);
    				if (hyp_el!=nullptr) {
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
								
								if (fcal_extrapolations[0].momentum.Mag() >= 0.5 && (extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag()) >= 0.75 && (extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag()) <= 1.3) {
									hFCALExtrapThetavsP_Selected->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
									if(i!=j){ ext_electron = true; extElectron++; }
								}
							}
						}
					}
				}
				
				
				if (inTRD) {
					extrap_FCALEnergy=fcalparms->dFCALShower->getEnergy();
                    if (extrap_FCALEnergy>0. && ext_electron) {
                        hFCALExtrapE_ext->Fill(extrap_FCALEnergy);
                        hFCALExtrapEP_ext->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag());
                        hFCALExtrapEPvsP_ext->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag(), fcal_extrapolations[0].momentum.Mag());
						hFCALExtrapThetavsP_Selected_ext->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
                    }
	                hFCALExtrapE_TRD->Fill(extrap_FCALEnergy);
        	        hFCALExtrapEP_TRD->Fill(extrap_FCALEnergy / p_hyp);
					hFCALShowerXY_TRD->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
					hFCALExtrapXY_TRD->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
                    hFCALExtrapThetavsP_TRD->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
                    hFCALExtrapTheta_TRD->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta());

				} //--END (inTRD) condition
        	} //--END if (trd_extrapolations && fcal_extrapolations && fcalparms)
			
//--START ELECTRON FOLDER ---------------			
			
			if (fcalparms!=nullptr && inTRD) {
				double extrap_FCAL_EP = fcalparms->dFCALShower->getEnergy() / p_hyp;
				if (TrackMatch) hFCALEP_TRD_el->Fill(extrap_FCAL_EP);
				if (extrap_FCAL_EP>0.85 && extrap_FCAL_EP<1.3) {
					if (TrackMatch) {
						hFCALEP_cut_el->Fill(extrap_FCAL_EP);
						hHypEnergy_el->Fill(hyp_el->energy());
						hHypMomentum_el->Fill(p_hyp);
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
            		} //END Point_Hits loop
					
					if (TrackMatch) {	
				        hXHitvsTime_Qmax_el->Fill(XPos_max, TX_max);
                    	hYHitvsTime_Qmax_el->Fill(YPos_max, TY_max);
						hXHitvsTime_Qmax_QW_el->Fill(XPos_max, TX_max, QX_max);
                    	hYHitvsTime_Qmax_QW_el->Fill(YPos_max, TY_max, QY_max);
			        }
					if (i3 > 0) hnumPointsSeen_el->Fill(i3);
					hFCALMatchXYDisplay_el->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
					hHypEnergyDiff_el->Fill(hyp_el->energy() - fcalparms->dFCALShower->getEnergy());
				} //--END E/P > 0.85 && <1.3 cut
      		} //--END if (fcalparms && inTRD)
    	} // End Electron Hypothesis

//--START PION FOLDER ------------------------------------------------------
		
		const DChargedTrackHypothesis *hyp_pi=tracks[j]->Get_Hypothesis(PiMinus);
		if (hyp_pi!=nullptr) {
			double p_hyp=hyp_pi->momentum().Mag();
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
				double extrap_FCAL_EP = fcalparms->dFCALShower->getEnergy() / p_hyp;
                hFCALEP_TRD_pi->Fill(extrap_FCAL_EP);
                if (extrap_FCAL_EP>0. && extrap_FCAL_EP<0.5) {
					if (TrackMatch) {
						hFCALEP_cut_pi->Fill(extrap_FCAL_EP);
    	                hHypEnergy_pi->Fill(hyp_pi->energy());
        	            hHypMomentum_pi->Fill(p_hyp);
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
                    }
					if (TrackMatch) {
				        hXHitvsTime_Qmax_pi->Fill(XPos_max, TX_max);
                    	hYHitvsTime_Qmax_pi->Fill(YPos_max, TY_max);
						hXHitvsTime_Qmax_QW_pi->Fill(XPos_max, TX_max, QX_max);
                    	hYHitvsTime_Qmax_QW_pi->Fill(YPos_max, TY_max, QY_max);
			        }
					if (i5 > 0) hnumPointsSeen_pi->Fill(i5);
                    hFCALMatchXYDisplay_pi->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
                    hHypEnergyDiff_pi->Fill(hyp_pi->energy() - fcalparms->dFCALShower->getEnergy());
                } //--END E/P < 0.5 cut
            }
        } //--END Pion Hypothesis
		
  	} //--END Track Loop
	if (numTracks>0) Count("nTrack");
	if (goodTrack>0) Count("nGoodTrack");
	if (extrapInTRD>0) Count("ExtrapInTRD");
	if (extrapInTRD_p1>0) Count("ExtrapInTRD_p1");
	if (singleSeenExtrap>0) Count("ExtrapSeen_p1");
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
}

//------------------
// Finish
//------------------
void JEventProcessor_TRDTrack::Finish()
{
}

