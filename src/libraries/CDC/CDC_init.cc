// $Id$

#include "DCDCDigiHit.h"
#include "DCDCHit.h"
#include "DCDCHit_factory.h"
#include "DCDCHit_factory_Calib.h"
#include "DCDCTrackHit.h"
#include "DCDCTrackHit_factory.h"

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

void CDC_init(JFactorySet *factorySet)
{
  /// Create and register CDC data factories
  factorySet->Add(new JFactoryT<DCDCDigiHit>());
  factorySet->Add(new DCDCHit_factory());
  factorySet->Add(new DCDCHit_factory_Calib());
  factorySet->Add(new JFactoryT<DCDCHit>("TRUTH"));
  factorySet->Add(new DCDCTrackHit_factory());
}
