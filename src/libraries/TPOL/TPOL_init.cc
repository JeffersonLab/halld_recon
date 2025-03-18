// $Id: CDC_init.cc 2151 2006-10-23 18:01:37Z davidl $

#include "DTPOLTruthHit.h"
#include "DTPOLSectorDigiHit.h"
#include "DTPOLRingDigiHit.h"
#include "DTPOLHit_factory.h"

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

void TPOL_init(JFactorySet *factorySet)
{
	/// Create and register TPOL data factories
	factorySet->Add(new JFactoryT<DTPOLSectorDigiHit>());
	factorySet->Add(new JFactoryT<DTPOLRingDigiHit>());
	factorySet->Add(new DTPOLHit_factory());
	factorySet->Add(new JFactoryT<DTPOLHit>("TRUTH"));
	factorySet->Add(new JFactoryT<DTPOLTruthHit>());
}
