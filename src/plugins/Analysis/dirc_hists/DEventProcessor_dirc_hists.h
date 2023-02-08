// -----------------------------------------
// DEventProcessor_dirc_hists.h
// -----------------------------------------

#ifndef DEVENTPROCESSOR_DIRC_HIST_H_
#define DEVENTPROCESSOR_DIRC_HIST_H_

#include <iostream>
#include <vector>
using namespace std;

#include <JANA/JFactory.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEventLoop.h>
#include <JANA/JApplication.h>
#include <DANA/DApplication.h>
#include <HDGEOMETRY/DGeometry.h>

using namespace jana;

#include <TRACKING/DMCThrown.h>
#include <TRACKING/DMCTrackHit.h>
#include <TRACKING/DTrackTimeBased.h>
#include <PID/DKinematicData.h>
#include <PID/DParticleID.h>
#include <DIRC/DDIRCTruthBarHit.h>
#include <DIRC/DDIRCTruthPmtHit.h>
#include <TRIGGER/DTrigger.h>

#include <TMath.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TDirectoryFile.h>
#include <TThread.h>
#include <DVector3.h>
#include <TROOT.h>
#include <TClonesArray.h>


class DEventProcessor_dirc_hists: public JEventProcessor {

public:
  DEventProcessor_dirc_hists();
  ~DEventProcessor_dirc_hists();

  pthread_mutex_t mutex;

private:
  jerror_t init(void);
  jerror_t brun(jana::JEventLoop *loop, int32_t runnumber);
  jerror_t evnt(JEventLoop *loop, uint64_t eventnumber);
  jerror_t erun(void);
  jerror_t fini(void); // called after last event

  TH2I* hDiffBar[DDIRCGeometry::kBars];
  TH1I* hNphCBar[DDIRCGeometry::kBars];
  TH1I* hNphCBarInclusive[DDIRCGeometry::kBars];
  TH2I *hNphCBarVsP[DDIRCGeometry::kBars];
  TH2I *hNphCBarInclusiveVsP[DDIRCGeometry::kBars];
  TH1I *hDeltaThetaCBar[DDIRCGeometry::kBars];
  TH2I *hDeltaThetaCVsDeltaYBar[DDIRCGeometry::kBars];
  TH2I *hDeltaThetaCVsDeltaYBarX[DDIRCGeometry::kBars][40];
  TH2I *hDeltaThetaCVsDeltaYBarXPos[DDIRCGeometry::kBars][40], *hDeltaThetaCVsDeltaYBarXNeg[DDIRCGeometry::kBars][40];
  map<Particle_t, TH2I*> hExtrapolatedBarHitXY;
  map<Particle_t, TH1I*> hExtrapolatedBarHitTime;
  map<Particle_t, vector<TH1I*>> hExtrapolatedBarHitTime_BadTime;
  map<Particle_t, vector<TH2I*>> hExtrapolatedBarHitXY_BadTime;
  map<Particle_t, vector<TH2I*>> hExtrapolatedBarTimeVsPixelHitTime;
  map<Particle_t, vector<TH2I*>> hExtrapolatedBarTimeVsPixelHitTime_BadTime;
  map<Particle_t, vector<TH2I*>> hExtrapolationTimeVsStartTime;
  map<Particle_t, vector<TH2I*>> hExtrapolationTimeVsStartTime_BadTime;
  map<Particle_t, vector<TH2I*>> hDiffVsEventNumber;
  map<Particle_t, vector<TH1I*>> hDiff;
  map<Particle_t, vector<TH2I*>> hDiffVsChannelDirect;
  map<Particle_t, vector<TH2I*>> hDiffVsChannelReflected;
  map<Particle_t, vector<TH1I*>> hNphC;
  map<Particle_t, vector<TH1I*>> hNphCInclusive;
  map<Particle_t, vector<TH1I*>> hThetaC;
  map<Particle_t, vector<TH1I*>> hDeltaThetaC;
  map<Particle_t, vector<TH1I*>> hDeltaThetaC_BadTime;
  map<Particle_t, vector<TH1I*>> hLikelihood;
  map<Particle_t, vector<TH1I*>> hLikelihoodDiff;
 
  map<Particle_t, vector<TH2I*>> hNphCVsP;
  map<Particle_t, vector<TH2I*>> hNphCInclusiveVsP;
  map<Particle_t, vector<TH2I*>> hThetaCVsP;
  map<Particle_t, vector<TH2I*>> hDeltaThetaCVsP;
  map<Particle_t, vector<TH2I*>> hLikelihoodDiffVsP;
  map<Particle_t, vector<TH2I*>> hDeltaTVsP;
  map<Particle_t, vector<TH2I*>> hTimeCalcVsMeas;
  map<Particle_t, vector<TH2S*>> hPixelHitMap_BadTime;

  TH1I *hHitTimeMap[40];
  TH2S *hPixelHitMap[40], *hPixelHitMapReflected[40];

  int dMaxChannels;
  
  deque<Particle_t> dFinalStatePIDs;
  bool DIRC_TRUTH_BARHIT;
  bool DIRC_BAR_DIAGNOSTIC;
  int DIRC_BAR_HIT_MAP;
  double DIRC_CUT_TDIFF;

};

#endif /* DEVENTPROCESSOR_DIRC_HIST_H_ */
