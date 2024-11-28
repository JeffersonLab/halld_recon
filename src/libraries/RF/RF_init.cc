// $Id: RF_init.cc 14984 2015-03-31 14:33:22Z pmatt $

#include "DRFTime_factory.h"
#include "DRFTime_factory_FDC.h"
#include "DRFTime_factory_PSC.h"
#include "DRFTime_factory_TAGH.h"
#include "DRFTime_factory_TOF.h"

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

void RF_init(JFactorySet *factorySet)
{
	/// Create and register RF data factories
	factorySet->Add(new JFactoryT<DRFDigiTime>());
	factorySet->Add(new JFactoryT<DRFTDCDigiTime>());
	factorySet->Add(new DRFTime_factory());
	factorySet->Add(new DRFTime_factory_FDC());
	factorySet->Add(new DRFTime_factory_PSC());
	factorySet->Add(new DRFTime_factory_TAGH());
	factorySet->Add(new DRFTime_factory_TOF());
	factorySet->Add(new JFactoryT<DRFTime>("TRUTH"));
}
