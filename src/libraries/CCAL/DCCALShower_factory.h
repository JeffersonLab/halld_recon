/*
 *  File: DCCALHit_factory.h
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */

#ifndef _DCCALShower_factory_
#define _DCCALShower_factory_

#include <JANA/JFactory.h>
#include <JANA/JEventLoop.h>

#include <JANA/JEvent.h>
#include <JANA/JCalibration.h>
#include <JANA/JResourceManager.h>

#include "CCAL/DCCALShower.h"
#include "CCAL/DCCALHit.h"
#include "CCAL/DCCALGeometry.h"
#include "HDGEOMETRY/DGeometry.h"

#include "ccal.h"

#include <DVector3.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <mutex>

using namespace std;
using namespace jana;


class DCCALShower_factory:public JFactory<DCCALShower>{
  
 public:
  DCCALShower_factory();
  ~DCCALShower_factory(){};
  
  /* 
  store these values before passing them on to the island code
  keep them simply public and static for now
  */
  
 private:
  
  JApplication *japp;
  
  jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);
  jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);
  
  void getHitPatterns(vector<const DCCALHit*> hitarray, 
		      vector<vector< const DCCALHit*>> &hitPatterns);
  
  void sortByTime(vector<const DCCALHit*> &hitarray, float hitTime);
  
  void cleanHitPattern(vector<const DCCALHit*> hitarray, 
		       vector<const DCCALHit*> &hitarrayClean);
  
  void processShowers(vector<gamma_t> gammas, DCCALGeometry ccalGeom, 
		      vector<const DCCALHit*> locHitPattern, int eventnumber, 
		      vector<ccalcluster_t> &ccalClusters, 
		      vector<cluster_t> &clusterStorage);
  
  double getEnergyWeightedTime(cluster_t clusterStorage, int nHits);
  double getCorrectedTime(double time, double energy);
  double getShowerDepth(double energy);
  double getCorrectedEnergy(double energy, int id);
  double nonlin_func(double e, int id);
  
  double m_zTarget;
  double m_beamSpotX,m_beamSpotY;
  double m_CCALfront;
  double m_CCALdX,m_CCALdY;	
  
  //-------------------  Nonlinearity & Timewalk Parameters -------------------//
  
  int CCAL_CHANS = DCCALGeometry::kCCALMaxChannels;
  vector< double > Nonlin_p0;
  vector< double > Nonlin_p1;
  vector< double > Nonlin_p2;
  vector< double > Nonlin_p3;
  
  vector< double > timewalk_p0;
  vector< double > timewalk_p1;
  vector< double > timewalk_p2;
  vector< double > timewalk_p3;
  
  //-----------------   Shower Profile Data & Channel Status  -----------------//
  
  double acell[501][501] = { {0.} };
  double  ad2c[501][501] = { {0.} };
  
  int stat_ch[MROW][MCOL];
  
  //----------------------- Island Functions -----------------------//
  
  void main_island(vector<int> &ia, vector<int> &id, vector<gamma_t> &gammas);
  
  int  clus_hyc(int nw, vector<int> &ia, vector<int> &id, vector<int> &lencl);
  
  void order_hyc(int nw, vector<int> &ia, vector<int> &id);
  
  void gams_hyc(int nadc, vector<int> &ia, vector<int> &id, int &nadcgam,
		vector<gamma_t> &gammas);
  
  int  peak_type(int ix, int iy);
  
  void gamma_hyc(int nadc, vector<int> ia, vector<int> id, double &chisq, 
		 double &e1, double &x1, double &y1, 
		 double &e2, double &x2, double &y2);
  
  void fill_zeros(int nadc, vector<int> ia, int &nneib, vector<int> &iaz);
  
  void mom1_pht(int nadc, vector<int> ia, vector<int> id, int nzero, 
		vector<int> iaz, double &e1, double &x1, double &y1);
  
  void chisq1_hyc(int nadc, vector<int> ia, vector<int> id, int nneib, 
		  vector<int> iaz, double e1, double x1, double y1, double &chisq);
  
  double sigma2(double dx, double dy, double fc, double e);
  double d2c(double x, double y);
  double cell_hyc(double dx, double dy);
  
  void tgamma_hyc(int nadc, vector<int> ia, vector<int> id, int nneib, 
		  vector<int> iaz, double &chisq, double &e1, double &x1, 
		  double &y1, double &e2, double &x2, double &y2);
  
  void mom2_pht(int nadc, vector<int> ia, vector<int> id, int nzero, 
		vector<int> iaz, double &a0, double &x0, double &y0, 
		double &xx, double &yy, double &yx);
  
  void c3to5_pht(double e0, double x0, double y0, double eps, double dx, 
		 double dy, double &e1, double &x1, double &y1, double &e2, 
		 double &x2, double &y2);
  
  void chisq2t_hyc(double ecell, double e1c, double dx1, double dy1, 
		   double e2c, double dx2, double dy2, double f1c, 
		   double f2c, double &chisqt);
  
  void ucopy1(vector<int> &ia, vector<int> &iwork, int start, int length);
  void ucopy2(vector<int> &ia, int start1, int start2, int length);
  void ucopy3(vector<int> &iwork, vector<int> &ia, int start, int length);
  
  //---------------------- Default Parameters ----------------------//

  int    VERBOSE;
  int    SHOWER_DEBUG;
  int    MIN_CLUSTER_BLOCK_COUNT;
  double MIN_CLUSTER_SEED_ENERGY;
  double MIN_CLUSTER_ENERGY;
  double MAX_CLUSTER_ENERGY;
  double TIME_CUT;
  int    MAX_HITS_FOR_CLUSTERING;
  int    DO_NONLINEAR_CORRECTION;
  int    DO_TIMEWALK_CORRECTION;
  
  double CCAL_RADIATION_LENGTH;
  double CCAL_CRITICAL_ENERGY;
  
  double LOG_POS_CONST;

  double CCAL_C_EFFECTIVE;
};

#endif // _DCCALShower_factory_
