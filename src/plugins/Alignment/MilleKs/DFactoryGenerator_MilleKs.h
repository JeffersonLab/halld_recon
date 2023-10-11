// $Id$
//
//    File: DFactoryGenerator_MilleKs.h
// Created: Mon Feb 22 18:20:38 EST 2021
// Creator: keigo (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64
// x86_64)
//

#ifndef _DFactoryGenerator_MilleKs_
#define _DFactoryGenerator_MilleKs_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_MilleKs.h"

class DFactoryGenerator_MilleKs : public JFactoryGenerator {
 public:
  virtual const char* className(void) { return static_className(); }
  static const char* static_className(void) {
    return "DFactoryGenerator_MilleKs";
  }

  void GenerateFactories(JFactorySet* fs) {
    fs->Add(new DReaction_factory_MilleKs());
  }
};

#endif  // _DFactoryGenerator_MilleKs_
