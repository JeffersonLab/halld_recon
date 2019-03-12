// $Id$
//
//    File: DTrackFitterStraightTrack.h
// Created: Tue Mar 12 10:22:32 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DTrackFitterStraightTrack_
#define _DTrackFitterStraightTrack_

#include <JANA/jerror.h>

#include <TRACKING/DTrackFitter.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h> 
#include <DMatrixSIMD.h>
#include <deque>
#include <TMatrixFSym.h>
#include "DResourcePool.h"

class DTrackFitterStraightTrack: public DTrackFitter { 
public:
  DTrackFitterStraightTrack(JEventLoop *loop);
  ~DTrackFitterStraightTrack();  
  
  enum state_vector{
    state_x,
    state_y,
  state_tx,
    state_ty,
  };
  
  class trajectory_t{
  public:
trajectory_t(double z,double t,DMatrix4x1 S,DMatrix4x4 J,DMatrix4x1 Skk,
	       DMatrix4x4 Ckk,unsigned int id=0,unsigned int numhits=0)
    :z(z),t(t),S(S),J(J),Skk(Skk),Ckk(Ckk),id(id),numhits(numhits){}
    double z,t; 
    DMatrix4x1 S;
    DMatrix4x4 J;
    DMatrix4x1 Skk;
    DMatrix4x4 Ckk;
    unsigned int id,numhits;   
  };
  
  typedef struct{
    double resi,err,d,delta,tdrift,ddrift,s,V;
    DMatrix4x1 S;
    DMatrix4x4 C;
  }cdc_update_t;
  

  // Virtual methods from TrackFitter base class
  string Name(void) const {return string("StraightTrack");}
  fit_status_t FitTrack(void);

unsigned int Locate(const vector<double>&xx,double x) const;
double CDCDriftVariance(double t) const;
double CDCDriftDistance(double dphi,double delta,double t) const;
  jerror_t SetReferenceTrajectory(double t0,double z,DMatrix4x1 &S,
			 const DCDCTrackHit *last_cdc,double &dzsign);
  jerror_t KalmanFilter(DMatrix4x1 &S,DMatrix4x4 &C,vector<int>&used_hits,
			vector<cdc_update_t>&updates,double &chi2,
			unsigned int &ndof,unsigned int iter);
  jerror_t Smooth(vector<cdc_update_t>&cdc_updates);
	
 private:
  deque<trajectory_t>trajectory;

bool COSMICS,DO_PRUNING;
int VERBOSE;
double CHI2CUT;   

// drift time tables
vector<double>cdc_drift_table;
vector<double>fdc_drift_table;

// Resolution parameters
double CDC_RES_PAR1,CDC_RES_PAR2,CDC_RES_PAR3;
// Parameters for fdc drift resolution
double DRIFT_RES_PARMS[3];
double DRIFT_FUNC_PARMS[4];

// variables to deal with CDC straw sag
vector<vector<double> >max_sag;
vector<vector<double> >sag_phi_offset;
double long_drift_func[3][3];
double short_drift_func[3][3];

double cdc_endplate_z, cdc_endplate_rmin, cdc_endplate_rmax,cdc_length;
	
shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;


};

#endif // _DTrackFitterStraightTrack_

