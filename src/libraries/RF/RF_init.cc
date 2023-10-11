// $Id: RF_init.cc 14984 2015-03-31 14:33:22Z pmatt $

#include <JANA/Compatibility/JGetObjectsFactory.h>

#include "DRFTime_factory.h"
#include "DRFTime_factory_FDC.h"
#include "DRFTime_factory_PSC.h"
#include "DRFTime_factory_TAGH.h"
#include "DRFTime_factory_TOF.h"

void RF_init(JFactorySet *factorySet)
{
	/// Create and register RF data factories
	factorySet->Add(new JGetObjectsFactory<DRFDigiTime>());
	factorySet->Add(new JGetObjectsFactory<DRFTDCDigiTime>());
	factorySet->Add(new DRFTime_factory());
	factorySet->Add(new DRFTime_factory_FDC());
	factorySet->Add(new DRFTime_factory_PSC());
	factorySet->Add(new DRFTime_factory_TAGH());
	factorySet->Add(new DRFTime_factory_TOF());
	factorySet->Add(new JGetObjectsFactory<DRFTime>("TRUTH"));

}
