// $Id$
//
//    File: DCTOFPoint_factory.h
// Created: Thu Oct 28 07:48:04 EDT 2021
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DCTOFPoint_factory_
#define _DCTOFPoint_factory_

#include <JANA/JFactoryT.h>
#include "DCTOFPoint.h"
#include <DVector3.h>

class DCTOFPoint_factory:public JFactoryT<DCTOFPoint>{
 public:
  DCTOFPoint_factory(){};
  ~DCTOFPoint_factory(){};
  
  
 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  vector<DVector3>ctof_positions;

  double LIGHT_PROPAGATION_SPEED,THRESHOLD,ATTENUATION_LENGTH;
};

#endif // _DCTOFPoint_factory_

