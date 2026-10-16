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
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>
#include <TF1.h>

class JEventProcessor_TRDTrack:public JEventProcessor{
public:
  JEventProcessor_TRDTrack();
  ~JEventProcessor_TRDTrack();
  const char* className(void){return "JEventProcessor_TRDTrack";}
  void Count(const char *tit);
  void Count_el(const char *tit);
  void Count_pi(const char *tit);
   
private:
  void Init() override;                       ///< Called once at program start.
  void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
  void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
  void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
  void Finish() override;                     ///< Called after last event of last event source has been processed.
  
  //-Sim
  TH2D *hExtrapXYPointDiff, *hTRDXCorr, *hTRDYCorr, *hTRDXCorr_matched, *hTRDYCorr_matched, *hExtrapXHitDiffvsTime, *hExtrapYHitDiffvsTime, *hExtrapXPointDiffvsTime, *hExtrapYPointDiffvsTime, *hExtrapXPointDiffvsX, *hExtrapYPointDiffvsY, *hResXHitDiffvsTime, *hResYHitDiffvsTime;
  TH1D *hExtrapPx, *hExtrapPy, *hExtrapPz, *hExtrapXPointDiff, *hExtrapYPointDiff, *hResXHitDiff_Corrected, *hResYHitDiff_Corrected;
  //-Projection
  TH2D *hExtrapXYMaxPointDiff_el, *hExtrapXYMaxPointDiff_pi;
  TH1D *hExtrapXMaxPointDiff_el, *hExtrapYMaxPointDiff_el, *hExtrapXMaxPointDiff_pi, *hExtrapYMaxPointDiff_pi;
  //-Cal
  TH2D *hFCALMatchXYDisplay_el, *hFCALMatchXYDisplay_pi, *hFCALExtrapEPvsP_TRD;
  TH1D *hFCALExtrapE_TRD, *hFCALEP_TRD_el, *hFCALEP_TRD_pi, *hFCALEP_cut_el, *hFCALEP_cut_pi, *hFCALExtrapEP_TRD;
 
  TH2D *hFCALExtrapXY, *hFCALExtrapEPvsP, *hFCALShowerXY, *hFCALXCorr, *hFCALYCorr, *hFCALTimeCorr;
  TH1D *hFCALExtrapE, *hFCALExtrapEP, *hFCALXDiff, *hFCALYDiff, *hFCALShowerTime, *hFCALExtrapTime, *hFCALFlightTime;
  
  TH1D *hHypEnergy_el, *hHypMomentum_el, *hHypTheta_el, *hHypEnergyDiff_el, *hHypEnergy_pi, *hHypMomentum_pi, *hHypTheta_pi, *hHypEnergyDiff_pi, *hTrackMult, *hTrackingFOMChisq, *hTrackingFOMNdof;
  TH1D *hCount, *hCount_el, *hCount_pi;
  TH2D *hFCALExtrapXY_TRD, *hFCALShowerXY_TRD, *hFCALExtrapThetavsP_TRD, *hSeenPointsXY, *hExtrapsXY, *hFCALExtrapXY_p1_TRD, *hSeenPointsSingleXY, *hSeenHitsSingleXY;
  TH1D *hFCALExtrapTheta_TRD, *hnumPointsSeen, *hnumExtrap, *hExtrapsX, *hExtrapsY, *hSeenPointsSingleX, *hSeenPointsSingleY, *hSeenHitsSingleX, *hSeenHitsSingleY;
  
  TH1D *hnumSeenExtrapFCAL_el, *hnumSeenExtrap_el, *hExtrapsX_el, *hExtrapsY_el, *hSeenPointsX_el, *hSeenPointsY_el, *hSeenPointsSingleX_el, *hSeenPointsSingleY_el, *hnumSeenExtrapFCAL_pi, *hnumSeenExtrap_pi, *hExtrapsX_pi, *hExtrapsY_pi, *hSeenPointsX_pi, *hSeenPointsY_pi, *hSeenPointsSingleX_pi, *hSeenPointsSingleY_pi, *hnumPointsSeen_el, *hnumPointsSeenFCAL_el, *hnumPointsSeen_pi, *hnumPointsSeenFCAL_pi;
  
  TH2D *hnumExtrapsXY_el, *hSeenPointsSingleXY_el, *hSeenPointsXY_el, *hXPointvsTime_el, *hYPointvsTime_el, *hXPointvsTime_QW_el, *hYPointvsTime_QW_el, *hXPointvsTime_Qmax_el, *hYPointvsTime_Qmax_el, *hXPointvsTime_Qmax_QW_el, *hYPointvsTime_Qmax_QW_el, *hnumExtrapsXY_pi, *hSeenPointsSingleXY_pi, *hSeenPointsXY_pi, *hXPointvsTime_pi, *hYPointvsTime_pi, *hXPointvsTime_QW_pi, *hYPointvsTime_QW_pi,  *hXPointvsTime_Qmax_pi, *hYPointvsTime_Qmax_pi, *hXPointvsTime_Qmax_QW_pi, *hYPointvsTime_Qmax_QW_pi, *hExtrapsXY_el, *hExtrapsXY_pi, *hXPointvsTime_Qmax_Converted_el, *hYPointvsTime_Qmax_Converted_el, *hXPointvsTime_Qmax_Converted_pi, *hYPointvsTime_Qmax_Converted_pi, *hXPointvsTime_Converted_el, *hYPointvsTime_Converted_el, *hXPointvsTime_Converted_pi, *hYPointvsTime_Converted_pi;

  TH1D *hDL1Time, *hPSPairTime;
  
  //--Max Histos
  TH2D *hPoint_TimeVsdE_Max_el, *hPoint_TimeVsdE_Max_pi, *hPointH_TimeVsdE_Max_el, *hPointH_TimeVsdE_Max_pi;
  
  TGraphErrors *hEfficiencyFitsX, *hEfficiencyFitsY, *hFluxFits, *hPulseConstFitsX, *hPulseConstFitsY, *hPulseMPVFitsX, *hPulseMPVFitsY, *hPulseSigmaFitsX, *hPulseSigmaFitsY;
    
  std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()
};

#endif // _JEventProcessor_TRDTrack_

