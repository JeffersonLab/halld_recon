#include "DESSkimData.h"

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

void EVENTSTORE_init(JFactorySet *factorySet)
{
    factorySet->Add(new JFactoryT<DESSkimData>());
}


