#include "DESSkimData.h"

#include <JANA/JFactorySet.h>
#include <JANA/Compatibility/JGetObjectsFactory.h>

void EVENTSTORE_init(JFactorySet *factorySet)
{
    factorySet->Add(new JGetObjectsFactory<DESSkimData>());
}


