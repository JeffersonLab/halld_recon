// $Id$
//
//    File: JEventProcessor_cppFMWPC_ana.h
// Created: Sat Mar 13 08:01:05 EST 2021
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_cppFMWPC_ana_
#define _JEventProcessor_cppFMWPC_ana_

#include <JANA/JEventProcessor.h>

#include <FMWPC/DFMWPCHit.h>
#include <FDC/DFDCHit.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

class JEventProcessor_cppFMWPC_ana:public JEventProcessor{
 public:
  JEventProcessor_cppFMWPC_ana();
  ~JEventProcessor_cppFMWPC_ana();

  int nFMWPCchambers;
  TH2D *FMWPCwiresT[8];
  TH2D *FMWPCwiresQ[8];

  TH2D *h2_pmuon_vs_mult[8];
  
  TH2D *FDCwiresT[24];

  // include scatter plots for between chambers

  TH2D *h2_V1_vs_H2;
  TH2D *h2_V3_vs_H4;
  TH2D *h2_V5_vs_H6;
  TH2D *h2_V1_vs_V3;
  TH2D *h2_H2_vs_H4;
  TH2D *h2_V3_vs_V5;
  TH2D *h2_H4_vs_H6;

  TH2D *h2_pmuon_vs_MWPC;

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _JEventProcessor_cppFMWPC_ana_

