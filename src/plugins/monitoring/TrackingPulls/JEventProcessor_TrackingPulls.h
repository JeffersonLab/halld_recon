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
  
  
	TH1I *hTrackingFOM;
	TH2I *hTrack_PVsTheta;
	TH2I *hTrack_PhiVsTheta;
	TH2I *hTrack_PVsPhi;

	TH1I *hTrackingFOM_SmoothFailure;
	TH2I *hTrack_PVsTheta_SmoothFailure;
	TH2I *hTrack_PhiVsTheta_SmoothFailure;
	TH2I *hTrack_PVsPhi_SmoothFailure;

	TH1I *hTrackingFOM_SmoothSuccess;
	TH2I *hTrack_PVsTheta_SmoothSuccess;
	TH2I *hTrack_PhiVsTheta_SmoothSuccess;
	TH2I *hTrack_PVsPhi_SmoothSuccess;

	TH1I *hTrackingFOM_SmoothSuccess_NaN;
	TH2I *hTrack_PVsTheta_SmoothSuccess_NaN;
	TH2I *hTrack_PhiVsTheta_SmoothSuccess_NaN;
	TH2I *hTrack_PVsPhi_SmoothSuccess_NaN;

	TH1I *hAllPulls;
	TH2I *hAllPulls_Vs_P;
	TH2I *hAllPulls_Vs_Phi;
	TH2I *hAllPulls_Vs_Theta;
	TH2I *hAllPulls_Vs_NDF;
	TH2I *hAllPulls_Vs_TrackingFOM;
	TH1I *hFDCWireError;
	TH1I *hFDCCathodeError;
	TH1I *hCDCError;

	TH1I *hFDCAllWirePulls;
	TH1I *hFDCAllCathodeulls;
	TH1I *hFDCAllWireResiduals;
	TH1I *hFDCAllCathodeResiduals;
	TH2I *hFDCAllWireResidualsVsPlane;
	TH2I *hFDCAllCathodeResidualsVsPlane;
	TH2I *hFDCAllWirePullsVsPlane;
	TH2I *hFDCAllCathodePullsVsPlane;
	TH2I *hFDCAllWireResidualsVsDriftTime;
	TH2I *hFDCAllWirePullsVsDriftTime;
	TH2I *hFDCAllWirePullsVsP;
	TH2I *hFDCAllWirePullsVsPhi;
	TH2I *hFDCAllWirePullsVsTheta;
	TH2I *hFDCAllCathodePullsVsP;
	TH2I *hFDCAllCathodePullsVsPhi;
	TH2I *hFDCAllCathodePullsVsTheta;
	TH2I *hFDCAllWireResidualsVsP;
	TH2I *hFDCAllWireResidualsVsPhi;
	TH2I *hFDCAllWireResidualsVsTheta;
	TH2I *hFDCAllCathodeResidualsVsP;
	TH2I *hFDCAllCathodeResidualsVsPhi;
	TH2I *hFDCAllCathodeResidualsVsTheta;
	TH2I *hFDCAllWirePullsVsNDF;
	TH2I *hFDCAllWirePullsVsTrackingFOM;
	TH2I *hFDCAllCathodePullsVsNDF;
	TH2I *hFDCAllCathodePullsVsTrackingFOM;

	vector<TH1I*> hFDCAllWirePulls_ByLayer;
	vector<TH1I*> hFDCAllCathodeulls_ByLayer;
	vector<TH1I*> hFDCAllWireResiduals_ByLayer;
	vector<TH1I*> hFDCAllCathodeResiduals_ByLayer;
	vector<TH1I*> hFDCWireResidualsGoodTracks_ByLayer;
	vector<TH1I*> hFDCWireResidualsGoodTracksRight_ByLayer;
	vector<TH1I*> hFDCWireResidualsGoodTracksLeft_ByLayer;
	vector<TH2I*> hFDCAllWireResidualsVsDriftTime_ByLayer;
	vector<TH2I*> hFDCAllWirePullsVsDriftTime_ByLayer;
	vector<TH2I*> hFDCAllWirePullsVsP_ByLayer;
	vector<TH2I*> hFDCAllWirePullsVsPhi_ByLayer;
	vector<TH2I*> hFDCAllWirePullsVsTheta_ByLayer;
	vector<TH2I*> hFDCAllCathodePullsVsP_ByLayer;
	vector<TH2I*> hFDCAllCathodePullsVsPhi_ByLayer;
	vector<TH2I*> hFDCAllCathodePullsVsTheta_ByLayer;
	vector<TH2I*> hFDCAllWireResidualsVsP_ByLayer;
	vector<TH2I*> hFDCAllWireResidualsVsPhi_ByLayer;
	vector<TH2I*> hFDCAllWireResidualsVsTheta_ByLayer;
	vector<TH2I*> hFDCAllCathodeResidualsVsP_ByLayer;
	vector<TH2I*> hFDCAllCathodeResidualsVsPhi_ByLayer;
	vector<TH2I*> hFDCAllCathodeResidualsVsTheta_ByLayer;

	vector<TH2I*> hFDCWirePulls_ByLayer;
	vector<TH2I*> hFDCWireResiduals_ByLayer;
	vector<TProfile2D*> hFDC2DWirePulls_ByLayer;
	vector<TProfile2D*> hFDC2DWireResiduals_ByLayer;
	vector<TProfile2D*> hFDC2DWireResidualsLocal_ByLayer;
	vector<TProfile2D*> hFDC2DCathodePulls_ByLayer;
	vector<TProfile2D*> hFDC2DCathodeResiduals_ByLayer;
	vector<TProfile2D*> hFDC2DCathodeResidualsLocal_ByLayer;

	TH1I *hCDCAllPulls;
	TH1I *hCDCAllResiduals;
	TH2I *hCDCAllResidualsVsRing;
	TH2I *hCDCAllPullsVsRing;
	TH2I *hCDCAllResidualsVsDriftTime;
	TH2I *hCDCAllPullsVsDriftTime;
	TH2I *hCDCAllPullsVsP;
	TH2I *hCDCAllPullsVsPhi;
	TH2I *hCDCAllPullsVsTheta;
	TH2I *hCDCAllResidualsVsP;
	TH2I *hCDCAllResidualsVsPhi;
	TH2I *hCDCAllResidualsVsTheta;
	TH2I *hCDCAllPullsVsNDF;
	TH2I *hCDCAllPullsVsTrackingFOM;
	
	vector<TH1I*> hCDCAllPulls_ByRing;
	vector<TH1I*> hCDCAllResiduals_ByRing;
	vector<TH2I*> hCDCAllResidualsVsDriftTime_ByRing;
	vector<TH2I*> hCDCAllPullsVsDriftTime_ByRing;
	vector<TH2I*> hCDCAllResidualsVsZ_ByRing;
	vector<TH2I*> hCDCAllPullsVsZ_ByRing;
	vector<TH2I*> hCDCAllPullsVsP_ByRing;
	vector<TH2I*> hCDCAllPullsVsPhi_ByRing;
	vector<TH2I*> hCDCAllPullsVsTheta_ByRing;
	vector<TH2I*> hCDCAllResidualsVsP_ByRing;
	vector<TH2I*> hCDCAllResidualsVsPhi_ByRing;
	vector<TH2I*> hCDCAllResidualsVsTheta_ByRing;
	vector<TH2I*> hCDCStrawPulls_ByRing;
	vector<TH2I*> hCDCStrawResiduals_ByRing;

	vector<TProfile2D*> hCDCStrawResidualsVsPhiTheta_ByRing;
	vector<TProfile2D*> hCDCStrawResidualsVsPhiZ_ByRing;
	vector<TProfile2D*> hCDCStrawResidualsVsPhiIntersectZ_ByRing;
	vector<TProfile2D*> hCDCStrawResidualsVsPTheta_ByRing;

};

#endif  // _JEventProcessor_TrackingPulls_
