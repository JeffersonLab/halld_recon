// $Id$
//
//    File: DGEMTRDSegment_factory.h
// Created: Wed Feb  9 13:19:03 EST 2022
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DGEMTRDSegment_factory_
#define _DGEMTRDSegment_factory_

#include <JANA/JFactory.h>
#include "DGEMTRDSegment.h"
#include "DGEMTRDHit.h"

class DGEMTRDSegment_factory:public jana::JFactory<DGEMTRDSegment>{
 public:
  DGEMTRDSegment_factory(){};
  ~DGEMTRDSegment_factory(){};
   
 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  

  double R_CUT=5.;
};

#endif // _DGEMTRDSegment_factory_

