// $Id: CDC_init.cc 2151 2006-10-23 18:01:37Z davidl $

#include <JANA/Compatibility/JGetObjectsFactory.h>

#include "DTPOLTruthHit.h"
#include "DTPOLSectorDigiHit.h"
#include "DTPOLRingDigiHit.h"
#include "DTPOLHit_factory.h"

void TPOL_init(JFactorySet *factorySet)
{
	/// Create and register TPOL data factories
	factorySet->Add(new JGetObjectsFactory<DTPOLSectorDigiHit>());
	factorySet->Add(new JGetObjectsFactory<DTPOLRingDigiHit>());
	factorySet->Add(new DTPOLHit_factory());
	factorySet->Add(new JGetObjectsFactory<DTPOLHit>("TRUTH"));
	factorySet->Add(new JGetObjectsFactory<DTPOLTruthHit>());
}
