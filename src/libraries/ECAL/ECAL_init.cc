// $Id: FCAL_init.cc 5342 2009-07-09 15:46:12Z davidl $
#include <JANA/JFactorySet.h>

#include <ECAL/DECALTruthShower_factory.h>

#include "DECALDigiHit.h"
#include "DECALHit_factory.h"


#include "DECALRefDigiHit.h"

void ECAL_init(JFactorySet *factorySet)
{
	/// Create and register ECAL data factories
  	factorySet->Add(new JFactoryT<DECALDigiHit>());
  	factorySet->Add(new JFactoryT<DECALRefDigiHit>());
	factorySet->Add(new DECALHit_factory());
	factorySet->Add(new JFactoryT<DECALHit>("TRUTH"));
	factorySet->Add(new DECALTruthShower_factory());
}
