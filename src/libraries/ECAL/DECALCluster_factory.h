// $Id$
//
//    File: DECALCluster_factory.h
// Created: Tue Mar 25 10:45:18 AM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

#ifndef _DECALCluster_factory_
#define _DECALCluster_factory_

#include <JANA/JFactoryT.h>
#include "DECALCluster.h"
#include "DECALHit.h"
#include "DECALGeometry.h"
#include <TMatrixD.h>

class DECALCluster_factory:public JFactoryT<DECALCluster>{
public:
  DECALCluster_factory(){
    SetTag("");
  }
  ~DECALCluster_factory(){}

  class HitInfo{
  public:
    HitInfo(unsigned int id,int row,int column,double E,double x,double y,double t)
      :id(id),row(row),column(column),E(E),x(x),y(y),t(t){}
    unsigned int id;
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
  void Init() override;                       ///< Called once at program start.
  void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
  void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
  void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
  void Finish() override;                     ///< Called after last event of last event source has been processed.

  void FindClusterCandidates(vector<HitInfo>&hits,
			     vector<vector<HitInfo>>&clusterCandidates) const;

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
  double CalcClusterXYDeriv(bool isXDeriv,double b,const HitInfo &hit,
			    const PeakInfo &myPeakInfo) const;
  double CalcClusterEDeriv(double b,const HitInfo &hit,
			   const PeakInfo &myPeakInfo) const;
  bool FitPeaks(const TMatrixD &W,double b,vector<HitInfo>&hitList,
		vector<PeakInfo>&peaks,PeakInfo &myNewPeak,double &chisq,
		unsigned int &ndf) const;

  double TIME_CUT;
  double VAR_E_PAR0,VAR_E_PAR1,VAR_E_PAR2;
  double SHOWER_WIDTH_PAR0,SHOWER_WIDTH_PAR1;
  double CHISQ_MARGIN,MIN_CUTDOWN_FRACTION;
  double MIN_EXCESS_SEED_ENERGY,MIN_CLUSTER_SEED_ENERGY;
  bool SPLIT_PEAKS;

  const DECALGeometry *dECALGeom=NULL;
};

#endif // _DECALCluster_factory_

