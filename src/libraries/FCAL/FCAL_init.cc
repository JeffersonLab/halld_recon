// $Id$

#include "DFCALCluster_factory.h"
#include "DFCALCluster_factory_Island.h"
#include "DFCALGeometry_factory.h"
#include "DFCALShower_factory.h"
#include "DFCALTruthShower.h"
#include "DFCALDigiHit.h"
#include "DFCALHit_factory.h"

#include <JANA/Compatibility/JGetObjectsFactory.h>

void FCAL_init(JFactorySet* factorySet)
{
	/// Create and register FCAL data factories
	factorySet->Add(new JGetObjectsFactory<DFCALDigiHit>());
	factorySet->Add(new DFCALHit_factory());
	factorySet->Add(new JGetObjectsFactory<DFCALHit>("TRUTH"));
	factorySet->Add(new DFCALCluster_factory());
	factorySet->Add(new DFCALCluster_factory_Island());
	factorySet->Add(new DFCALShower_factory());
	factorySet->Add(new DFCALGeometry_factory());
	factorySet->Add(new JGetObjectsFactory<DFCALTruthShower>());
}
