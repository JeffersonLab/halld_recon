// -----------------------------------------
// DEventProcessor_dirc_hists.h
// -----------------------------------------

#ifndef DEVENTPROCESSOR_DIRC_HIST_H_
#define DEVENTPROCESSOR_DIRC_HIST_H_

#include <iostream>
#include <vector>
using namespace std;

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>
#include <JANA/JApplication.h>
#include <HDGEOMETRY/DGeometry.h>


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
#include <TLorentzVector.h>
#include <TVector3.h>
#include <TROOT.h>
#include <TClonesArray.h>


class DEventProcessor_dirc_hists: public JEventProcessor {

public:
  DEventProcessor_dirc_hists();
  ~DEventProcessor_dirc_hists();

  pthread_mutex_t mutex;

private:

  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

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
