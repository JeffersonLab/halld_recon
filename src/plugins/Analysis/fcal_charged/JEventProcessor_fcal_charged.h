// $Id$
//
//    File: JEventProcessor_cppFMWPC_ana.h
// Created: Sat Mar 13 08:01:05 EST 2021
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_fcal_charged_
#define _JEventProcessor_fcal_charged_

#include <JANA/JEventProcessor.h>

#include <FMWPC/DFMWPCHit.h>
#include <FDC/DFDCHit.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

class JEventProcessor_fcal_charged:public JEventProcessor{
 public:
  JEventProcessor_fcal_charged();
  ~JEventProcessor_fcal_charged();

  TH1D *h1_events;

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _JEventProcessor_fcal_charged_

