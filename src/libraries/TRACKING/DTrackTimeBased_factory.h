// $Id$
//
//    File: DTrackTimeBased_factory.h
// Created: Thu Sep  4 14:02:44 EDT 2008
// Creator: davidl (on Darwin harriet.jlab.org 8.11.1 i386)
//

#ifndef _DTrackTimeBased_factory_
#define _DTrackTimeBased_factory_

#include <memory>

#include <TH2.h>

#include <JANA/JFactoryT.h>
#include <PID/DParticleID.h>
#include <BCAL/DBCALShower.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALHit.h>
#include <TOF/DTOFPoint.h>
#include <CDC/DCDCHit.h>
#include <START_COUNTER/DSCHit.h>
#include "PID/DParticleID.h"

#include "DMCThrown.h"
#include "DTrackFitter.h"
#include "DTrackTimeBased.h"
#include "DReferenceTrajectory.h"

class DTrackWireBased;
class DTrackHitSelector;
class DParticleID;


using namespace std;

class DTrackTimeBased_factory:public JFactoryT<DTrackTimeBased>{
  
 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;


  bool DEBUG_HISTS;
  int DEBUG_LEVEL;
  bool PID_FORCE_TRUTH;
  bool USE_HITS_FROM_WIREBASED_FIT;

  bool SAVE_TRUNCATED_DEDX;       // store extra dEdx information in time based tracks

  DTrackFitter *fitter;
  const DParticleID* pid_algorithm;
  vector<int> mass_hypotheses_positive;
  vector<int> mass_hypotheses_negative;
 
  // Optional debugging histograms
  TH1F *fom_chi2_trk;
  TH1F *fom;
  TH1F *hitMatchFOM;
  TH2F *chi2_trk_mom;
  TH2F *Hstart_time;
 
  void FilterDuplicates(void);  
  double GetTruthMatchingFOM(int trackIndex,DTrackTimeBased *dtrack,vector<const DMCThrown*>mcthrowns);
  int GetThrownIndex(vector<const DMCThrown*>& locMCThrowns, const DKinematicData *kd, double &f);

  void CreateStartTimeList(const DTrackWireBased *track,
			   vector<const DSCHit*>&sc_hits,
			   vector<const DTOFPoint*>&tof_points,
			   vector<const DBCALShower*>&bcal_showers,	  
			   vector<const DFCALShower*>&fcal_showers,
			   vector<const DFCALHit*>&fcal_hits,
			   vector<DTrackTimeBased::DStartTime_t>&start_times);
  bool DoFit(const DTrackWireBased *track,
	     vector<DTrackTimeBased::DStartTime_t>&start_times,
	     const std::shared_ptr<const JEvent>&event,double mass);  

  void AddMissingTrackHypothesis(vector<DTrackTimeBased*>&tracks_to_add,
				 const DTrackTimeBased *src_track,
				 double my_mass,double q,
				 const std::shared_ptr<const JEvent>&event);
  bool InsertMissingHypotheses(const std::shared_ptr<const JEvent>&event);
  void CorrectForELoss(DVector3 &position,DVector3 &momentum,double q,double my_mass);
  void AddMissingTrackHypotheses(unsigned int mass_bits,
				 vector<DTrackTimeBased*>&tracks_to_add,
				 vector<DTrackTimeBased *>&hypotheses,
				 double q,bool flipped_charge,const std::shared_ptr<const JEvent>&event);

  // Geometry
  const DGeometry *geom;
  vector<vector<DCDCWire *> > cdcwires;
  vector<double> cdc_rmid;
  vector<double>fdc_z_wires;
  double fdc_package_size;
  double fdc_rmax;
  vector<double> fdc_rmin_packages;
  double TARGET_Z=65.;

  //  double mPathLength,mEndTime,mStartTime,mFlightTime;
  double mStartTime;
  //  DetectorSystem_t mDetector, mStartDetector;
  DetectorSystem_t mStartDetector;
  int mNumHypPlus,mNumHypMinus;
  bool dIsNoFieldFlag,INSERT_MISSING_HYPOTHESES;
  bool USE_SC_TIME; // use start counter hits for t0
  bool USE_FCAL_TIME; // use fcal hits for t0
  bool USE_BCAL_TIME; // use bcal hits for t0
  bool USE_TOF_TIME; // use tof hits for t0
//  double SC_DPHI_CUT_WB;

  // start counter geometry
//  double sc_light_guide_length_cor;
//  double sc_angle_cor;
  vector<DVector3>sc_pos;
  vector<DVector3>sc_norm;

  int myevt;
};

#endif // _DTrackTimeBased_factory_

