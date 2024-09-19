// $Id$
//
//    File: DFCALCluster_factory_Island.h
// Created: Fri Dec  4 08:25:47 EST 2020
// Creator: staylor (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DFCALCluster_factory_Island_
#define _DFCALCluster_factory_Island_

#include <JANA/JFactoryT.h>
#include "DFCALCluster.h"
#include "DFCALGeometry.h"
#include "DFCALHit.h"
#include <ECAL/DECALHit.h>
#include "DANA/DObjectID.h"

#include "TMatrixD.h"
#include "TH1D.h"
#include "TH2D.h"

class DFCALCluster_factory_Island:public JFactoryT<DFCALCluster>{
 public:
  DFCALCluster_factory_Island(){
    SetTag("Island");
  };
  ~DFCALCluster_factory_Island(){};
  class HitInfo{
  public:
    HitInfo(int id,int row,int column,double E,double x,double y,double t)
      :id(id),row(row),column(column),E(E),x(x),y(y),t(t){}
    oid_t id;
    int row;
    int column;
    double E;
    double x;
    double y;
    double t;
  };
  
  class PeakInfo{
  public:
    PeakInfo(double E,double x,double y,int ic,int ir,int status):E(E),x(x),y(y),ic(ic),ir(ir),status(status){}
    double E;
    double x;
    double y;
    int ic;
    int ir;
    int status;
  }; 
  
 private: 

  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  void FindClusterCandidates(vector<HitInfo>&fcal_hits,
			     vector<vector<HitInfo>>&clusterCandidates) const;
  bool FitPeaks(const TMatrixD &W,double b,vector<HitInfo>&hitList,
		vector<PeakInfo>&peaks,PeakInfo &myPeak,double &chisq,
		unsigned int &ndf) const;
  double CalcClusterEDeriv(double b,const HitInfo &hit,const PeakInfo &myPeakInfo) const;
  double CalcClusterXYDeriv(bool isXDeriv,double b,const HitInfo &hit,
			    const PeakInfo &myPeakInfo) const;
  void GetRowColRanges(int idiff,int nrows,int ncols,int ir,int ic,int &lo_row,
		       int &hi_row, int &lo_col,int &hi_col) const{
    lo_col=ic-idiff;
    if (lo_col<0) lo_col=0;
    hi_col=ic+idiff;
    if (hi_col>=ncols) hi_col=ncols-1;
    lo_row=ir-idiff;
    if (lo_row<0) lo_row=0;
    hi_row=ir+idiff;
    if (hi_row>=nrows) hi_row=nrows-1;
  }

  double TIME_CUT,MIN_CLUSTER_SEED_ENERGY,SHOWER_ENERGY_THRESHOLD;
  double MIN_EXCESS_SEED_ENERGY,MIN_E_FRACTION;
  double SHOWER_WIDTH_PARAMETER,ENERGY_SHARING_CUTOFF;
  double INSERT_SHOWER_WIDTH_PAR0,INSERT_SHOWER_WIDTH_PAR1;
  double SHOWER_WIDTH_PAR0,SHOWER_WIDTH_PAR1;
  double MIN_CUTDOWN_FRACTION,CHISQ_MARGIN,MASS_CUT;
  bool DEBUG_HISTS;
  unsigned int MAX_HITS_FOR_CLUSTERING;

  double m_insert_Eres[3],m_Eres[3];
  double m_zdiff;

  bool SPLIT_PEAKS,MERGE_HITS_AT_BOUNDARY;
  bool APPLY_S_CURVE_CORRECTION;
  double S_CURVE_PAR1,S_CURVE_PAR2,S_CURVE_PAR3;
  double INSERT_S_CURVE_PAR1,INSERT_S_CURVE_PAR2,INSERT_S_CURVE_PAR3;
  
  const DFCALGeometry *dFCALGeom=NULL;
  TH2D *HistdE=NULL;
  TH1D *HistProb=NULL;
};

#endif // _DFCALCluster_factory_Island_

