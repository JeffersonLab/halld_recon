// $Id$
//
//    File: JEventProcessor_TRDTrack.h
// Created: Thu Mar 27 02:20:19 PM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_TRDTrack_
#define _JEventProcessor_TRDTrack_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services
#include <TH1D.h>
#include <TH2D.h>

class JEventProcessor_TRDTrack:public JEventProcessor{
public:
  JEventProcessor_TRDTrack();
  ~JEventProcessor_TRDTrack();
  const char* className(void){return "JEventProcessor_TRDTrack";}
  
private:
  void Init() override;                       ///< Called once at program start.
  void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
  void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
  void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
  void Finish() override;                     ///< Called after last event of last event source has been processed.
  
  //-Sim
  TH2D *hTRDExtrapXY, *hExtrapXYHitDiff, *hTRDXCorr, *hTRDYCorr, *hExtrapThetavsP, *hExtrapThetavsP_Selected, *hExtrapXHitDiffvsTime, *hExtrapYHitDiffvsTime;
  TH1D *hTRDExtrapPx, *hTRDExtrapPy, *hTRDExtrapPz, *hExtrapXHitDiff, *hExtrapYHitDiff, *hExtrapTheta;
  //-Projection
  TH2D *hProjectionXYHitDiff_el, *hProjectionXYHitDiff_pi;
  TH1D *hProjectionXHitDiff_el, *hProjectionYHitDiff_el, *hProjectionXHitDiff_pi, *hProjectionYHitDiff_pi;
  //-Cal
  TH2D *hfCALXY_el, *hfCALMatchXYDiff_el, *hfCALMatchXYDisplay_el, *hfCALXY_pi, *hfCALMatchXYDiff_pi, *hfCALMatchXYDisplay_pi;
  TH1D *hfCALShower_el, *hfCALEP_TRD_el, *hfCALMatchXDiff_el, *hfCALMatchYDiff_el, *hfCALShower_pi, *hfCALEP_TRD_pi, *hfCALMatchXDiff_pi, *hfCALMatchYDiff_pi, *hfCALEP_cut_el, *hfCALEP_cut_pi, *hfCALEP_el, *hfCALEP_pi;
 
  TH2D *hFCALExtrapXY, *hExtrapXYDiff_FCAL, *hFCALShowerDisplay, *hFCALExtrapEPvsP;
  TH1D *hFCALExtrapPx, *hFCALExtrapPy, *hFCALExtrapPz, *hExtrapXDiff_FCAL, *hExtrapYDiff_FCAL, *hFCALExtrapE, *hFCALExtrapEP;
   
  TH1D *hTRDEnergy_el, *hTRDMomentum_el, *hTRDTheta_el, *hTRDEnergyDiff_el, *hTRDEnergy_pi, *hTRDMomentum_pi, *hTRDTheta_pi, *hTRDEnergyDiff_pi;
  
  TH1D *hnumElTracks, *hnumPiTracks, *hnumTracks, *hnumTracksInTRD, *hnumTracksGoodExtrap;
  
  //--New
  TH2D *hFCALExtrapXY_TRD, *hExtrapXYDiff_FCAL_TRD, *hFCALExtrapEPvsP_TRD, *hExtrapThetavsP_TRD, *hExtrapThetavsP_Selected_TRD, *hSeenPointsXY, *hExtrapsXY, *hSeenPointsFCALXY, *hExtrapsFCALXY, *hSeenPointsSingleXY;
  TH1D *hExtrapTheta_TRD, *hExtrapXDiff_FCAL_TRD, *hExtrapYDiff_FCAL_TRD, *hFCALExtrapE_TRD, *hFCALExtrapEP_TRD, *hnumSeenExtrap, *hnumPointsSeen, *hnumExtrap, *hnumSeenExtrapFCAL, *hnumPointsSeenFCAL, *hExtrapsX, *hExtrapsY, *hSeenPointsSingleX, *hSeenPointsSingleY;
   
  TH1D *hnumSeenExtrapFCAL_el, *hnumExtrap_el, *hnumSeenExtrap_el, *hExtrapsX_el, *hExtrapsY_el, *hSeenPointsX_el, *hSeenPointsY_el, *hSeenPointsSingleX_el, *hSeenPointsSingleY_el, *hnumSeenExtrapFCAL_pi, *hnumExtrap_pi, *hnumSeenExtrap_pi, *hExtrapsX_pi, *hExtrapsY_pi, *hSeenPointsX_pi, *hSeenPointsY_pi, *hSeenPointsSingleX_pi, *hSeenPointsSingleY_pi, *hnumPointsSeen_el, *hnumPointsSeenFCAL_el, *hnumPointsSeen_pi, *hnumPointsSeenFCAL_pi;
  TH2D *hnumExtrapsXY_el, *hSeenPointsSingleXY_el, *hSeenPointsXY_el, *hSeenPointsFCALXY_el, *hProjectionXHitDiffvsTime_el, *hProjectionYHitDiffvsTime_el, *hTRDXCorr_el, *hTRDYCorr_el, *hnumExtrapsXY_pi, *hSeenPointsSingleXY_pi, *hSeenPointsXY_pi, *hSeenPointsFCALXY_pi, *hProjectionXHitDiffvsTime_pi, *hProjectionYHitDiffvsTime_pi, *hTRDXCorr_pi, *hTRDYCorr_pi, *hExtrapsXY_el, *hExtrapsXY_pi;
  
  std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()
};

#endif // _JEventProcessor_TRDTrack_

