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

class DFCALCluster_factory_Island:public jana::JFactory<DFCALCluster>{
 public:
  DFCALCluster_factory_Island(){};
  ~DFCALCluster_factory_Island(){};
  const char* Tag(void){return "Island";}
  
 private:
  class PeakInfo{
  public:
    PeakInfo(int row_index,int col_index,double E,double x,double y,double t)
      :row_index(row_index),col_index(col_index),E(E),x(x),y(y),t(t){}
    int row_index;
    int col_index;
    double E;
    double x;
    double y;
    double t;
  };
  
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  void FindClusterCandidates(vector<const DFCALHit*>&fcal_hits,
		    vector<vector<const DFCALHit*>>&clusterCandidates) const;
  void FitPeaks(vector<const DFCALHit*>&hitList,
		vector<PeakInfo>&peaks) const;
  double CalcClusterEDeriv(const DFCALHit *hit,
			   const PeakInfo &myPeakInfo) const;
  double CalcClusterXYDeriv(bool isXDeriv,const DFCALHit *hit,
			    const PeakInfo &myPeakInfo) const;

  double TIME_CUT;
  const DFCALGeometry *dFCALGeom=NULL;
};

#endif // _DFCALCluster_factory_Island_

