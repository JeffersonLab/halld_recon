// $Id$
//
//    File: DEventProcessor_MilleKs.h
// Created: Mon Feb 22 18:20:38 EST 2021
// Creator: keigo (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DEventProcessor_MilleKs_
#define _DEventProcessor_MilleKs_

#include <ANALYSIS/DEventWriterROOT.h>
#include <ANALYSIS/DHistogramActions.h>
#include <HDDM/DEventWriterREST.h>
#include <JANA/JApplication.h>
#include <JANA/JEventProcessor.h>

#include "DFactoryGenerator_MilleKs.h"
#include "Mille.h"

using namespace jana;
using namespace std;

class DEventProcessor_MilleKs : public jana::JEventProcessor {
 public:
  const char* className(void) { return "DEventProcessor_MilleKs"; }

 private:
  jerror_t init(void);  ///< Called once at program start.
  jerror_t brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber);  ///< Called every time a new run number is detected.
  jerror_t evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber);  ///< Called every event.
  jerror_t erun(void);  ///< Called every time run number changes, provided brun has been called.
  jerror_t fini(void);  ///< Called after last event of last event source has been processed.

  Mille *milleWriter;

  // For non-custom reaction-independent histograms, it is recommended that you
  // simultaneously run the monitoring_hists plugin instead of defining them here
  const DMagneticFieldMap *bfield;

  TTree *tree_;
  double m2_;
  double m2_measured_;
  double m_;
  double m_measured_;
  double conf_lv_;
};

#endif  // _DEventProcessor_MilleKs_
