// $Id: FCAL_init.cc 5342 2009-07-09 15:46:12Z davidl $
#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

#include "DECALTruthShower_factory.h"
#include "DECALDigiHit.h"
#include "DECALHit_factory.h"
#include "DECALRefDigiHit.h"
#include "DECALCluster_factory.h"
#include "DECALShower_factory.h"
#include "DECALGeometry_factory.h"

void ECAL_init(JFactorySet *factorySet)
{
  /// Create and register ECAL data factories
  factorySet->Add(new JFactoryT<DECALDigiHit>());
  factorySet->Add(new JFactoryT<DECALRefDigiHit>());
  factorySet->Add(new DECALHit_factory());
  factorySet->Add(new JFactoryT<DECALHit>("TRUTH"));
  factorySet->Add(new DECALTruthShower_factory());
  factorySet->Add(new DECALCluster_factory());
  factorySet->Add(new DECALShower_factory());
  factorySet->Add(new DECALGeometry_factory());
}
