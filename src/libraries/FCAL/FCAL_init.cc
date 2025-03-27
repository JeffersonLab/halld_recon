// $Id$

#include "DFCALCluster_factory.h"
#include "DFCALGeometry_factory.h"
#include "DFCALShower_factory.h"
#include "DFCALTruthShower.h"
#include "DFCALDigiHit.h"
#include "DFCALHit_factory.h"

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

void FCAL_init(JFactorySet* factorySet)
{
	/// Create and register FCAL data factories
	factorySet->Add(new JFactoryT<DFCALDigiHit>());
	factorySet->Add(new DFCALHit_factory());
	factorySet->Add(new JFactoryT<DFCALHit>("TRUTH"));
	factorySet->Add(new DFCALCluster_factory());
	factorySet->Add(new DFCALShower_factory());
	factorySet->Add(new DFCALGeometry_factory());
	factorySet->Add(new JFactoryT<DFCALTruthShower>());
}
