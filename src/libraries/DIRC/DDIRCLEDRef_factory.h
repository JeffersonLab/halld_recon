// $Id$
//
//    File: DDIRCLEDRef_factory.h
//

#ifndef _DDIRCLEDRef_factory_
#define _DDIRCLEDRef_factory_

#include <vector>
#include <map>
#include <utility>
using namespace std;

#include <JANA/JFactory.h>
#include "DDIRCLEDRef.h"
#include "TTAB/DTranslationTable.h"
#include "TTAB/DTTabUtilities.h"
#include <DAQ/Df250PulseData.h>
#include <DAQ/DCAEN1290TDCHit.h>
using namespace jana;


class DDIRCLEDRef_factory:public jana::JFactory<DDIRCLEDRef>{
 public:
  DDIRCLEDRef_factory(){};
  ~DDIRCLEDRef_factory(){};
  
  // overall scale factors
  double a_scale;
  double t_scale;
  double t_base,t_base_tdc;
  double tdc_adc_time_offset;

  double tdc_time_offset;
  double adc_time_offset;

  DDIRCLEDRef* FindMatch(double T);
  
 private:
  jerror_t init(void);
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
  jerror_t erun(void);
  jerror_t fini(void);
  
  double CalcWalkCorrIntegral(DDIRCLEDRef* hit);
  double CalcWalkCorrAmplitude(DDIRCLEDRef* hit);
  double CalcWalkCorrNEW(DDIRCLEDRef* hit);
  double CalcWalkCorrNEWAMP(DDIRCLEDRef* hit);

  bool CHECK_FADC_ERRORS;
};

#endif // _DDIRCLEDRef_factory_

