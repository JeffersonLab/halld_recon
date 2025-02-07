/*
 * TRD_init.cc
 *
 *  Created on: Oct 24, 2019
 *      Author: jrsteven
 */

#include <JANA/JEventLoop.h>
using namespace jana;

#include "DTRDDigiHit.h"
#include "DTRDHit_factory.h"
#include "DTRDStripCluster_factory.h"
#include "DTRDStripCluster_factory_RAW.h"
#include "DTRDPoint_factory.h"
#include "DTRDSegment_factory.h"


jerror_t TRD_init(JEventLoop *loop) {

	/// Create and register TRD data factories
	loop->AddFactory(new JFactory<DTRDDigiHit>());
	loop->AddFactory(new DTRDHit_factory());

	loop->AddFactory(new DTRDStripCluster_factory());
	loop->AddFactory(new DTRDStripCluster_factory_RAW());
	loop->AddFactory(new DTRDPoint_factory());
	loop->AddFactory(new DTRDSegment_factory());

	//loop->AddFactory(new DTRDGeometry_factory());
	//loop->AddFactory(new JFactory<DTRDTruthGEMHit>());
	//loop->AddFactory(new JFactory<DTRDTruthWireHit>());

	return NOERROR;
}

