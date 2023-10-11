// $Id$

#include <JANA/Compatibility/JGetObjectsFactory.h>

#include "DTOFGeometry_factory.h"
#include "DTOFHit_factory.h"
#include "DTOFPaddleHit_factory.h"
#include "DTOFPoint_factory.h"

#include "DTOFDigiHit.h"
#include "DTOFTDCDigiHit.h"
#include "DTOFHitMC.h"
#include "DTOFTruth.h"

void TOF_init(JFactorySet *factorySet)
{
  /// Create and register TOF data factories
  factorySet->Add(new DTOFGeometry_factory());
  factorySet->Add(new JGetObjectsFactory<DTOFDigiHit>());
  factorySet->Add(new JGetObjectsFactory<DTOFTDCDigiHit>());
  factorySet->Add(new DTOFHit_factory());            // smeared MC data
  factorySet->Add(new JGetObjectsFactory<DTOFHit>("TRUTH"));   // unsmeared MC data
  factorySet->Add(new DTOFPaddleHit_factory());
  factorySet->Add(new DTOFPoint_factory());

  factorySet->Add(new JGetObjectsFactory<DTOFTruth>());
  factorySet->Add(new JGetObjectsFactory<DTOFHitMC>());        // associated MC data objects
  factorySet->Add(new JGetObjectsFactory<DTOFHitMC>("TRUTH")); // associated MC data objects
}
