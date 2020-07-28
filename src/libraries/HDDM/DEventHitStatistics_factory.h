// $Id$
//
//    File: DEventHitStatistics_factory.h
// Created: Wed Jul 8 17:49:15 EDT 2020
// Creator: jonesrt (on Linux gluey.phys.uconn.edu)
//

#ifndef _DEventHitStatistics_factory_
#define _DEventHitStatistics_factory_

#include <vector>
using namespace std;

#include <JANA/JFactory.h>

#include "DEventHitStatistics.h"

class DEventHitStatistics_factory: public jana::JFactory<DEventHitStatistics> {
   public:
      DEventHitStatistics_factory() {};
      ~DEventHitStatistics_factory() {};

   private:
      jerror_t init(void);                                          ///< Called once at program start
      jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);    ///< Called everytime a new run number is detected
      jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);  ///< Called every event
      jerror_t erun(void);                                          ///< Called everytime run number changes, if brun has been called
      jerror_t fini(void);                                          ///< Called after last event of last event source has been processed

      void Reset_Data(void);
};

#endif // _DEventHitStatistics_factory_
