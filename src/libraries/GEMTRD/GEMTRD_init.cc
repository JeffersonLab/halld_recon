// $Id$
//
//    File: GEMTRD_init.cc
// Created: Wed Feb 16, 2022
// Creator: staylor
//

#include <JANA/JEventLoop.h>
#include <JANA/JFactory.h>
using namespace jana;

#include "DGEMTRDHit_factory.h"
#include "DGEMTRDSegment_factory.h"
#include "DGEMTRDTruthHit.h"
#include "DGEMTRDTruth.h"

jerror_t GEMTRD_init(JEventLoop *loop) {

  /// Create and register GEMTRD data factories
  loop->AddFactory(new JFactory<DGEMTRDTruthHit>());
  loop->AddFactory(new JFactory<DGEMTRDTruth>());
  loop->AddFactory(new DGEMTRDHit_factory());
  loop->AddFactory(new DGEMTRDSegment_factory());

  return NOERROR;
}

