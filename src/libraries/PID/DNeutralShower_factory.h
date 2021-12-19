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

#include <JANA/JFactory.h>
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
using namespace jana;

class DNeutralShower_factory:public jana::JFactory<DNeutralShower>
{
 public:
  DNeutralShower_factory();
  ~DNeutralShower_factory(){ delete dFCALClassifier;}

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;
  DVector3 dTargetCenter;
  double m_beamSpotX;
  double m_beamSpotY;  
  int RunNumber;
  
  const char* inputVars[8] = { "nHits", "e9e25Sh", "e1e9Sh", "sumUSh", "sumVSh", "asymUVSh", "speedSh", "dtTrSh" };
  DNeutralShower_FCALQualityMLP* dFCALClassifier;

  double getFCALQuality( const DFCALShower* fcalShower, double rfTime ) const;
  bool STORE_VETO_INFO;
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

