// $Id$
//
//    File: FMWPC_init.cc
// Created: Tue Jun 16 07:04:58 EDT 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

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

#include <JANA/Compatibility/JGetObjectsFactory.h>

void FMWPC_init(JFactorySet *factorySet) {
  /// Create and register FMWPC data factories
  //factorySet->Add(new JGetObjectsFactory<DFMWPCHit>());
  factorySet->Add(new JGetObjectsFactory<DFMWPCDigiHit>());
  factorySet->Add(new JGetObjectsFactory<DFMWPCTruthHit>());
  factorySet->Add(new JGetObjectsFactory<DFMWPCTruth>());
  factorySet->Add(new DFMWPCHit_factory());
  factorySet->Add(new DFMWPCCluster_factory());
  factorySet->Add(new DFMWPCMatchedTrack_factory());

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

