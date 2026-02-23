// $Id$
//
//    File: JEventProcessor_npp_skim.h
// Created: Tue Apr 16 10:14:04 EDT 2024
// Creator: gxproj2 (on Linux ifarm1802.jlab.org 3.10.0-1160.102.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_npp_skim_
#define _JEventProcessor_npp_skim_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"

#include <ECAL/DECALShower.h>
#include <FCAL/DFCALShower.h>
#include <BCAL/DBCALShower.h>

#include <vector>
#include <TMath.h>

class JEventProcessor_npp_skim:public JEventProcessor{
 public:
  JEventProcessor_npp_skim();
  ~JEventProcessor_npp_skim();
  const char* className(void){return "JEventProcessor_npp_skim";}
  int num_epics_events;
  
 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  struct Shower {DVector3 pos{0.0,0.0,0.0}; double e = 0;};

  double  targetZ;
  double  ECAL_E_CUT, FCAL_E_CUT, BCAL_E_CUT, EGG_CUT, MGG_CUT, EPI0_CUT, MPI0_CUT, EPI0PI0_CUT;

  double mgg(const Shower &s1, const Shower &s2) {
    DVector3 tgt_pos(0,0,targetZ);
    DVector3 n1 = (s1.pos - tgt_pos).Unit();
    DVector3 n2 = (s2.pos - tgt_pos).Unit();
    double cost = n1.Dot(n2);
    double m2 = s1.e*s2.e*(1.0-cost);

    return (m2>0.0) ? std::sqrt(2*m2) : 0.0;
  }
  
};

#endif // _JEventProcessor_npp_skim_

