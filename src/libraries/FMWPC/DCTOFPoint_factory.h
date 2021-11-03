// $Id$
//
//    File: DCTOFPoint_factory.h
// Created: Thu Oct 28 07:48:04 EDT 2021
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DCTOFPoint_factory_
#define _DCTOFPoint_factory_

#include <JANA/JFactory.h>
#include "DCTOFPoint.h"

class DCTOFPoint_factory:public jana::JFactory<DCTOFPoint>{
 public:
  DCTOFPoint_factory(){};
  ~DCTOFPoint_factory(){};
  
  
 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  double LIGHT_PROPAGATION_SPEED,THRESHOLD,ATTENUATION_LENGTH;
};

#endif // _DCTOFPoint_factory_

