// $Id$
//
//    File: CGEM_init.cc
// Created: Tue Jun 16 07:04:58 EDT 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

//#include <JANA/JEventLoop.h>
#include <JANA/JFactory.h>
using namespace jana;

#include "DCGEMHit.h"
#include "DCGEMDigiHit.h"
#include "DCGEMHit_factory.h"
#include "DCGEMTruthHit.h"

jerror_t CGEM_init(JEventLoop *loop) {

  /// Create and register CGEM data factories
  loop->AddFactory(new JFactory<DCGEMDigiHit>());
  loop->AddFactory(new DCGEMHit_factory());
  loop->AddFactory(new JFactory<DCGEMHit>());
  loop->AddFactory(new JFactory<DCGEMHit>("TRUTH"));
  loop->AddFactory(new JFactory<DCGEMTruthHit>());
  

  //loop->AddFactory(new JFactory<DCGEMTruthHit>());
  //loop->AddFactory(new JFactory<DCGEMDigiHit>());
  //loop->AddFactory(new DCGEMHit_factory());
  //loop->AddFactory(new JFactory<DCGEMHit>());
  //loop->AddFactory(new JFactory<DCGEMHit>("TRUTH"));

  return NOERROR;
}

