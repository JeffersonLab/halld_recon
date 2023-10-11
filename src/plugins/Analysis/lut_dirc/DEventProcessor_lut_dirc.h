// -----------------------------------------
// DEventProcessor_lut_dirc.h
// created on: 29.11.2017
// Initial athor: r.dzhygadlo at gsi.de
// -----------------------------------------

#ifndef DEVENTPROCESSOR_LUT_DIRC_H_
#define DEVENTPROCESSOR_LUT_DIRC_H_

#include <iostream>
#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>
#include <JANA/JApplication.h>

#include <TRACKING/DMCThrown.h>
#include <TRACKING/DMCTrackHit.h>
#include <PID/DKinematicData.h>
#include <PID/DBeamPhoton.h>
#include <DIRC/DDIRCTruthBarHit.h>
#include <DIRC/DDIRCTruthPmtHit.h>

#include <TMath.h>
#include <TFile.h>
#include <TTree.h>
#include <TDirectoryFile.h>
#include <TThread.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <TROOT.h>
#include <TClonesArray.h>

#include "DrcLutNode.h"

class DEventProcessor_lut_dirc: public JEventProcessor {

public:
  DEventProcessor_lut_dirc();
  ~DEventProcessor_lut_dirc();

  TClonesArray *fLut[48];
  DrcLutNode *fLutNode;
  TTree *fTree;

  pthread_mutex_t mutex;

private:
  void Init() override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun(void);
  void Finish() override;

};

#endif /* DEVENTPROCESSOR_LUT_DIRC_H_ */
