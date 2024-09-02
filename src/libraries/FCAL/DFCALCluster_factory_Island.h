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
#include <ECAL/DECALHit.h>

#include "TMatrixD.h"
#include "TH1D.h"
#include "TH2D.h"

class DFCALCluster_factory_Island:public jana::JFactory<DFCALCluster>{
 public:
  DFCALCluster_factory_Island(){};
  ~DFCALCluster_factory_Island(){};
  const char* Tag(void){return "Island";}

  class HitInfo{
  public:
    HitInfo(JObject::oid_t id,int row,int column,double E,double x,double y,double t)
      :id(id),row(row),column(column),E(E),x(x),y(y),t(t){}
    JObject::oid_t id;
    int row;
    int column;
    double E;
    double x;
    double y;
    double t;
  };
  
  class PeakInfo{
  public:
  PeakInfo(double E,double x,double y,int ic,int ir,int nhits):E(E),x(x),y(y),ic(ic),ir(ir),nhits(nhits){}
    double E;
    double x;
    double y;
    int ic;
    int ir;
    int nhits;
  }; 
  
 private: 
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  void FindClusterCandidates(vector<HitInfo>&fcal_hits,
			     vector<vector<HitInfo>>&clusterCandidates) const;
  bool FitPeaks(const TMatrixD &W,double b,vector<HitInfo>&hitList,
		vector<PeakInfo>&peaks,PeakInfo &myPeak,double &chisq,
		unsigned int &ndf) const;
  double CalcClusterEDeriv(double b,const HitInfo &hit,const PeakInfo &myPeakInfo) const;
  double CalcClusterXYDeriv(bool isXDeriv,double b,const HitInfo &hit,
			    const PeakInfo &myPeakInfo) const;
  void SplitPeaks(const TMatrixD &W,double b,vector<HitInfo>&hits,
		  vector<PeakInfo>&peaks,double &chisq,unsigned int &ndf) const;
 
  double TIME_CUT,MIN_CLUSTER_SEED_ENERGY,SHOWER_ENERGY_THRESHOLD;
  double MIN_EXCESS_SEED_ENERGY;
  double SHOWER_WIDTH_PARAMETER,ENERGY_SHARING_CUTOFF;
  double INSERT_SHOWER_WIDTH_PAR0,INSERT_SHOWER_WIDTH_PAR1;
  double SHOWER_WIDTH_PAR0,SHOWER_WIDTH_PAR1,SHOWER_WIDTH_PAR2;
  double MIN_CUTDOWN_FRACTION,CHISQ_MARGIN,MASS_CUT;
  bool DEBUG_HISTS;
  unsigned int MAX_HITS_FOR_CLUSTERING;

  double m_insert_Eres[3],m_Eres[3];
  double m_zdiff;

  bool SPLIT_PEAKS,MERGE_HITS_AT_BOUNDARY;
  bool APPLY_S_CURVE_CORRECTION;
  double S_CURVE_PAR1,S_CURVE_PAR2,INSERT_S_CURVE_PAR1,INSERT_S_CURVE_PAR2;
  
  const DFCALGeometry *dFCALGeom=NULL;
  TH2D *HistdE;
  TH1D *HistProb;
};

#endif // _DFCALCluster_factory_Island_

