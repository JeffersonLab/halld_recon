// $Id$
//
//    File: JEventProcessor_CalCal2.h
// Created: Tue Feb 10 11:29:02 AM EST 2026
// Creator: staylor (on Linux ifarm2402.jlab.org 5.14.0-611.20.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_CalCal2_
#define _JEventProcessor_CalCal2_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services
#include <TH1F.h>
#include <TH2F.h>
#include <TFitResult.h>
#include <PID/DChargedTrack.h>
#include <PID/DChargedTrackHypothesis.h>
#include <ECAL/DECALShower.h>
#include <ECAL/DECALGeometry.h>
#include <ECAL/DECALCluster.h>
#include <ECAL/DECALHit.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALGeometry.h>
#include <FCAL/DFCALCluster.h>
#include <FCAL/DFCALHit.h>
#include <DVector3.h>
#include <DLorentzVector.h>
#include <TRIGGER/DTrigger.h>
#include <TRACKING/DTrackCandidate.h>
#include <fstream>
#include "DANA/DEvent.h"

class JEventProcessor_CalCal2:public JEventProcessor{
public:
  JEventProcessor_CalCal2();
  ~JEventProcessor_CalCal2();
  const char* className(void){return "JEventProcessor_CalCal2";}
  
private:
  void Init() override;                       ///< Called once at program start.
  void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
  void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
  void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
  void Finish() override;                     ///< Called after last event of last event source has been processed.

  void ApplyConstraint(Particle_t particle,double one_minus_costheta12,
		       double E1S,double V1S,double E2S,double V2S,
		       vector<double>&E1vec,vector<double>&V1vec,
		       vector<double>&E2vec,vector<double>&V2vec) const;
  void EcalAnalysis(pair<DVector3,vector<const DECALHit*>>&cluster1,
		    pair<DVector3,vector<const DECALHit*>>&cluster2);
  void FcalAnalysis(pair<DVector3,vector<const DFCALHit*>>&cluster1,
		    pair<DVector3,vector<const DFCALHit*>>&cluster2);
  void EcalFcalAnalysis(pair<DVector3,vector<const DECALHit*>>&ecluster,
			pair<DVector3,vector<const DFCALHit*>>&fcluster);
  void MakeEcalVectors(vector<const DECALHit *>&hits,double &Esum,double &Vsum,
		       vector<double>&Evec,vector<double>&Vvec) const; 
  void MakeFcalVectors(vector<const DFCALHit *>&hits,double &Esum,double &Vsum,
		       vector<double>&Evec,vector<double>&Vvec) const;  
  void UpdateGains(vector<double>&Evec,vector<const DECALHit*>&hits);
  void UpdateGains(vector<double>&Evec,vector<const DFCALHit*>&hits);
  
  std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()

  double TRACK_CL_CUT,ESHOWER_MIN,EHIT_MIN,TIME_CUT;
  double PI0_LO_CUT,PI0_HI_CUT,ETA_LO_CUT,ETA_HI_CUT;
  double ECAL_VAR_E_PAR0,ECAL_VAR_E_PAR1,ECAL_VAR_E_PAR2;
  double FCAL_VAR_E_PAR0,FCAL_VAR_E_PAR1,FCAL_VAR_E_PAR2;

  const DECALGeometry *dECALGeom=NULL;
  vector<double>ecal_gains,old_ecal_gains,ecal_counts;

  const DFCALGeometry *dFCALGeom=NULL;
  vector<double>fcal_gains,old_fcal_gains,fcal_counts;
  vector<double>ecal_times,fcal_times;

  TH1F *hFcalGains,*hEcalGains;
  TH2F *hEcalDt,*hFcalDt,*hEcalGains2D,*hFcalGains2D;
  TH1F *hProtonECALGammaTimeDiff,*hProtonFCALGammaTimeDiff;
  TH1F *h2GammaMassECAL,*h2GammaMassFCAL,*h2GammaMassECALFCAL;
};

#endif // _JEventProcessor_CalCal2_

