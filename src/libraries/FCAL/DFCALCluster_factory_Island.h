// $Id$
//
//    File: DFCALCluster_factory_Island.h
// Created: Fri Dec  4 08:25:47 EST 2020
// Creator: staylor (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DFCALCluster_factory_Island_
#define _DFCALCluster_factory_Island_

#include <JANA/JFactory.h>
#include "DFCALCluster.h"
#include "DFCALGeometry.h"
#include "DFCALHit.h"
#include "TMatrixD.h"
#include "TH1D.h"
#include "TH2D.h"

class DFCALCluster_factory_Island:public jana::JFactory<DFCALCluster>{
 public:
  DFCALCluster_factory_Island(){};
  ~DFCALCluster_factory_Island(){};
  const char* Tag(void){return "Island";}
  
 private:
  class PeakInfo{
  public:
  PeakInfo(double E,double x,double y):E(E),x(x),y(y){}
    double E;
    double x;
    double y;
  };
  
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  void FindClusterCandidates(vector<const DFCALHit*>&fcal_hits,
		    vector<vector<const DFCALHit*>>&clusterCandidates) const;
  bool FitPeaks(const TMatrixD &W,vector<const DFCALHit*>&hitList,
		vector<PeakInfo>&peaks,PeakInfo &myPeak,double &chisq) const;
  double CalcClusterEDeriv(const DFCALHit *hit,const PeakInfo &myPeakInfo) const;
  double CalcClusterXYDeriv(bool isXDeriv,const DFCALHit *hit,
			    const PeakInfo &myPeakInfo) const;
  void SplitPeaks(const TMatrixD &W,vector<const DFCALHit*>&hits,
		  vector<PeakInfo>&peaks,double &chisq) const;
  void CorrectPosition(int channel,double d,double &x,double &y) const;

  double TIME_CUT,MIN_CLUSTER_SEED_ENERGY,SHOWER_ENERGY_THRESHOLD;
  double SHOWER_WIDTH_PARAMETER;
  double INSERT_SHOWER_WIDTH_PARAMETER;
  double MIN_CUTDOWN_FRACTION,CHISQ_MARGIN;
  bool DEBUG_HISTS;

  double insert_Eres[3],Eres[3];
  double posConst1,posConst2,posConst3;
  double insertPosConst1,insertPosConst2,insertPosConst3;
  
  const DFCALGeometry *dFCALGeom=NULL;
  TH2D *HistdE;
  TH1D *HistProb;
};

#endif // _DFCALCluster_factory_Island_

