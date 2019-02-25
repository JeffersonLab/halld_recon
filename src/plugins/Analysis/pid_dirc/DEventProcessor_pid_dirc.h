// -----------------------------------------
// DEventProcessor_pid_dirc.h
// created on: 07.04.2017
// initial athor: r.dzhygadlo at gsi.de
// -----------------------------------------

#ifndef DEVENTPROCESSOR_PID_DIRC_H_
#define DEVENTPROCESSOR_PID_DIRC_H_

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
#include <PID/DKinematicData.h>
#include <PID/DBeamPhoton.h>
#include <DIRC/DDIRCTruthBarHit.h>
#include <DIRC/DDIRCTruthPmtHit.h>

#include <DAQ/DDIRCTDCHit.h>
#include "TTAB/DTTabUtilities.h"
#include <DIRC/DDIRCTDCDigiHit.h>
#include <DIRC/DDIRCPmtHit.h>

#include <TRIGGER/DL1Trigger.h>
#include <DIRC/DDIRCLEDRef.h>

#include <TMath.h>
#include <TFile.h>
#include <TTree.h>
#include <TDirectoryFile.h>
#include <TThread.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <TROOT.h>
#include <TClonesArray.h>

#include "DrcHit.h"
#include "DrcEvent.h"

class DEventProcessor_pid_dirc: public JEventProcessor {

public:
  DEventProcessor_pid_dirc();
  ~DEventProcessor_pid_dirc();

  class hit_set {
  public:
    Int_t hits_cdc;		// Number of hits in CDC
    Int_t hits_fdc;		// Number of hits in FDC
    Int_t hits_bcal;	// Number of hits in BCAL
    Int_t hits_fcal;	// Number of hits in FCAL
    Int_t hits_upv;		// Number of hits in UPV
    Int_t hits_tof;		// Number of hits in TOF
    Int_t hits_rich;	// Number of hits in RICH
    Int_t hits_cere;	// Number of hits in Cherenkov
  };

  TClonesArray *fcEvent;
  DrcEvent *fEvent;
  DrcHit *fHit;
  TTree *fTree;

  pthread_mutex_t mutex;

private:
  jerror_t init(void);
  jerror_t brun(jana::JEventLoop *loop, int32_t runnumber);
  jerror_t evnt(JEventLoop *loop, uint64_t eventnumber);
  jerror_t erun(void);
  jerror_t fini(void); // called after last event

};

#endif /* DEVENTPROCESSOR_PID_DIRC_H_ */
