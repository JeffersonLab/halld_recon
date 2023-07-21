// $Id$
//
//    File: JEventProcessor_TrackingPulls.h
// Created: Thu Nov  3 14:30:19 EDT 2016
// Creator: mstaib (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TrackingPulls_
#define _JEventProcessor_TrackingPulls_

#include <JANA/JEventProcessor.h>
#include "ANALYSIS/DTreeInterface.h"
#include <TTree.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <vector>

class JEventProcessor_TrackingPulls : public jana::JEventProcessor {
 public:
  JEventProcessor_TrackingPulls();
  ~JEventProcessor_TrackingPulls();
  const char *className(void) { return "JEventProcessor_TrackingPulls"; }

  static const int kNumFdcPlanes = 24;
  static const int kNumCdcRings = 28;

 private:
  jerror_t init(void);  ///< Called once at program start.
  jerror_t brun(
      jana::JEventLoop *eventLoop,
      int32_t runnumber);  ///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop,
                uint64_t eventnumber);  ///< Called every event.
  jerror_t erun(void);  ///< Called everytime run number changes, provided brun
                        ///< has been called.
  jerror_t fini(void);  ///< Called after last event of last event source has
                        ///< been processed.

  //TREE
  bool MAKE_TREE;
  DTreeInterface* dTreeInterface;
  static thread_local DTreeFillData dTreeFillData;
  
  
	TH1F *hTrackingFOM;
	TH2F *hTrack_PVsTheta;
	TH2F *hTrack_PhiVsTheta;
	TH2F *hTrack_PVsPhi;

	TH1F *hTrackingFOM_SmoothFailure;
	TH2F *hTrack_PVsTheta_SmoothFailure;
	TH2F *hTrack_PhiVsTheta_SmoothFailure;
	TH2F *hTrack_PVsPhi_SmoothFailure;

	TH1F *hTrackingFOM_SmoothSuccess;
	TH2F *hTrack_PVsTheta_SmoothSuccess;
	TH2F *hTrack_PhiVsTheta_SmoothSuccess;
	TH2F *hTrack_PVsPhi_SmoothSuccess;

	TH1F *hTrackingFOM_SmoothSuccess_NaN;
	TH2F *hTrack_PVsTheta_SmoothSuccess_NaN;
	TH2F *hTrack_PhiVsTheta_SmoothSuccess_NaN;
	TH2F *hTrack_PVsPhi_SmoothSuccess_NaN;

	TH1F *hAllPulls;
	TH2F *hAllPulls_Vs_P;
	TH2F *hAllPulls_Vs_Phi;
	TH2F *hAllPulls_Vs_Theta;
	TH2F *hAllPulls_Vs_NDF;
	TH2F *hAllPulls_Vs_TrackingFOM;
	TH1F *hFDCWireError;
	TH1F *hFDCCathodeError;
	TH1F *hCDCError;

	TH1F *hFDCAllWirePulls;
	TH1F *hFDCAllCathodeulls;
	TH1F *hFDCAllWireResiduals;
	TH1F *hFDCAllCathodeResiduals;
	TH2F *hFDCAllWireResidualsVsPlane;
	TH2F *hFDCAllCathodeResidualsVsPlane;
	TH2F *hFDCAllWirePullsVsPlane;
	TH2F *hFDCAllCathodePullsVsPlane;
	TH2F *hFDCAllWireResidualsVsDriftTime;
	TH2F *hFDCAllWirePullsVsDriftTime;
	TH2F *hFDCAllWirePullsVsP;
	TH2F *hFDCAllWirePullsVsPhi;
	TH2F *hFDCAllWirePullsVsTheta;
	TH2F *hFDCAllCathodePullsVsP;
	TH2F *hFDCAllCathodePullsVsPhi;
	TH2F *hFDCAllCathodePullsVsTheta;
	TH2F *hFDCAllWireResidualsVsP;
	TH2F *hFDCAllWireResidualsVsPhi;
	TH2F *hFDCAllWireResidualsVsTheta;
	TH2F *hFDCAllCathodeResidualsVsP;
	TH2F *hFDCAllCathodeResidualsVsPhi;
	TH2F *hFDCAllCathodeResidualsVsTheta;
	TH2F *hFDCAllWirePullsVsNDF;
	TH2F *hFDCAllWirePullsVsTrackingFOM;
	TH2F *hFDCAllCathodePullsVsNDF;
	TH2F *hFDCAllCathodePullsVsTrackingFOM;

	vector<TH1F*> hFDCAllWirePulls_ByLayer;
	vector<TH1F*> hFDCAllCathodeulls_ByLayer;
	vector<TH1F*> hFDCAllWireResiduals_ByLayer;
	vector<TH1F*> hFDCAllCathodeResiduals_ByLayer;
	vector<TH1F*> hFDCWireResidualsGoodTracks_ByLayer;
	vector<TH1F*> hFDCWireResidualsGoodTracksRight_ByLayer;
	vector<TH1F*> hFDCWireResidualsGoodTracksLeft_ByLayer;
	vector<TH2F*> hFDCAllWireResidualsVsDriftTime_ByLayer;
	vector<TH2F*> hFDCAllWirePullsVsDriftTime_ByLayer;
	vector<TH2F*> hFDCAllWirePullsVsP_ByLayer;
	vector<TH2F*> hFDCAllWirePullsVsPhi_ByLayer;
	vector<TH2F*> hFDCAllWirePullsVsTheta_ByLayer;
	vector<TH2F*> hFDCAllCathodePullsVsP_ByLayer;
	vector<TH2F*> hFDCAllCathodePullsVsPhi_ByLayer;
	vector<TH2F*> hFDCAllCathodePullsVsTheta_ByLayer;
	vector<TH2F*> hFDCAllWireResidualsVsP_ByLayer;
	vector<TH2F*> hFDCAllWireResidualsVsPhi_ByLayer;
	vector<TH2F*> hFDCAllWireResidualsVsTheta_ByLayer;
	vector<TH2F*> hFDCAllCathodeResidualsVsP_ByLayer;
	vector<TH2F*> hFDCAllCathodeResidualsVsPhi_ByLayer;
	vector<TH2F*> hFDCAllCathodeResidualsVsTheta_ByLayer;

	vector<TH2F*> hFDCWirePulls_ByLayer;
	vector<TH2F*> hFDCWireResiduals_ByLayer;
	vector<TProfile2D*> hFDC2DWirePulls_ByLayer;
	vector<TProfile2D*> hFDC2DWireResiduals_ByLayer;
	vector<TProfile2D*> hFDC2DWireResidualsLocal_ByLayer;
	vector<TProfile2D*> hFDC2DCathodePulls_ByLayer;
	vector<TProfile2D*> hFDC2DCathodeResiduals_ByLayer;
	vector<TProfile2D*> hFDC2DCathodeResidualsLocal_ByLayer;

	TH1F *hCDCAllPulls;
	TH1F *hCDCAllResiduals;
	TH2F *hCDCAllResidualsVsRing;
	TH2F *hCDCAllPullsVsRing;
	TH2F *hCDCAllResidualsVsDriftTime;
	TH2F *hCDCAllPullsVsDriftTime;
	TH2F *hCDCAllPullsVsP;
	TH2F *hCDCAllPullsVsPhi;
	TH2F *hCDCAllPullsVsTheta;
	TH2F *hCDCAllResidualsVsP;
	TH2F *hCDCAllResidualsVsPhi;
	TH2F *hCDCAllResidualsVsTheta;
	TH2F *hCDCAllPullsVsNDF;
	TH2F *hCDCAllPullsVsTrackingFOM;
	
	vector<TH1F*> hCDCAllPulls_ByRing;
	vector<TH1F*> hCDCAllResiduals_ByRing;
	vector<TH2F*> hCDCAllResidualsVsDriftTime_ByRing;
	vector<TH2F*> hCDCAllPullsVsDriftTime_ByRing;
	vector<TH2F*> hCDCAllResidualsVsZ_ByRing;
	vector<TH2F*> hCDCAllPullsVsZ_ByRing;
	vector<TH2F*> hCDCAllPullsVsP_ByRing;
	vector<TH2F*> hCDCAllPullsVsPhi_ByRing;
	vector<TH2F*> hCDCAllPullsVsTheta_ByRing;
	vector<TH2F*> hCDCAllResidualsVsP_ByRing;
	vector<TH2F*> hCDCAllResidualsVsPhi_ByRing;
	vector<TH2F*> hCDCAllResidualsVsTheta_ByRing;
	vector<TH2F*> hCDCStrawPulls_ByRing;
	vector<TH2F*> hCDCStrawResiduals_ByRing;

	vector<TProfile2D*> hCDCStrawResidualsVsPhiTheta_ByRing;
	vector<TProfile2D*> hCDCStrawResidualsVsPhiZ_ByRing;
	vector<TProfile2D*> hCDCStrawResidualsVsPhiIntersectZ_ByRing;
	vector<TProfile2D*> hCDCStrawResidualsVsPTheta_ByRing;

};

#endif  // _JEventProcessor_TrackingPulls_
