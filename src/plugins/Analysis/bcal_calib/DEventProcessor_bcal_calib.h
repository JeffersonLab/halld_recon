// $Id$
//
//    File: DEventProcessor_bcal_calib.h
// Created: Thu Oct 18 17:15:41 EDT 2012
// Creator: staylor (on Linux ifarm1102 2.6.18-274.3.1.el5 x86_64)
//

#ifndef _DEventProcessor_bcal_calib_
#define _DEventProcessor_bcal_calib_

#include <pthread.h>
#include <map>
#include <vector>
#include <deque>
using std::map;

#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibration.h>
#include <JANA/Compatibility/jerror.h>

#include <PID/DKinematicData.h>
#include <CDC/DCDCTrackHit.h>
#include <BCAL/DBCALShower.h>
#include <DMatrixSIMD.h>
#include <DVector2.h>
#include <DVector3.h>

typedef struct{
  DMatrix4x1 S;
  DMatrix4x4 J;
  DMatrix4x1 Skk;
  DMatrix4x4 Ckk;
  double z,t;
  int num_hits;
  unsigned int h_id;
}trajectory_t;

typedef struct{
  bool matched;
  DVector3 dir;
  vector<const DCDCTrackHit *>hits;
}cdc_segment_t;

typedef struct{
  DVector3 dir;
  vector<const DCDCTrackHit *>axial_hits;
  vector<const DCDCTrackHit *>stereo_hits;
}cdc_track_t;

typedef struct{
  DVector3 dir;
  const DBCALShower *match;
}bcal_match_t;



#define EPS 1e-3
#define ITER_MAX 20
#define CDC_MATCH_RADIUS 5.0

class DEventProcessor_bcal_calib:public JEventProcessor{
 public:
  DEventProcessor_bcal_calib();
  ~DEventProcessor_bcal_calib();

  enum track_type{
    kWireBased,
    kTimeBased,
  };
  enum state_vector{
    state_x,
    state_y,
    state_tx,
    state_ty,
  };

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
  
  jerror_t GuessForStateVector(const cdc_track_t &track,DMatrix4x1 &S);
  jerror_t DoFilter(DMatrix4x1 &S,vector<const DCDCTrackHit *>&hits);
  jerror_t KalmanFilter(DMatrix4x1 &S,DMatrix4x4 &C,
			vector<const DCDCTrackHit *>&hits,
			deque<trajectory_t>&trajectory,
			double &chi2,unsigned int &ndof,
			bool timebased=false);
  jerror_t SetReferenceTrajectory(double z,DMatrix4x1 &S,
				  deque<trajectory_t>&trajectory,
				  const DCDCTrackHit *last_cdc); 
  jerror_t FindSegments(vector<const DCDCTrackHit*>&hits,
			vector<cdc_segment_t>&segments,
			vector<bool>&used_hits
			);
  jerror_t LinkSegments(vector<cdc_segment_t>&axial_segments,
			vector<bool>&used_axial,
			vector<const DCDCTrackHit *>&axial_hits,
			vector<const DCDCTrackHit *>&stereo_hits,
			vector<cdc_track_t>&LinkedSegments);
  bool MatchCDCHit(const DVector3 &vhat,const DVector3 &pos0,
		   const DCDCTrackHit *hit);

  void PlotLines(deque<trajectory_t>&traj);

  bool MatchToBCAL(vector<const DBCALShower *>&bcalshowers,DMatrix4x1 &S);

  unsigned int locate(vector<double>&xx,double x);
  double cdc_variance(double t); 
  double cdc_drift_distance(double t);
  double FindDoca(double z,const DMatrix4x1 &S,const DVector3 &vhat,
		  const DVector3 &origin);

  double GetDriftDistance(double t);
  double GetDriftVariance(double t);
    
  pthread_mutex_t mutex;

  TH1F *Hcdc_prob,*Hcdcmatch,*Hcdcmatch_stereo;
  TH2F *Hbcalmatchxy;

  double mT0;

  DMatrix4x1 Zero4x1;
  DMatrix4x4 Zero4x4;

  double mOuterTime,mOuterZ;
  
  // drift time tables
  vector<double>cdc_drift_table;
  
  // Resolution parameters
  double CDC_RES_PAR1,CDC_RES_PAR2;

  bool DEBUG_HISTS,DEBUG_PLOT_LINES;
};


// Smearing function derived from fitting residuals
inline double DEventProcessor_bcal_calib::cdc_variance(double t){ 
  //  return 0.001*0.001;
  //if (t<0.) t=0.;
  
  double sigma=CDC_RES_PAR1/(t+1.)+CDC_RES_PAR2;
  sigma+=0.025;
  return sigma*sigma;
}

#endif // _DEventProcessor_bcal_calib_
