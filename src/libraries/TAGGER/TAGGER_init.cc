// $Id$

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

#include "DTAGMDigiHit.h"
#include "DTAGMTDCDigiHit.h"
#include "DTAGHDigiHit.h"
#include "DTAGHTDCDigiHit.h"
#include "DTAGMHit.h"
#include "DTAGHHit.h"
#include "DTAGMGeometry.h"
#include "DTAGHGeometry.h"
#include "DTAGMGeometry_factory.h"
#include "DTAGHGeometry_factory.h"
#include "DTAGMHit_factory.h"
#include "DTAGMHit_factory_Calib.h"
#include "DTAGHHit_factory.h"
#include "DTAGHHit_factory_Calib.h"


void TAGGER_init(JFactorySet *factorySet)
{
  /// Create and register TAGGER data factories
  factorySet->Add(new JFactoryT<DTAGMDigiHit>());
  factorySet->Add(new JFactoryT<DTAGMTDCDigiHit>());
  factorySet->Add(new JFactoryT<DTAGHDigiHit>());
  factorySet->Add(new JFactoryT<DTAGHTDCDigiHit>());
  factorySet->Add(new DTAGMHit_factory());
  factorySet->Add(new DTAGMHit_factory_Calib());
  factorySet->Add(new DTAGHHit_factory());
  factorySet->Add(new DTAGHHit_factory_Calib());
  factorySet->Add(new JFactoryT<DTAGMHit>("TRUTH"));
  factorySet->Add(new JFactoryT<DTAGHHit>("TRUTH"));
  factorySet->Add(new DTAGMGeometry_factory());
  factorySet->Add(new DTAGHGeometry_factory());
}
