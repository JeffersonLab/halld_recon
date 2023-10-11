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

#include <JANA/Compatibility/JGetObjectsFactory.h>


void PAIR_SPECTROMETER_init(JFactorySet* factorySet)
{
  /// Create and register Pair Spectrometer data factories
  factorySet->Add(new DPSGeometry_factory());
  factorySet->Add(new JGetObjectsFactory<DPSDigiHit>());
  factorySet->Add(new DPSHit_factory());
  factorySet->Add(new JGetObjectsFactory<DPSCDigiHit>());
  factorySet->Add(new JGetObjectsFactory<DPSCTDCDigiHit>());
  factorySet->Add(new DPSCHit_factory());
  factorySet->Add(new JGetObjectsFactory<DPSCHit>("TRUTH"));
  factorySet->Add(new JGetObjectsFactory<DPSCTruthHit>());
  factorySet->Add(new JGetObjectsFactory<DPSTruthHit>());
  factorySet->Add(new DPSCPair_factory());
  factorySet->Add(new DPSPair_factory());
  factorySet->Add(new DLumi_factory());
}
