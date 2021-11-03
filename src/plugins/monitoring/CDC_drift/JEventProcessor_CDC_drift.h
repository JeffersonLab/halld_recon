// $Id$
//
//    File: JEventProcessor_CDC_drift.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_CDC_drift_
#define _JEventProcessor_CDC_drift_

#include <JANA/JEventProcessor.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TTree.h>


class JEventProcessor_CDC_drift:public jana::JEventProcessor{
 public:
  JEventProcessor_CDC_drift();
  ~JEventProcessor_CDC_drift();
  const char* className(void){return "JEventProcessor_CDC_drift";}


 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.


  // histograms

  TH1F *cdc_time = NULL;
  TH1I *cdc_rawtime = NULL;

  TTree *tfit = NULL;
  TTree *rtfit = NULL;

  bool FIT;
  bool VERBOSE;
  
};

#endif // _JEventProcessor_CDC_drift_

