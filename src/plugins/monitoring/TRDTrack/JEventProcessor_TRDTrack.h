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
  void Count(const char *tit);
   
private:
  void Init() override;                       ///< Called once at program start.
  void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
  void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
  void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
  void Finish() override;                     ///< Called after last event of last event source has been processed.
  
  //-Sim
  TH2D *hTRDExtrapXY, *hExtrapXYHitDiff, *hTRDXCorr, *hTRDYCorr, *hFCALExtrapThetavsP, *hFCALExtrapThetavsP_Selected, *hExtrapXHitDiffvsTime, *hExtrapYHitDiffvsTime;
  TH1D *hExtrapPx, *hExtrapPy, *hExtrapPz, *hExtrapXHitDiff, *hExtrapYHitDiff, *hFCALExtrapTheta;
  //-Projection
  TH2D *hExtrapXYHitDiff_el, *hExtrapXYHitDiff_pi;
  TH1D *hExtrapXHitDiff_el, *hExtrapYHitDiff_el, *hExtrapXHitDiff_pi, *hExtrapYHitDiff_pi;
  //-Cal
  TH2D *hFCALMatchXYDisplay_el, *hFCALMatchXYDisplay_pi;
  TH1D *hFCALExtrapE_TRD, *hFCALEP_TRD_el, *hFCALEP_TRD_pi, *hFCALEP_cut_el, *hFCALEP_cut_pi, *hFCALExtrapEP_TRD;
 
  TH2D *hFCALExtrapXY, *hFCALExtrapEPvsP, *hFCALShowerXY, *hFCALXCorr, *hFCALYCorr, *hFCALTimeCorr;
  TH1D *hFCALExtrapPx, *hFCALExtrapPy, *hFCALExtrapPz, *hFCALExtrapE, *hFCALExtrapEP, *hFCALXDiff, *hFCALYDiff, *hFCALShowerTime, *hFCALExtrapTime, *hFCALFlightTime;
  
  TH1D *hHypEnergy_el, *hHypMomentum_el, *hHypTheta_el, *hHypEnergyDiff_el, *hHypEnergy_pi, *hHypMomentum_pi, *hHypTheta_pi, *hHypEnergyDiff_pi, *hTrackMult, *hCount, *hHypMomentumDiff, *hTrackingFOMChisq, *hTrackingFOMNdof;
  
  TH2D *hFCALExtrapXY_TRD, *hFCALShowerXY_TRD, *hFCALExtrapEPvsP_ext, *hFCALExtrapThetavsP_TRD, *hFCALExtrapThetavsP_Selected_ext, *hSeenPointsXY, *hExtrapsXY, *hFCALExtrapXY_p1_TRD, *hSeenPointsSingleXY, *hSeenHitsSingleXY;
  TH1D *hFCALExtrapTheta_TRD, *hFCALExtrapE_ext, *hFCALExtrapEP_ext, *hnumPointsSeen, *hnumExtrap, *hExtrapsX, *hExtrapsY, *hSeenPointsSingleX, *hSeenPointsSingleY, *hSeenHitsSingleX, *hSeenHitsSingleY;
  
  TH1D *hnumSeenExtrapFCAL_el, *hnumSeenExtrap_el, *hExtrapsX_el, *hExtrapsY_el, *hSeenPointsX_el, *hSeenPointsY_el, *hSeenPointsSingleX_el, *hSeenPointsSingleY_el, *hnumSeenExtrapFCAL_pi, *hnumSeenExtrap_pi, *hExtrapsX_pi, *hExtrapsY_pi, *hSeenPointsX_pi, *hSeenPointsY_pi, *hSeenPointsSingleX_pi, *hSeenPointsSingleY_pi, *hnumPointsSeen_el, *hnumPointsSeenFCAL_el, *hnumPointsSeen_pi, *hnumPointsSeenFCAL_pi;
  
  TH2D *hnumExtrapsXY_el, *hSeenPointsSingleXY_el, *hSeenPointsXY_el, *hXHitvsTime_el, *hYHitvsTime_el, *hXHitvsTime_QW_el, *hYHitvsTime_QW_el, *hXHitvsTime_Qmax_el, *hYHitvsTime_Qmax_el, *hXHitvsTime_Qmax_QW_el, *hYHitvsTime_Qmax_QW_el, *hnumExtrapsXY_pi, *hSeenPointsSingleXY_pi, *hSeenPointsXY_pi, *hXHitvsTime_pi, *hYHitvsTime_pi, *hXHitvsTime_QW_pi, *hYHitvsTime_QW_pi,  *hXHitvsTime_Qmax_pi, *hYHitvsTime_Qmax_pi, *hXHitvsTime_Qmax_QW_pi, *hYHitvsTime_Qmax_QW_pi, *hExtrapsXY_el, *hExtrapsXY_pi;
  
  std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()
};

#endif // _JEventProcessor_TRDTrack_

