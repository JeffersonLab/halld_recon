/**************************************************************************                                                             
* HallD software                                                          * 
* Copyright(C) 2022       GlueX and Prim_-D Collaborations               * 
*                                                                         *                                                                
* Author: The GlueX and Prim_-D Collaborations                           *                                                                
* Contributors: Igal Jaegle                                               *                                                               
*                                                                         *
*                                                                         *   
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#ifndef _JEventProcessor_cal_cal_
#define _JEventProcessor_cal_cal_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include "TTree.h"
#include "TH1.h"
#include "TH2.h"

#include <PID/DBeamPhoton.h>
#include <PID/DEventRFBunch.h>
#include <HDDM/DEventHitStatistics.h>
#include <FDC/DFDCPseudo.h>

#include <ECAL/DECALShower.h>
#include <FCAL/DFCALShower.h>
#include <BCAL/DBCALShower.h>
#include <TOF/DTOFPoint.h>
//#include "PID/DVertex.h"
#include <PID/DNeutralParticle.h>
//#include <PID/DParticleID.h>
//#include <CCAL/DCCALGeometry.h>
#include <FCAL/DFCALGeometry.h>
#include <ECAL/DECALGeometry.h>
#include <FCAL/DFCALCluster.h>
#include <ECAL/DECALCluster.h>
#include <BCAL/DBCALHit.h>
#include <FCAL/DFCALHit.h>
#include <ECAL/DECALHit.h>
//#include <FCAL/DFCALDigiHit.h>
#include <TRIGGER/DL1Trigger.h>
#include <TRIGGER/DL1MCTrigger.h>
#include <START_COUNTER/DSCHit.h>
#include "TRACKING/DMCThrown.h"
#include <HDGEOMETRY/DGeometry.h>
//#include <PID/DChargedTrack.h>
//#include "TRACKING/DTrackTimeBased.h"
#include "HistogramTools.h"
#include "PID/DMCReaction.h"
#include "TProfile.h"
//#include <PID/DDetectorMatches.h>

#include "ANALYSIS/DTreeInterface.h"

//#include <KINFITTER/DKinFitter.h>
//#include <ANALYSIS/DKinFitUtils_GlueX.h>

#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"
#include "TRandom3.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <thread>
#include <mutex>

//using namespace jana;
using namespace std;



//class JEventProcessor_cal_cal:public jana::JEventProcessor{
class JEventProcessor_cal_cal:public JEventProcessor{
public:
  JEventProcessor_cal_cal() {};
  ~JEventProcessor_cal_cal() {};
  const char* className(void){return "JEventProcessor_cal_cal";}
  
private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()
  
  int get_ecal_layer(int,int);
  int get_fcal_layer(int,int);
  
  double m_TIME_CUT_RF_SC_MIN, m_TIME_CUT_RF_SC_MAX, m_ENERGY_CUT_SC;
  int check_SC_match(double phi, double rfTime, vector< const DSCHit* > locSCHits, double &dphi_min);
  int check_TOF_match(DVector3 fcalpos, double rfTime, DVector3 vertex, vector<const DTOFPoint*> tof_points, double &dx_min, double &dy_min);
  
  double m_TIME_CUT_RF_FCAL, m_TIME_CUT_RF_ECAL, m_TIME_CUT_RF_TOF, m_TIME_CUT_RF_TAG, m_FCAL1;
  double m_beamX, m_beamY, m_beamZ;
  double m_1stfdc_package;
  double m_fcalX, m_fcalY, m_fcalZ;
  double m_ccalX, m_ccalY, m_ccalZ;
  vector<vector<DVector3> >sc_pos;
  vector<vector<DVector3> >sc_norm;
  Particle_t TargetA;
  Particle_t TargetN;

  const DECALGeometry * ecalGeom = NULL;
  const DFCALGeometry * fcalGeom = NULL;
  double TargetMass;
  int m_rnb;
  double m_time_rf_offset;
  TH2F * h_ecal_e_v_ce;
  TH2F * h_ecal_e_ratio;
  TH2F * h_fcal_e_v_ce;
  TH2F * h_fcal_e_ratio;
  TH1F * h_ecal_cluster_size, * h_fcal_cluster_size;
  TH2F * h_ecal_cr_ring[35], * h_fcal_cr_ring[35];
  TH2F * h_ecal_blk_v_layer, * h_fcal_blk_v_layer;
  TH1F * h_ecal_layer, * h_fcal_layer;
  TH1F * h_ecal_ring, * h_fcal_ring;
  TH1F * h_trg_bit;
  TH2F * h_tag_time_rf_v_e;
  TH2F * h_tag_time_fcal_v_e;
  TH2F * h_tag_time_ecal_v_e;
  TH1F * h_tof_time;
  TH2F * h_sc_time_v_e;
  TH1F * h_esum_bcal[4], * h_esum_ecal[4], * h_esum_fcal[4], * h_esum_efcal[4];
  TH2F * h_esum_trg[4];
  TProfile *h_fcal_to;
  TProfile *h_fcal_gains;
  TProfile *h_fcal_qualities;
  TH2F * h_fcal_xy[20];
  TH2F * h_fcal_cr[20];
  TH2F * h_fcal_time[5];
  TH2F * h_fcal_time_v_bit[40];
  TH2F * h_fcal_mgg_v_blk[40];
  TH2F * h_fcal_fdc_mgg_v_blk[40];
  TH2F * h_fecal_mgg_v_blk[40];

  TH2F * h_ecal_mgg_v_layer;
  TH2F * h_fcal_mgg_v_layer;
  TH2F * h_ecal_mgg_v_ring;
  TH2F * h_fcal_mgg_v_ring;
  
  TProfile *h_ecal_to;
  TProfile *h_ecal_gains;
  TProfile *h_ecal_qualities;
  TH2F * h_ecal_xy[13];
  TH2F * h_ecal_cr[13];
  TH2F * h_ecal_time[5];
  TH2F * h_ecal_v_ecal_time[4];
  TH2F * h_fcal_v_fcal_time[4];
  TH2F * h_ecal_mgg_v_blk[40];
  TH2F * h_efcal_mgg_v_blk[40];
};

#endif // _JEventProcessor_cal_cal_

