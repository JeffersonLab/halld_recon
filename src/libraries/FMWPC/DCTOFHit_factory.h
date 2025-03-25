// $Id$
//
//    File: DCTOFHit_factory.h
// Created: Fri Mar 18 09:00:12 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DCTOFHit_factory_
#define _DCTOFHit_factory_

#include <JANA/JFactoryT.h>
#include "DCTOFHit.h"

class DCTOFHit_factory:public JFactoryT<DCTOFHit>{
 public:
  DCTOFHit_factory(){};
  ~DCTOFHit_factory(){};
  
 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  bool INSTALLED;

  double DELTA_T_ADC_TDC_MAX;
  double t_base_adc,t_base_tdc,t_scale;
  vector<double>adc_time_offsets,tdc_time_offsets,adc2E;
};

#endif // _DCTOFHit_factory_

