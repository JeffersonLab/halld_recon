#include <JANA/JFactorySet.h>

#include "DEventWriterREST_factory.h"
#include "DEventWriterHDDM_factory.h"
#include "DEventHitStatistics_factory.h"

void HDDM_init(JFactorySet* factorySet)
{
	/// Create and register HDDM data factories
	factorySet->Add(new DEventWriterREST_factory());
	factorySet->Add(new DEventWriterHDDM_factory());
	factorySet->Add(new DEventHitStatistics_factory());
}


