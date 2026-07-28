// $Id$
//
//    File: JEventProcessor_FCAL2_invmass.h
// Created: Thu Apr  9 02:26:57 PM EDT 2026
// Creator: staylor (on Linux ifarm2402.jlab.org 5.14.0-611.38.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_FCAL2_invmass_
#define _JEventProcessor_FCAL2_invmass_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services
#include <TH1F.h>
#include <TH2F.h>
#include <DVector3.h>

class JEventProcessor_FCAL2_invmass:public JEventProcessor{
public:
  JEventProcessor_FCAL2_invmass();
  ~JEventProcessor_FCAL2_invmass();
  const char* className(void){return "JEventProcessor_FCAL2_invmass";}
  
private:
  void Init() override;                       ///< Called once at program start.
  void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
  void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
  void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
  void Finish() override;                     ///< Called after last event of last event source has been processed.
  
  std::shared_ptr<JLockService> lockService;

  TH1F *h_2gamma_ECAL_ECAL,*h_2gamma_ECAL_FCAL,*h_2gamma_FCAL_FCAL;
  TH2F *h_2gamma_ECAL_ECAL_vs_E,*h_2gamma_ECAL_FCAL_vs_E,*h_2gamma_FCAL_FCAL_vs_E;
  TH2F *h_Epred_ECAL_ECAL_vs_E,*h_2gamma_ECAL_ECAL_vs_ch;
  TH2F *h_Epred_FCAL_FCAL_vs_E,*h_2gamma_FCAL_FCAL_vs_ch;
  TH1F *h_2gamma_BCAL_BCAL,*h_2gamma_BCAL_FCAL,*h_2gamma_BCAL_ECAL;
  
  DVector3 m_FCALCenter;
  double SHOWER_THRESHOLD,ECAL_POS_CUT,DELTA_E_CUT;
};

#endif // _JEventProcessor_FCAL2_invmass_

