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
	TDirectory *trdDir = gDirectory->mkdir("TRD");
	trdDir->cd();
	
	//--Book Histograms
	hnumTrackMatches=new TH1D("hnumTrackMatches","Num Tracks Matched with TRD;Num Tracks",3,-0.5,2.5);
	hExtrapXDiff=new TH1D("ExtrapXDiff","Track Extrapolation X Difference from TRD Points;x(Extrap) - x(TRD Point) [cm]",1000,-10.,10.);
    hExtrapYDiff=new TH1D("ExtrapYDiff","Track Extrapolation Y Difference from TRD Points;y(Extrap) - y(TRD Point) [cm]",1000,-10.,10.);
    hExtrapXYDiff=new TH2D("ExtrapXYDiff","Track Extrapolation XY Difference from TRD Points;x(Extrap) - x(TRD Point) [cm];y(Extrap) - y(TRD Point) [cm]",1000,-10.,10.,1000,-10.,10.);
    hExtrapXHitDiff=new TH1D("ExtrapXHitDiff","Track Extrapolation X Difference from TRD Point_Hits;x(Extrap) - x(TRD Point_Hit) [cm]",1000,-10.,10.);
    hExtrapYHitDiff=new TH1D("ExtrapYHitDiff","Track Extrapolation Y Difference from TRD Point_Hits;y(Extrap) - y(TRD Point_Hit) [cm]",1000,-10.,10.);
    hExtrapXYHitDiff=new TH2D("ExtrapXYHitDiff","Track Extrapolation XY Difference from TRD Point_Hits;x(Extrap) - x(TRD Point_Hit) [cm];y(Extrap) - y(TRD Point_Hit) [cm]",1000,-10.,10.,1000,-10.,10.);
	hSegmentExtrapXDiff=new TH1D("SegmentExtrapXDiff","Track Extrapolation X Difference from TRD Segment;x(Extrap) - x(TRD Segment) [cm]",1000,-10.,10.);
    hSegmentExtrapYDiff=new TH1D("SegmentExtrapYDiff","Track Extrapolation Y Difference from TRD Segment;y(Extrap) - y(TRD Segment) [cm]",1000,-10.,10.);
    hSegmentExtrapXYDiff=new TH2D("SegmentExtrapXYDiff","Track Extrapolation XY Difference from TRD Segment;x(Extrap) - x(TRD Segment) [cm];y(Extrap) - y(TRD Segment) [cm]",1000,-10.,10.,1000,-10.,10.);
	hTRDExtrapXY=new TH2D("TRDExtrapXY","Track Extrapolation XY Display at TRD Plane;x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
    hTRDExtrapPx=new TH1D("TRDExtrapPx","TRD Track Extrapolation X Mom.;Px [GeV/c]",200,-5.,5.);
    hTRDExtrapPy=new TH1D("TRDExtrapPy","TRD Track Extrapolation Y Mom.;Py [GeV/c]",200,-5.,5.);
    hTRDExtrapPz=new TH1D("TRDExtrapPz","TRD Track Extrapolation Z Mom.;Pz [GeV/c]",120,-0.1,11.9);
	
	hFCALExtrapXY=new TH2D("FCALExtrapXY","Track Extrapolation XY Display at FCAL Plane;x(Track Extrap) [cm];y(Track Extrap) [cm]",280,-140.,140.,280,-140.,140.);
    hFCALExtrapPx=new TH1D("FCALExtrapPx","FCAL Track Extrapolation X Mom.;Px [GeV/c]",200,-5.,5.);
    hFCALExtrapPy=new TH1D("FCALExtrapPy","FCAL Track Extrapolation Y Mom.;Py [GeV/c]",200,-5.,5.);
    hFCALExtrapPz=new TH1D("FCALExtrapPz","FCAL Track Extrapolation Z Mom.;Pz [GeV/c]",120,-0.1,11.9);
	hExtrapXDiff_FCAL=new TH1D("ExtrapXDiff_FCAL","Track Extrapolation X Difference between TRD and FCAL;x(FCAL) - x(TRD Point_Hit) [cm]",500,-5.,5.);
    hExtrapYDiff_FCAL=new TH1D("ExtrapYDiff_FCAL","Track Extrapolation Y Difference between TRD and FCAL;y(FCAL) - y(TRD Point_Hit) [cm]",500,-5.,5.);
    hExtrapXYDiff_FCAL=new TH2D("ExtrapXYDiff_FCAL","Track Extrapolation XY Difference between TRD and FCAL;x(FCAL) - x(TRD Point_Hit) [cm];y(FCAL) - y(TRD Point_Hit) [cm]",500,-5.,5.,500,-5.,5.);
	hFCALExtrapE=new TH1D("FCALExtrapE","FCAL Track Extrapolation Energy;E [GeV]",120,-0.1,11.9);
	hFCALExtrapEP=new TH1D("FCALExtrapEP","FCAL Track Extrapolation E / P;E / P [GeV/c]",600,-0.1,2.9);
	hFCALExtrapEPvsP=new TH2D("FCALExtrapEPvsP","FCAL Track Extrapolation E / P vs P;E / P [GeV/c]; P [GeV/c]",600,-0.1,2.9,480,-0.1,11.9);
	hFCALShowerDisplay=new TH2D("FCALShowerDisplay","fCAL Shower Extrapolation XY Display;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
	hExtrapTheta=new TH1D("ExtrapTheta","FCAL Track Extrapolation Angle;Theta [Degrees]",350,-0.1,34.9);
	hExtrapThetavsP=new TH2D("ExtrapThetavsP","FCAL Track Extrapolation Angle vs P;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hExtrapThetavsP_Selected=new TH2D("ExtrapThetavsP_Selected","FCAL Track Extrapolation Angle vs P for Selected EP vs P Corr.;Theta [Degrees];P [GeV/c]",350,-0.1,34.9,480,-0.1,11.9);
	hTRDXCorr=new TH2D("TRDXCorr","X Corr. Between Track Extrapolation and TRD Point_Hits;x(Track Extrap.) [cm];x(TRD Point_Hit) [cm]",800,-90.,-10.,800,-90.,-10.);
	hTRDYCorr=new TH2D("TRDYCorr","Y Corr. Between Track Extrapolation and TRD Point_Hits;y(Track Extrap.) [cm];y(TRD Point_Hit) [cm]",400,-70.,-30.,400,-70.,-30.);
	
	//--Electron Subdirectory
	gDirectory->mkdir("Electron")->cd();
	hnumElTracks=new TH1D("numElTracks","Num Electron Tracks; # Tracks",12,-0.5,12-0.5);
	hTRDFlightTime_el=new TH1D("TRDFlightTime_el","TRD Particle Flight Time;Flight Time [?]",400,0.5,20+0.5);
    hTRDEnergy_el=new TH1D("TRDEnergy_el","TRD Track Projection Energy;E [GeV]",120,-0.1,11.9);
    hTRDEnergyDiff_el=new TH1D("TRDEnergyDiff_el","fCAL Energy - TRD Track Projection Energy;(fCAL E) - (Track E) [GeV]",600,-3.,3.);
    hTRDMomentum_el=new TH1D("TRDMomentum_el","TRD Track Projection Mom.;Pz [GeV/c]",120,-0.1,11.9);
    hTRDTheta_el=new TH1D("TRDTheta_el","TRD Track Projection Theta;Theta [Degrees]",350,-0.1,34.9);
	
	hTRDSegmentMatchX_el=new TH1D("TRDSegmentMatchX_el","TRD Track Segment X Difference from Track Proj;x(Track) - x(TRD) [cm]",500,-5.,5.);
	hTRDSegmentMatchY_el=new TH1D("TRDSegmentMatchY_el","TRD Track Segment Y Difference from Track Proj;y(Track) - y(TRD) [cm]",500,-5.,5.);
  	hTRDSegmentMatchXY_el=new TH2D("TRDSegmentMatchXY_el","TRD 2D Track Segment Difference from Track Proj;x(Track) - x(TRD) [cm];y(Track) - y(TRD) [cm]",500,-5,5,500,-5,5);
  	hTRDSegmentMatchTxTy_el=new TH2D("TRDSegmentMatchTxTy_el","TRD 2D Track Segment T Difference from Track Proj;Tx(Track)-Tx(TRD);Ty(Track) - Ty(TRD)",500,-0.5,0.5,500,-0.5,0.5);
  	hTRDSegmentMatchTx_el=new TH1D("TRDSegmentMatchTx_el","TRD Track Segment Tx Difference from Track Proj;Tx(Track) - Tx(TRD)",500,-0.5,0.5);
	hTRDSegmentMatchTy_el=new TH1D("TRDSegmentMatchTy_el","TRD Track Segment Ty Difference from Track Proj;Ty(Track) - Ty(TRD)",500,-0.5,0.5);
	hTRDSegmentXY_el=new TH2D("TRDSegmentXY_el","TRD 2D Track Segment;x(TRD) [cm];y(TRD) [cm]",280,-140.,140.,280,-140.,140.);
	
	hfCALEP_TRD_el=new TH1D("fCALEP_TRD_el","fCAL Track Projection E/P (Electrons);(fCAL E) / (Measured Track P)",600,-0.1,2.9);
	hfCALEP_cut_el=new TH1D("fCALEP_cut_el","fCAL Track Projection E/P (Electrons) With Cut;(fCAL E) / (Measured Track P)",600,-0.1,2.9);
	hfCALEP_el=new TH1D("fCALEP_el","fCAL Track Projection E/P (Electrons);(fCAL E) / (Track P)",600,-0.1,2.9);
	hfCALShower_el=new TH1D("fCALShower_el","fCAL Shower Energy;E [GeV]",120,-0.1,11.9);
	hfCALMatchX_el=new TH1D("fCALMatchX_el","fCAL Track X Difference;x(fCAL) - x(TRD) [cm]",270,-22.,5.);
	hfCALMatchY_el=new TH1D("fCALMatchY_el","fCAL Track Y Difference;y(fCAL) - y(TRD) [cm]",270,-22.,5.);
	hfCALXY_el=new TH2D("fCALXY_el","fCAL Track XY Display_el;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
	hfCALMatchXYDisplay_el=new TH2D("fCALMatchXYDisplay_el","fCAL Track XY Display for TRD Matches;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
	hfCALMatchXY_el=new TH2D("fCALMatchXY_el","TRD 2D Track Segment Difference from fCAL Extrapolation;x(fCAL) - x(TRD) [cm];y(fCAL) - y(TRD) [cm]",270,-22.,5.,270,-22.,5.);
	
	hProjectionXDiff_el=new TH1D("ProjectionXDiff_el","Track Projection X Difference from TRD Points;x(Proj) - x(TRD Point) [cm]",500,-5.,5.);
	hProjectionYDiff_el=new TH1D("ProjectionYDiff_el","Track Projection Y Difference from TRD Points;y(Proj) - y(TRD Point) [cm]",500,-5.,5.);
	hProjectionXYDiff_el=new TH2D("ProjectionXYDiff_el","Track Projection XY Difference from TRD Points;x(Proj) - x(TRD Point) [cm];y(Proj) - y(TRD Point) [cm]",500,-5.,5.,500,-5.,5.);
	hProjectionXHitDiff_el=new TH1D("ProjectionXHitDiff_el","Track Projection X Difference from TRD Point_Hits;x(Proj) - x(TRD Point_Hit) [cm]",500,-5.,5.);
    hProjectionYHitDiff_el=new TH1D("ProjectionYHitDiff_el","Track Projection Y Difference from TRD Point_Hits;y(Proj) - y(TRD Point_Hit) [cm]",500,-5.,5.);
    hProjectionXYHitDiff_el=new TH2D("ProjectionXYHitDiff_el","Track Projection XY Difference from TRD Point_Hits;x(Proj) - x(TRD Point_Hit) [cm];y(Proj) - y(TRD Point_Hit) [cm]",500,-5.,5.,500,-5.,5.);
	
	
	//--Pion Subdirectory
	trdDir->cd();
	gDirectory->mkdir("Pion")->cd();
	hnumPiTracks=new TH1D("numPiTracks","Num Pion Tracks; # Tracks",12,-0.5,12-0.5);
	hTRDFlightTime_pi=new TH1D("TRDFlightTime_pi","TRD Particle Flight Time;Flight Time [?]",400,0.5,20+0.5);
    hTRDEnergy_pi=new TH1D("TRDEnergy_pi","TRD Track Projection Energy;E [GeV]",120,-0.1,11.9);
    hTRDEnergyDiff_pi=new TH1D("TRDEnergyDiff_pi","fCAL Energy - TRD Track Projection Energy;(fCAL E) - (Track E) [GeV]",600,-3.,3.);
    hTRDMomentum_pi=new TH1D("TRDMomentum_pi","TRD Track Projection Mom.;Pz [GeV/c]",120,-0.1,11.9);
    hTRDTheta_pi=new TH1D("TRDTheta_pi","TRD Track Projection Theta;Theta [Degrees]",350,-0.1,34.9);

    hTRDSegmentMatchX_pi=new TH1D("TRDSegmentMatchX_pi","TRD Track Segment X Difference from Track Proj;x(Track) - x(TRD) [cm]",500,-5.,5.);
    hTRDSegmentMatchY_pi=new TH1D("TRDSegmentMatchY_pi","TRD Track Segment Y Difference from Track Proj;y(Track) - y(TRD) [cm]",500,-5.,5.);
    hTRDSegmentMatchXY_pi=new TH2D("TRDSegmentMatchXY_pi","TRD 2D Track Segment Difference from Track Proj;x(Track) - x(TRD) [cm];y(Track) - y(TRD) [cm]",500,-5,5,500,-5,5);
    hTRDSegmentMatchTxTy_pi=new TH2D("TRDSegmentMatchTxTy_pi","TRD 2D Track Segment T from Track Proj;Tx(Track)-Tx(TRD);Ty(Track) - Ty(TRD)",500,-0.5,0.5,500,-0.5,0.5);
    hTRDSegmentMatchTx_pi=new TH1D("TRDSegmentMatchTx_pi","TRD Track Segment Tx Difference from Track Proj;Tx(Track) - Tx(TRD)",500,-0.5,0.5);
    hTRDSegmentMatchTy_pi=new TH1D("TRDSegmentMatchTy_pi","TRD Track Segment Ty Difference from Track Proj;Ty(Track) - Ty(TRD)",500,-0.5,0.5);
    hTRDSegmentXY_pi=new TH2D("TRDSegmentXY_pi","TRD 2D Track Segment;x(TRD) [cm];y(TRD) [cm]",280,-140.,140.,280,-140.,140.);

    hfCALEP_TRD_pi=new TH1D("fCALEP_TRD_pi","fCAL Track Projection E/P (Pions);(fCAL E) / (Measured Track P)",600,-0.1,2.9);
	hfCALEP_cut_pi=new TH1D("fCALEP_cut_pi","fCAL Track Projection E/P (Pions) With Cut;(fCAL E) / (Measured Track P)",600,-0.1,2.9);
	hfCALEP_pi=new TH1D("fCALEP_pi","fCAL Track Projection E/P (Pions);(fCAL E) / (Track P)",600,-0.1,2.9);
    hfCALShower_pi=new TH1D("fCALShower_pi","fCAL Shower Energy;E [GeV]",120,-0.1,11.9);
    hfCALMatchX_pi=new TH1D("fCALMatchX_pi","fCAL Track X Difference;x(fCAL) - x(TRD) [cm]",270,-22.,5.);
    hfCALMatchY_pi=new TH1D("fCALMatchY_pi","fCAL Track Y Difference;y(fCAL) - y(TRD) [cm]",270,-22.,5.);
    hfCALXY_pi=new TH2D("fCALXY_pi","fCAL Track XY Display;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
    hfCALMatchXYDisplay_pi=new TH2D("fCALMatchXYDisplay_pi","fCAL Track XY Display for TRD Matches;x(fCAL) [cm];y(fCAL) [cm]",280,-140.,140.,280,-140,140);
    hfCALMatchXY_pi=new TH2D("fCALMatchXY_pi","TRD 2D Track Segment Difference from fCAL Extrapolation;x(fCAL) - x(TRD) [cm];y(fCAL) - y(TRD) [cm]",270,-22.,5.,270,-22.,5.);
    
    hProjectionXDiff_pi=new TH1D("ProjectionXDiff_pi","Track Projection X Difference from TRD Points;x(Proj) - x(TRD Point) [cm]",500,-5.,5.);
    hProjectionYDiff_pi=new TH1D("ProjectionYDiff_pi","Track Projection Y Difference from TRD Points;y(Proj) - y(TRD Point) [cm]",500,-5.,5.);
    hProjectionXYDiff_pi=new TH2D("ProjectionXYDiff_pi","Track Projection XY Difference from TRD Points;x(Proj) - x(TRD Point) [cm];y(Proj) - y(TRD Point) [cm]",500,-5.,5.,500,-5.,5.);
	hProjectionXHitDiff_pi=new TH1D("ProjectionXHitDiff_pi","Track Projection X Difference from TRD Point_Hits;x(Proj) - x(TRD Point_Hit) [cm]",500,-5.,5.);
    hProjectionYHitDiff_pi=new TH1D("ProjectionYHitDiff_pi","Track Projection Y Difference from TRD Point_Hits;y(Proj) - y(TRD Point_Hit) [cm]",500,-5.,5.);
    hProjectionXYHitDiff_pi=new TH2D("ProjectionXYHitDiff_pi","Track Projection XY Difference from TRD Point_Hits;x(Proj) - x(TRD Point_Hit) [cm];y(Proj) - y(TRD Point_Hit) [cm]",500,-5.,5.,500,-5.,5.);
	
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
	vector<const DTRDHit*>hits; 
    event->Get(hits);
	vector<const DTRDStripCluster*>clusters; 
    event->Get(clusters);
	vector<const DTRDPoint*>pointHits; 
    event->Get(pointHits,"Hit");
	vector<const DTRDPoint*>points;
	event->Get(points);
	vector<const DTRDSegment*>segments; 
    event->Get(segments);
	vector<const DChargedTrack*>tracks; 
    event->Get(tracks);
	vector<const DTrackTimeBased*>trackTBases; 
    event->Get(trackTBases);
	vector<const DFCALShower*> FCALshowers;
    event->Get(FCALshowers);
	
  	lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

  	for (unsigned int j=0;j<tracks.size();j++){
    	const DChargedTrackHypothesis *hyp_el=tracks[j]->Get_Hypothesis(Electron);
    	if (hyp_el!=nullptr){
			double p=hyp_el->momentum().Mag();
			hnumElTracks->Fill(tracks.size());
      		shared_ptr<const DTRDMatchParams>trdparms=hyp_el->Get_TRDMatchParams();
			shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_el->Get_FCALShowerMatchParams();
			
			const DTrackTimeBased *time_track=hyp_el->Get_TrackTimeBased();
			vector<DTrackFitter::Extrapolation_t> trd_extrapolations = time_track->extrapolations.at(SYS_TRD);
        	vector<DTrackFitter::Extrapolation_t> fcal_extrapolations = time_track->extrapolations.at(SYS_FCAL);
			
			if (trd_extrapolations.size()>0) {
            	hTRDExtrapPx->Fill(trd_extrapolations[0].momentum.x());
            	hTRDExtrapPy->Fill(trd_extrapolations[0].momentum.y());
            	hTRDExtrapPz->Fill(trd_extrapolations[0].momentum.z());
            	hTRDExtrapXY->Fill(trd_extrapolations[0].position.x(), trd_extrapolations[0].position.y());
            	for (const auto& point : pointHits) {
                	hExtrapXHitDiff->Fill(trd_extrapolations[0].position.x() - (point->x));
                	hExtrapYHitDiff->Fill(trd_extrapolations[0].position.y() - (point->y));
                	hExtrapXYHitDiff->Fill(trd_extrapolations[0].position.x() - (point->x), trd_extrapolations[0].position.y() - (point->y));
                	hTRDXCorr->Fill(trd_extrapolations[0].position.x(), point->x);
                	hTRDYCorr->Fill(trd_extrapolations[0].position.y(), point->y);
            	}
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
				if (fcal_extrapolations.size()>0){
            	    hnumTrackMatches->Fill(1);
                	hFCALExtrapPx->Fill(fcal_extrapolations[0].momentum.x());
	                hFCALExtrapPy->Fill(fcal_extrapolations[0].momentum.y());
    	            hFCALExtrapPz->Fill(fcal_extrapolations[0].momentum.z());
        	        hFCALExtrapXY->Fill(fcal_extrapolations[0].position.x(), fcal_extrapolations[0].position.y());
            	    hExtrapXDiff_FCAL->Fill(fcal_extrapolations[0].position.x() - trd_extrapolations[0].position.x());
                	hExtrapYDiff_FCAL->Fill(fcal_extrapolations[0].position.y() - trd_extrapolations[0].position.y());
                	hExtrapXYDiff_FCAL->Fill(fcal_extrapolations[0].position.x() - trd_extrapolations[0].position.x(), fcal_extrapolations[0].position.y() - trd_extrapolations[0].position.y());
					hExtrapThetavsP->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
					hExtrapTheta->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta());
                	double extrap_FCALEnergy = 0.;
	                for (const auto& shower : FCALshowers) {
    	                hFCALShowerDisplay->Fill(shower->getPosition().X(), shower->getPosition().Y());
        	            if (abs(shower->getPosition().X() - fcal_extrapolations[0].position.x()) < 5) {
            	            extrap_FCALEnergy = shower->getEnergy();
                	        break;
                    	}
	                }
    	            if (extrap_FCALEnergy>0.) {
						hFCALExtrapE->Fill(extrap_FCALEnergy);
        	        	hFCALExtrapEP->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag());
						hFCALExtrapEPvsP->Fill(extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag(), fcal_extrapolations[0].momentum.Mag());
						if (fcal_extrapolations[0].momentum.Mag() > 0.5 && fcal_extrapolations[0].momentum.Mag() < 1.5 && (extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag()) > 0.7 && (extrap_FCALEnergy / fcal_extrapolations[0].momentum.Mag()) < 1.2) hExtrapThetavsP_Selected->Fill((180./3.141592)*fcal_extrapolations[0].momentum.Theta(), fcal_extrapolations[0].momentum.Mag());
					}
            	}
        	}
			
			
			if (trdparms!=nullptr && fcalparms!=nullptr){
				hfCALEP_TRD_el->Fill(fcalparms->dFCALShower->getEnergy() / p);
				if ((fcalparms->dFCALShower->getEnergy() / p) > 0.85){
					hfCALEP_cut_el->Fill(fcalparms->dFCALShower->getEnergy() / p);
					hTRDSegmentMatchX_el->Fill(-1.*(trdparms->dDeltaXToSegment));
					hTRDSegmentMatchY_el->Fill(-1.*(trdparms->dDeltaYToSegment));
					hTRDSegmentMatchXY_el->Fill(-1.*(trdparms->dDeltaXToSegment), -1.*(trdparms->dDeltaYToSegment));
					hTRDSegmentMatchTx_el->Fill(-1.*(trdparms->dDeltaTxToSegment));
					hTRDSegmentMatchTy_el->Fill(-1.*(trdparms->dDeltaTyToSegment));
					hTRDSegmentMatchTxTy_el->Fill(-1.*(trdparms->dDeltaTxToSegment), -1.*(trdparms->dDeltaTyToSegment));
					hTRDSegmentXY_el->Fill(trdparms->dTRDSegment->x, trdparms->dTRDSegment->y);
					hTRDFlightTime_el->Fill(trdparms->dFlightTime);
					hTRDEnergy_el->Fill(hyp_el->energy());
					hTRDMomentum_el->Fill(hyp_el->momentum().Pz());
					hTRDTheta_el->Fill((180./3.141592)*hyp_el->momentum().Theta());
					
					for (const auto& point : pointHits) {
    	            	hProjectionXHitDiff_el->Fill(trdparms->dTRDSegment->x - point->x);
        	        	hProjectionYHitDiff_el->Fill(trdparms->dTRDSegment->y - point->y);
            	    	hProjectionXYHitDiff_el->Fill(trdparms->dTRDSegment->x - point->x, trdparms->dTRDSegment->y - point->y);
            		}
	            	for (const auto& point : points) {
    	            	hProjectionXDiff_el->Fill(trdparms->dTRDSegment->x - point->x);
        	        	hProjectionYDiff_el->Fill(trdparms->dTRDSegment->y - point->y);
            	    	hProjectionXYDiff_el->Fill(trdparms->dTRDSegment->x - point->x, trdparms->dTRDSegment->y - point->y);
            		}
					
					hfCALMatchX_el->Fill(fcalparms->dFCALShower->getPosition().X() - trdparms->dTRDSegment->x);
					hfCALMatchY_el->Fill(fcalparms->dFCALShower->getPosition().Y() - trdparms->dTRDSegment->y);
					hfCALMatchXYDisplay_el->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
					hfCALMatchXY_el->Fill(fcalparms->dFCALShower->getPosition().X() - trdparms->dTRDSegment->x, fcalparms->dFCALShower->getPosition().Y() - trdparms->dTRDSegment->y);
					hTRDEnergyDiff_el->Fill(fcalparms->dFCALShower->getEnergy() - hyp_el->energy());
				}
      		}
			if (fcalparms!=nullptr){
				hfCALShower_el->Fill(fcalparms->dFCALShower->getEnergy());
				hfCALXY_el->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
				hfCALEP_el->Fill(fcalparms->dFCALShower->getEnergy() / p);
			}
    	} // End Electron Hypothesis
		
		const DChargedTrackHypothesis *hyp_pi=tracks[j]->Get_Hypothesis(PiMinus);
		if (hyp_pi!=nullptr){
            hnumPiTracks->Fill(tracks.size());
			double p=hyp_pi->momentum().Mag();
            shared_ptr<const DTRDMatchParams>trdparms=hyp_pi->Get_TRDMatchParams();
            shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp_pi->Get_FCALShowerMatchParams();
            if (trdparms!=nullptr && fcalparms!=nullptr){
                hfCALEP_TRD_pi->Fill(fcalparms->dFCALShower->getEnergy() / p);
                if ((fcalparms->dFCALShower->getEnergy() / p) > 0. && (fcalparms->dFCALShower->getEnergy() / p) < 0.8){
					hfCALEP_cut_pi->Fill(fcalparms->dFCALShower->getEnergy() / p);
                    hTRDSegmentMatchX_pi->Fill(-1.*(trdparms->dDeltaXToSegment));
                    hTRDSegmentMatchY_pi->Fill(-1.*(trdparms->dDeltaYToSegment));
                    hTRDSegmentMatchXY_pi->Fill(-1.*(trdparms->dDeltaXToSegment), -1.*(trdparms->dDeltaYToSegment));
                    hTRDSegmentMatchTx_pi->Fill(-1.*(trdparms->dDeltaTxToSegment));
                    hTRDSegmentMatchTy_pi->Fill(-1.*(trdparms->dDeltaTyToSegment));
                    hTRDSegmentMatchTxTy_pi->Fill(-1.*(trdparms->dDeltaTxToSegment), -1.*(trdparms->dDeltaTyToSegment));
                    hTRDSegmentXY_pi->Fill(trdparms->dTRDSegment->x, trdparms->dTRDSegment->y);
                    hTRDFlightTime_pi->Fill(trdparms->dFlightTime);
                    hTRDEnergy_pi->Fill(hyp_pi->energy());
                    hTRDMomentum_pi->Fill(hyp_pi->momentum().Pz());
                    hTRDTheta_pi->Fill((180./3.141592)*hyp_pi->momentum().Theta());

                    for (const auto& point : pointHits) {
                        hProjectionXHitDiff_pi->Fill(trdparms->dTRDSegment->x - point->x);
                        hProjectionYHitDiff_pi->Fill(trdparms->dTRDSegment->y - point->y);
                        hProjectionXYHitDiff_pi->Fill(trdparms->dTRDSegment->x - point->x, trdparms->dTRDSegment->y - point->y);
                    }
                    for (const auto& point : points) {
                        hProjectionXDiff_pi->Fill(trdparms->dTRDSegment->x - point->x);
                        hProjectionYDiff_pi->Fill(trdparms->dTRDSegment->y - point->y);
                        hProjectionXYDiff_pi->Fill(trdparms->dTRDSegment->x - point->x, trdparms->dTRDSegment->y - point->y);
                    }
					hfCALMatchX_pi->Fill(fcalparms->dFCALShower->getPosition().X() - trdparms->dTRDSegment->x);
                    hfCALMatchY_pi->Fill(fcalparms->dFCALShower->getPosition().Y() - trdparms->dTRDSegment->y);
                    hfCALMatchXYDisplay_pi->Fill(fcalparms->dFCALShower->getPosition().X(), fcalparms->dFCALShower->getPosition().Y());
                    hfCALMatchXY_pi->Fill(fcalparms->dFCALShower->getPosition().X() - trdparms->dTRDSegment->x, fcalparms->dFCALShower->getPosition().Y() - trdparms->dTRDSegment->y);
                    hTRDEnergyDiff_pi->Fill(fcalparms->dFCALShower->getEnergy() - hyp_pi->energy());
                }
            }
            if (fcalparms!=nullptr){
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

