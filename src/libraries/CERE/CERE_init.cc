/*
 * CERE_init.cc
 *
 *  Created on: Oct 3, 2012
 *      Author: yqiang
 *
 *  Modified on:
 *  	Oct 9 2012, Yi Qiang, add general Cerenkov hit
 */

#include <JANA/Compatibility/JGetObjectsFactory.h>

#include "DCereHit.h"

void CERE_init(JFactorySet *factorySet) {
	/// Create and register Cherenkov data factories
	factorySet->Add(new JGetObjectsFactory<DCereHit>());
}

