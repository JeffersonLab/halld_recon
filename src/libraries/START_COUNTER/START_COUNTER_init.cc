// $Id: CDC_init.cc 2151 2006-10-23 18:01:37Z davidl $

#include <JANA/JFactorySet.h>
#include <JANA/Compatibility/JGetObjectsFactory.h>

#include "DSCHit_factory.h"
#include "DSCTDCDigiHit.h"
#include "DSCTruthHit.h"

void START_COUNTER_init(JFactorySet *factorySet)
{
	/// Create and register Start Counter data factories
	factorySet->Add(new JGetObjectsFactory<DSCDigiHit>());
	factorySet->Add(new JGetObjectsFactory<DSCTDCDigiHit>());
	factorySet->Add(new DSCHit_factory());
	factorySet->Add(new JGetObjectsFactory<DSCHit>("TRUTH"));
	factorySet->Add(new JGetObjectsFactory<DSCTruthHit>());
}
