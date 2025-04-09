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
  
  //-Segment
  TH2D *hTRDSegmentMatchXY_el, *hTRDSegmentMatchTxTy_el, *hTRDSegmentXY_el, *hTRDSegmentMatchXY_pi, *hTRDSegmentMatchTxTy_pi, *hTRDSegmentXY_pi;
  TH1D *hTRDSegmentMatchX_el, *hTRDSegmentMatchY_el, *hTRDSegmentMatchTx_el, *hTRDSegmentMatchTy_el, *hTRDSegmentMatchX_pi, *hTRDSegmentMatchY_pi, *hTRDSegmentMatchTx_pi, *hTRDSegmentMatchTy_pi;
  //-Sim
  TH2D *hTRDExtrapXY, *hExtrapXYDiff, *hExtrapXYHitDiff, *hSegmentExtrapXYDiff, *hTRDXCorr, *hTRDYCorr, *hExtrapThetavsP, *hExtrapThetavsP_Selected;
  TH1D *hTRDExtrapPx, *hTRDExtrapPy, *hTRDExtrapPz, *hExtrapXDiff, *hExtrapYDiff, *hExtrapXHitDiff, *hExtrapYHitDiff, *hSegmentExtrapXDiff, *hSegmentExtrapYDiff, *hExtrapTheta;
  //-Projection
  TH2D *hProjectionXYDiff_el, *hProjectionXYHitDiff_el, *hProjectionXYDiff_pi, *hProjectionXYHitDiff_pi;
  TH1D *hProjectionXDiff_el, *hProjectionYDiff_el, *hProjectionXHitDiff_el, *hProjectionYHitDiff_el, *hProjectionXDiff_pi, *hProjectionYDiff_pi, *hProjectionXHitDiff_pi, *hProjectionYHitDiff_pi;
  //-Cal
  TH2D *hfCALXY_el, *hfCALMatchXY_el, *hfCALMatchXYDisplay_el, *hfCALXY_pi, *hfCALMatchXY_pi, *hfCALMatchXYDisplay_pi;
  TH1D *hfCALShower_el, *hfCALEP_TRD_el, *hfCALMatchX_el, *hfCALMatchY_el, *hfCALShower_pi, *hfCALEP_TRD_pi, *hfCALMatchX_pi, *hfCALMatchY_pi, *hfCALEP_cut_el, *hfCALEP_cut_pi, *hfCALEP_el, *hfCALEP_pi;
 
  TH2D *hFCALExtrapXY, *hExtrapXYDiff_FCAL, *hFCALShowerDisplay, *hFCALExtrapEPvsP;
  TH1D *hFCALExtrapPx, *hFCALExtrapPy, *hFCALExtrapPz, *hExtrapXDiff_FCAL, *hExtrapYDiff_FCAL, *hFCALExtrapE, *hFCALExtrapEP;
   
  TH1D *hTRDFlightTime_el, *hTRDEnergy_el, *hTRDMomentum_el, *hTRDTheta_el, *hTRDEnergyDiff_el, *hTRDFlightTime_pi, *hTRDEnergy_pi, *hTRDMomentum_pi, *hTRDTheta_pi, *hTRDEnergyDiff_pi;
  
  TH1D *hnumElTracks, *hnumPiTracks, *hnumTrackMatches;
  
  
  std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()
};

#endif // _JEventProcessor_TRDTrack_

