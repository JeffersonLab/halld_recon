// $Id: CDC_init.cc 2151 2006-10-23 18:01:37Z davidl $

#include "DSCHit_factory.h"
#include "DSCTDCDigiHit.h"
#include "DSCTruthHit.h"

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

void START_COUNTER_init(JFactorySet *factorySet)
{
	/// Create and register Start Counter data factories
	factorySet->Add(new JFactoryT<DSCDigiHit>());
	factorySet->Add(new JFactoryT<DSCTDCDigiHit>());
	factorySet->Add(new DSCHit_factory());
	factorySet->Add(new JFactoryT<DSCHit>("TRUTH"));
	factorySet->Add(new JFactoryT<DSCTruthHit>());
}
