// $Id$
//
//    File: DCTOFHit_factory.h
// Created: Fri Mar 18 09:00:12 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DCTOFHit_factory_
#define _DCTOFHit_factory_

#include <JANA/JFactory.h>
#include "DCTOFHit.h"

class DCTOFHit_factory:public jana::JFactory<DCTOFHit>{
 public:
  DCTOFHit_factory(){};
  ~DCTOFHit_factory(){};
  
 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  double DELTA_T_ADC_TDC_MAX;
  double t_base_adc,t_base_tdc,t_scale;
  vector<double>adc_time_offsets,tdc_time_offsets,adc2E;
};

#endif // _DCTOFHit_factory_

