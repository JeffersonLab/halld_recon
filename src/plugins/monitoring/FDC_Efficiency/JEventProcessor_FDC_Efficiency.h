// $Id$
//
//    File: JEventProcessor_FDC_Efficiency.cc
// Created: Thu May 26 15:57:38 EDT 2016
// Creator: aaustreg
//

#ifndef _JEventProcessor_FDC_Efficiency_
#define _JEventProcessor_FDC_Efficiency_

//#include <pthread.h>
#include <map>
#include <vector>
#include <deque>
using namespace std;

#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TMath.h>

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <JANA/Calibrations/JCalibration.h>

#include <HDGEOMETRY/DGeometry.h>
#include <TRACKING/DTrackCandidate_factory_StraightLine.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <TRACKING/DTrackWireBased.h>
#include <PID/DChargedTrack.h>
#include <PID/DDetectorMatches.h>
#include <FDC/DFDCHit.h>

class JEventProcessor_FDC_Efficiency:public JEventProcessor{
 public:
  JEventProcessor_FDC_Efficiency();
  ~JEventProcessor_FDC_Efficiency();

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

  DGeometry * dgeom;
  bool dIsNoFieldFlag;
  vector< vector< DFDCWire * > > fdcwires; // FDC Wires Referenced by [layer 1-24][wire 1-96]
  vector<double> fdcz; // FDC z positions
  vector<double> fdcrmin; // FDC inner radii
  double fdcrmax; // FDC outer radius
};

#endif // _JEventProcessor_FDC_Efficiency_

