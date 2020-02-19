// -----------------------------------------
// DEventProcessor_dircml.h
// -----------------------------------------

#ifndef DEVENTPROCESSOR_DIRCML_H_
#define DEVENTPROCESSOR_DIRCML_H_

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


class DEventProcessor_dircml: public JEventProcessor {

public:
  DEventProcessor_dircml();
  ~DEventProcessor_dircml();

  pthread_mutex_t mutex;

private:
  jerror_t init(void);
  jerror_t brun(jana::JEventLoop *loop, int32_t runnumber);
  jerror_t evnt(JEventLoop *loop, uint64_t eventnumber);
  jerror_t erun(void);
  jerror_t fini(void); // called after last event

  TTree *dircmlTree;
  int NPixels;
  int PixelRow[1000];
  int PixelCol[1000];
  double PixelTime[1000];
};

#endif /* DEVENTPROCESSOR_DIRCML_H_ */
