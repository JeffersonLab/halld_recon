// $Id$


#include "DPSDigiHit.h"
#include "DPSHit_factory.h"
#include "DPSCDigiHit.h"
#include "DPSCHit_factory.h"
#include "DPSCTDCDigiHit.h"
#include "DPSCTruthHit.h"
#include "DPSTruthHit.h"
#include "DPSGeometry_factory.h"
#include "DPSCPair_factory.h"
#include "DPSPair_factory.h"
#include "DLumi_factory.h"

#include <JANA/JFactorySet.h>


void PAIR_SPECTROMETER_init(JFactorySet* factorySet)
{
  /// Create and register Pair Spectrometer data factories
  factorySet->Add(new DPSGeometry_factory());
  factorySet->Add(new JFactoryT<DPSDigiHit>());
  factorySet->Add(new DPSHit_factory());
  factorySet->Add(new JFactoryT<DPSCDigiHit>());
  factorySet->Add(new JFactoryT<DPSCTDCDigiHit>());
  factorySet->Add(new DPSCHit_factory());
  factorySet->Add(new JFactoryT<DPSCHit>("TRUTH"));
  factorySet->Add(new JFactoryT<DPSCTruthHit>());
  factorySet->Add(new JFactoryT<DPSTruthHit>());
  factorySet->Add(new DPSCPair_factory());
  factorySet->Add(new DPSPair_factory());
  factorySet->Add(new DLumi_factory());
}
