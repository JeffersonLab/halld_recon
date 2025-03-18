// $Id: TTAB_init.cc $

#include <JANA/JFactorySet.h>

#include "DTranslationTable_factory.h"
#include "DTTabUtilities_factory.h"

void TTAB_init(JFactorySet *factorySet)
{
  /// Create and register DTranslationTable factory
  factorySet->Add(new DTranslationTable_factory());
  factorySet->Add(new DTTabUtilities_factory());
}
