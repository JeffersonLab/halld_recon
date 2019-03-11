// $Id$
//
//    File: JEventProcessor_CCAL_online.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_CCAL_online_
#define _JEventProcessor_CCAL_online_

#include <JANA/JEventProcessor.h>

class TH1D;
class TH1I;
class TH2I;
class TH2F;
class TH1F;
class TProfile;
class TProfile2D;

class JEventProcessor_CCAL_online:public jana::JEventProcessor{
 public:
  JEventProcessor_CCAL_online();
  ~JEventProcessor_CCAL_online();
  const char* className(void){return "JEventProcessor_CCAL_online";}

 private:

  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  double m_targetZ;
  
  const double pi = 3.1415926535;
  const double me = 0.510998928e-3;
  const double ZFCAL = 624.906;
  const double ZCCAL = 1279.77;
  
  const double CRYS_SIZE_X  = 2.077;
  const double CRYS_SIZE_Y  = 2.075;
  const double GLASS_SIZE   = 4.0157;

  TH1D *ccal_num_events;

  TH1I *hdigN;
  TH2F *hdigOcc2D;
  TH1I *hdigInt;
  TH1I *hdigPeak;
  TH1I *hdigT;
  TH1I *hdigPed;
  TProfile *hdigPedChan;
  TH2F *hdigPed2D;
  TH2F *hdigPedSq2D;
  TH2I *hdigIntVsPeak;
  TH1I *hdigQF;
  
  TH1I *hhitN;
  TH1I *hhitE;
  TH1I *hhitETot;
  TH1I *hhitiop;
  TH1I *hhitT;
  TH2F *hhitE2D;
  TH2F *hhitOcc2D;
  
  TH1I *hclusN;
  TH1I *hclusE;
  TH1I *hclusETot;
  TH1I *hclusT;
  TH1I *hclusDime;
  TH2I *hclusXYHigh;
  TH2I *hclusXYLow;
  TH1I *hclusPhi;
  TH1I *hclus2GMass;
  TH1I *hclus2GMass_fcal;
  TH2I *hclusOccEmax;
  
  TH1F *hcomp_bfdt;
  TH1F *hcomp_fcdt;
  TH1F *hcomp_bcdt_full;
  
  TH1F *hcomp_cratio;
  TH1F *hcomp_cfbratio;
  TH2F *hcomp_cfb2d;
  TH1F *hcomp_pfpc;
  TH2F *hcomp_cxy;
  TH2F *hcomp_fxy;
  TH1F *hcomp_bcdt;
  
  TH1F *hcomp_cratio_bkgd;
  TH1F *hcomp_cfbratio_bkgd;
  TH2F *hcomp_cfb2d_bkgd;
  TH1F *hcomp_pfpc_bkgd;
  TH2F *hcomp_cxy_bkgd;
  TH2F *hcomp_fxy_bkgd;
  TH1F *hcomp_bcdt_bkgd;
  
  TH1I *hNPhotons;

  TH2F *hfcalOcc;

};

#endif // _JEventProcessor_CCAL_online_

