// -----------------------------------------
// DEventProcessor_truth_dirc.h
// -----------------------------------------

#ifndef DEVENTPROCESSOR_TRUTH_DIRC_H_
#define DEVENTPROCESSOR_TRUTH_DIRC_H_

#include <iostream>
#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>
#include <JANA/JApplication.h>
#include <HDGEOMETRY/DGeometry.h>


#include <TRACKING/DMCThrown.h>
#include <TRACKING/DMCTrackHit.h>
#include <PID/DKinematicData.h>
#include <PID/DBeamPhoton.h>
#include <DIRC/DDIRCTruthBarHit.h>
#include <DIRC/DDIRCTruthPmtHit.h>
#include "DIRC/DDIRCGeometry.h"
#include <DIRC/DDIRCPmtHit.h>

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


class DEventProcessor_truth_dirc: public JEventProcessor {

public:
  DEventProcessor_truth_dirc();
  ~DEventProcessor_truth_dirc() override;

  pthread_mutex_t mutex;

private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  TH1F *hTruthWavelength;
  TH1F *hTruthBarHitBar;
  TH2F *hTruthBarHitXY;
  TH2F *hTruthPmtHitZY_North, *hTruthPmtHitZY_South;
  TH2F *hTruthPmtHit_North, *hTruthPmtHit_South;
  TH2F *hTruthPixelHit_North, *hTruthPixelHit_South;
  TH2F *hPixelHit_North, *hPixelHit_South;
  TH2F *hTruthPixelHitTime;

};

#endif /* DEVENTPROCESSOR_TRUTH_DIRC_H_ */
