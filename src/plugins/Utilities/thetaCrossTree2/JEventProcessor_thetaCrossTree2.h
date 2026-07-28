 // $Id$
//
//    File: JEventProcessor_thetaCrossTree.h
// Created: Fri Aug  8 03:02:21 PM EDT 2025
// Creator: shannen (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_thetaCrossTree2_
#define _JEventProcessor_thetaCrossTree2_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services


#include <ANALYSIS/DAnalysisUtilities.h>
#include "DANA/DEvent.h"
#include "HDGEOMETRY/DGeometry.h"
#include <RF/DRFTime.h>
#include <PID/DEventRFBunch.h>
#include <PID/DBeamPhoton.h>
#include <TOF/DTOFPoint.h>
#include <PID/DNeutralShower.h>
#include <PID/DChargedTrack.h>
//#include <TRACKING/DMCThrown.h>
//#include <TRACKING/DMCTrackHit.h>
//#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALGeometry.h>
#include <FCAL/DFCALShower.h>
#include <TRIGGER/DL1Trigger.h>
#include <TRIGGER/DTrigger.h>

#include <FMWPC/DFMWPCHit.h>

#include <TLorentzVector.h>
#include <TMinuit.h>

#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include "TProfile.h"
#include <TProfile2D.h>
#include <TDirectory.h>
#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>

//#include "dict.cc"

static const double clight  = 29.9792458;
static const double mpi0    = 0.1349766;
static const double mpic    = 0.13957;
static const double meta    = 0.547862;
static const double metap   = 0.95778;
//static const double clight  = 29.9792458;                                                                                                                                                                                                                                  

struct FitParams {double e1, e2, s1, s2, m0;};
thread_local FitParams* currentParams = nullptr;

void chi2func(int& npar, double* grad, double& fval, double* par, int flag) {
  double ec1 = par[0];
  double ec2 = par[1];
  const FitParams& p = *currentParams;

  double chi2 = (p.e1-ec1)*(p.e1-ec1)/p.s1/p.s1 + (p.e2-ec2)*(p.e2-ec2)/p.s2/p.s2;
  double pro;
  if(p.m0 < 0.45){
    pro  = p.e1*p.e2*mpi0*mpi0/p.m0/p.m0;
  }
  else{
    pro  = p.e1*p.e2*meta*meta/p.m0/p.m0;
  }
  double constraint = ec1 * ec2 - pro;
  fval = chi2 + 1.e9 * constraint * constraint;
}


class JEventProcessor_thetaCrossTree2:public JEventProcessor{
    public:
        JEventProcessor_thetaCrossTree2();
        ~JEventProcessor_thetaCrossTree2();
        const char* className(void){return "JEventProcessor_thetaCrossTree2";}

  double caleres(double e, const bool fcal){
    const float a = 2.4e-2, b = 5.4e-2, d = 0.75e-2;
    double sig = sqrt(a*a + b*b/e + d*d);
    sig *= e;
    if(!fcal) sig *=  2.;
    return sig;
  }
  void fitm0(double e1, double e2, double s1, double s2, double m0,
             double& ec1, double& ec2) {

    FitParams params{e1, e2, s1, s2, m0};
    currentParams = &params;

    TMinuit minuit(2);
    minuit.SetPrintLevel(-1);
    minuit.SetFCN(chi2func);

    minuit.DefineParameter(0, "ec1", e1, 1e-3*e1, 0.25*e1, 4*e1);
    minuit.DefineParameter(1, "ec2", e2, 1e-3*e2, 0.25*e2, 4*e2);

    minuit.Migrad();

    double err;
    minuit.GetParameter(0, ec1, err);
    minuit.GetParameter(1, ec2, err);

  }


  void get_pi0_momentum_and_mass(const DLorentzVector p1, const bool fcal1,
                                 const DLorentzVector p2, const bool fcal2,
                                 double &sc1,  double &sc2) {

    const double m0 = (p1+p2).M();
    double e1 = p1.E();
    double e2 = p2.E();
    const double s1  = caleres(e1,fcal1);
    const double s2  = caleres(e2,fcal2);

    double ec1, ec2;
    fitm0(e1,e2,s1,s2,m0,ec1,ec2);
    
    sc1 = ec1/e1;
    sc2 = ec2/e2;
    return;
  }
  void get_pi0_energyScaled(const double e1, const double e2, const double et, double &st1, double & st2, const bool fcal1, const bool fcal2){
    const double s1 = caleres(e1, fcal1);
    const double s2 = caleres(e2, fcal2);

    const double ec1 = e1 + (et - e1 - e2)/(s2/s1 + 1);
    const double ec2 = e2 + (et - e1 - e2)/(s1/s2 + 1);

    st1 = ec1/e1;
    st2 = ec2/e2;

    return;

  }

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

    	std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()

  TFile *ofile;
  TTree *tree1;

  TH1 * h, * Bunch_time_hist, * ThetaD, * ED, * ThetaDFCAL, * EDFCAL, * ThetaDBCAL, * EDBCAL;
  TH2 * FCal_centroid, * EThetaD, * EThetaDFCAL, * EThetaDBCAL;
  
  float runNum_fill, evtNum_fill, E1_fill, E2_fill, E1s_fill, E2s_fill;
  vector<float> ri_fill, rj_fill, beamE_fill, w_fill, E1t, E2t;

  float runNum_fill_BCAL, evtNum_fill_BCAL, E1_fill_BCAL, E2_fill_BCAL, E1s_fill_BCAL, E2s_fill_BCAL;
  vector<float> ri_fill_BCAL, rj_fill_BCAL, beamE_fill_BCAL, w_fill_BCAL, E1t_BCAL, E2t_BCAL;

  float runNum_fill_TOF, evtNum_fill_TOF, E1_fill_TOF, E2_fill_TOF, E1s_fill_TOF, E2s_fill_TOF;
  vector<float> ri_fill_TOF, rj_fill_TOF, beamE_fill_TOF, w_fill_TOF, E1t_TOF, E2t_TOF;
  
};

#endif // _JEventProcessor_thetaCrossTree_

