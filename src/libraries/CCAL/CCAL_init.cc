// $Id: FCAL_init.cc 5342 2009-07-09 15:46:12Z davidl $

#include <CCAL/DCCALTruthShower_factory.h>
#include <CCAL/DCCALGeometry_factory.h>

#include "DCCALDigiHit.h"
#include "DCCALHit_factory.h"
#include "DCCALShower_factory.h"

#include "DCCALRefDigiHit.h"

#include <JANA/JFactorySet.h>
#include <JANA/Compatibility/JGetObjectsFactory.h>

void CCAL_init(JFactorySet *factorySet)
{
	/// Create and register CCAL data factories
	factorySet->Add(new JGetObjectsFactory<DCCALDigiHit>());
	factorySet->Add(new JGetObjectsFactory<DCCALRefDigiHit>());
	factorySet->Add(new DCCALHit_factory());
	factorySet->Add(new DCCALShower_factory());
	factorySet->Add(new JGetObjectsFactory<DCCALHit>("TRUTH"));
	factorySet->Add(new DCCALTruthShower_factory());
	factorySet->Add(new DCCALGeometry_factory());
}
