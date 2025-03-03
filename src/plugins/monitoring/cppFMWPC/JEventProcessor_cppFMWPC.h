// $Id$
//
//    File: JEventProcessor_cppFMWPC.h
// Created: Sat Mar 13 08:01:05 EST 2021
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_cppFMWPC_
#define _JEventProcessor_cppFMWPC_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h>

#include <FMWPC/DFMWPCHit.h>
#include <FDC/DFDCHit.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

class JEventProcessor_cppFMWPC:public JEventProcessor{
 public:
  JEventProcessor_cppFMWPC();
  ~JEventProcessor_cppFMWPC();

  int nFMWPCchambers;
  TH2D *FMWPCwiresT[8];
  TH2D *FMWPCwiresQ[8];
  
  TH2D *FDCwiresT[24];

  // include scatter plots for between chambers

  TH2D *h2_V1_vs_H2;
  TH2D *h2_V3_vs_H4;
  TH2D *h2_V5_vs_H4;
  TH2D *h2_V6_vs_H4;
  TH2D *h2_V3_vs_V1;
  TH2D *h2_H2_vs_H4;
  TH2D *h2_V3_vs_V5;
  TH2D *h2_V5_vs_V6;

 private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_cppFMWPC_

