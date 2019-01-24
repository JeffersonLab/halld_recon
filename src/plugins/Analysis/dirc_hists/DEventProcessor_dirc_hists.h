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
  jerror_t init(void);
  jerror_t brun(jana::JEventLoop *loop, int32_t runnumber);
  jerror_t evnt(JEventLoop *loop, uint64_t eventnumber);
  jerror_t erun(void);
  jerror_t fini(void); // called after last event

  TH2I* hDiffBar[48];
  TH1I* hNphCBar[48];
  map<Particle_t, TH1I*> hDiff;
  map<Particle_t, TH1I*> hNphC;
  map<Particle_t, TH1I*> hThetaC;
  map<Particle_t, TH1I*> hDeltaThetaC;
  map<Particle_t, TH1I*> hLikelihood;
  map<Particle_t, TH1I*> hLikelihoodDiff;
  
  map<Particle_t, TH2I*> hThetaCVsP;
  map<Particle_t, TH2I*> hDeltaThetaCVsP;
  map<Particle_t, TH2I*> hLikelihoodDiffVsP;
  map<Particle_t, TH2I*> hDeltaTVsP;

  int dMaxChannels;

  const DParticleID* dParticleID;
  const DDIRCGeometry* dDIRCGeometry;
  const DDIRCLut* dDIRCLut;
  
  deque<Particle_t> dFinalStatePIDs;
  bool DIRC_TRUTH_BARHIT;

};

#endif /* DEVENTPROCESSOR_DIRC_HIST_H_ */
