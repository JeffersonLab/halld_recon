// $Id$
//
//    File: JEventProcessor_cppFMWPC_ana.h
// Created: Sat Mar 13 08:01:05 EST 2021
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_cppFMWPC_ana_
#define _JEventProcessor_cppFMWPC_ana_

#include <JANA/JEventProcessor.h>
using namespace jana;

#include <FMWPC/DFMWPCHit.h>
#include <FDC/DFDCHit.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

class JEventProcessor_fcal_charged:public jana::JEventProcessor{
 public:
  JEventProcessor_fcal_charged();
  ~JEventProcessor_fcal_charged();
  const char* className(void){return "JEventProcessor_fcal_charged";}


  TH1D *h1_events;

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_fcal_charged_

