/*
 * TRD_init.cc
 *
 *  Created on: Oct 24, 2019
 *      Author: jrsteven
 */

#include "DTRDDigiHit.h"
#include "DTRDHit_factory.h"
#include "DTRDStripCluster_factory.h"
#include "DTRDStripCluster_factory_RAW.h"
#include "DTRDPoint_factory.h"
#include "DTRDSegment_factory.h"

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryT.h>

void TRD_init(JFactorySet *factorySet) {

	/// Create and register TRD data factories
	factorySet->Add(new JFactoryT<DTRDDigiHit>());

	factorySet->Add(new DTRDHit_factory());
	factorySet->Add(new DTRDStripCluster_factory());
	factorySet->Add(new DTRDStripCluster_factory_RAW());
	factorySet->Add(new DTRDPoint_factory());
	factorySet->Add(new DTRDSegment_factory());

	//factorySet->Add(new DTRDGeometry_factory());
	//factorySet->Add(new JFactoryT<DTRDTruthGEMHit>());
	//factorySet->Add(new JFactoryT<DTRDTruthWireHit>());
}

