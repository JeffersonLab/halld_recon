// $Id$
//
//    File: FMWPC_init.cc
// Created: Tue Jun 16 07:04:58 EDT 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#include <JANA/JEventLoop.h>
#include <JANA/JFactory.h>
using namespace jana;

#include "DFMWPCHit.h"
#include "DFMWPCHit_factory.h"
#include "DFMWPCCluster_factory.h"
#include "DFMWPCTruthHit.h"
#include "DFMWPCTruth.h"
#include "DCTOFTruth.h"
#include "DCTOFHit_factory.h"
#include "DCTOFDigiHit.h"
#include "DCTOFTDCDigiHit.h"
#include "DCTOFPoint_factory.h"
#include "DFMWPCMatchedTrack_factory.h"
#include "DCPPEpEm_factory.h"

jerror_t FMWPC_init(JEventLoop *loop) {

  /// Create and register FMWPC data factories
  //loop->AddFactory(new JFactory<DFMWPCHit>());
  loop->AddFactory(new JFactory<DFMWPCDigiHit>());
  loop->AddFactory(new JFactory<DFMWPCTruthHit>());
  loop->AddFactory(new JFactory<DFMWPCTruth>());
  loop->AddFactory(new DFMWPCHit_factory());
  loop->AddFactory(new DFMWPCCluster_factory());
  loop->AddFactory(new DFMWPCMatchedTrack_factory());

  /// Factories for downstream scintillators
  loop->AddFactory(new JFactory<DCTOFDigiHit>());
  loop->AddFactory(new JFactory<DCTOFTDCDigiHit>());
  loop->AddFactory(new JFactory<DCTOFTruth>());
  loop->AddFactory(new DCTOFHit_factory());
  loop->AddFactory(new DCTOFPoint_factory());

  // Factory for e+e-/ pi+pi- fitting
  loop->AddFactory(new DCPPEpEm_factory());

  return NOERROR;
}

