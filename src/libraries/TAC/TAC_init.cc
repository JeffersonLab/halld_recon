/*
 * TAC_init.cc
 *
 *  Created on: Mar 28, 2017
 *      Author: Hovanes Egiyan
 */

#include "DTACDigiHit.h"
#include "DTACTDCDigiHit.h"
#include "DTACHit_factory.h"
#include "DRebuildFromRawFADC_factory.h"
#include "HitRebuilderByFit.h"
#include "WaveformSpikeFunctor.h"
#include "WaveformErfcFunctor.h"

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

void TAC_init(JFactorySet *factorySet) {
	/// Create and register TAC data factories
	factorySet->Add(new JFactoryT<DTACDigiHit>());
	factorySet->Add(new JFactoryT<DTACTDCDigiHit>());
	factorySet->Add(new DTACHit_factory());
	factorySet->Add(
			new DRebuildFromRawFADC_factory<DTACHit_factory,
					HitRebuilderByFit<WaveformSpikeFunctor>>());
	factorySet->Add(
			new DRebuildFromRawFADC_factory<DTACHit_factory,
					HitRebuilderByFit<WaveformErfcFunctor>>());

	factorySet->Add(new DRebuildFromRawFADC_factory<>());
	factorySet->Add(new JFactoryT<DTACHit>("TRUTH"));

}

