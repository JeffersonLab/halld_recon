// $Id$
//
//    File: DNeutralShower_factory.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DNeutralShower_factory_
#define _DNeutralShower_factory_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <PID/DNeutralShower.h>
#include <PID/DChargedTrack.h>
#include <PID/DChargedTrackHypothesis.h>
#include <TOF/DTOFPoint.h>
#include <START_COUNTER/DSCHit.h>
#include <FCAL/DFCALShower.h>
#include <BCAL/DBCALShower.h>
#include <CCAL/DCCALShower.h>
#include "DResourcePool.h"
#include "DVector3.h"

#include "DNeutralShower_FCALQualityMLP.h"

using namespace std;


class DNeutralShower_factory:public JFactoryT<DNeutralShower>
{
 public:
  DNeutralShower_factory();
  ~DNeutralShower_factory(){ delete dFCALClassifier;}

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;
  DVector3 dTargetCenter;
  double m_beamSpotX;
  double m_beamSpotY;  
  int RunNumber;
  
  const char* inputVars[8] = { "nHits", "e9e25Sh", "e1e9Sh", "sumUSh", "sumVSh", "asymUVSh", "speedSh", "dtTrSh" };
  DNeutralShower_FCALQualityMLP* dFCALClassifier;

  double getFCALQuality( const DFCALShower* fcalShower, double rfTime ) const;
  double TOF_RF_CUT;
  double SC_RF_CUT_MIN;
  double SC_RF_CUT_MAX;
  double SC_Energy_CUT;
  int check_SC_match(double phi, double rfTime, vector< const DSCHit* > locSCHits, double &dphi_min);
  int check_TOF_match(DVector3 fcalpos, double rfTime, DVector3 vertex, vector<const DTOFPoint*> tof_points, double &dx_min, double &dy_min);
  
  vector<vector<DVector3> >sc_pos;
  vector<vector<DVector3> >sc_norm;
  
};

#endif // _DNeutralShower_factory_

