/*
 * TAC_init.cc
 *
 *  Created on: Mar 28, 2017
 *      Author: Hovanes Egiyan
 */

#include <JANA/Compatibility/JGetObjectsFactory.h>

#include "DTACDigiHit.h"
#include "DTACTDCDigiHit.h"
#include "DTACHit_factory.h"
#include "DRebuildFromRawFADC_factory.h"
#include "HitRebuilderByFit.h"
#include "WaveformSpikeFunctor.h"
#include "WaveformErfcFunctor.h"

void TAC_init(JFactorySet *factorySet) {
	/// Create and register TAC data factories
	factorySet->Add(new JGetObjectsFactory<DTACDigiHit>());
	factorySet->Add(new JGetObjectsFactory<DTACTDCDigiHit>());
	factorySet->Add(new DTACHit_factory());
	factorySet->Add(
			new DRebuildFromRawFADC_factory<DTACHit_factory,
					HitRebuilderByFit<WaveformSpikeFunctor>>());
	factorySet->Add(
			new DRebuildFromRawFADC_factory<DTACHit_factory,
					HitRebuilderByFit<WaveformErfcFunctor>>());

	factorySet->Add(new DRebuildFromRawFADC_factory<>());
	factorySet->Add(new JGetObjectsFactory<DTACHit>("TRUTH"));

}

