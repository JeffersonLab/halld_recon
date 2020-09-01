/*
 * TRD_init.cc
 *
 *  Created on: Oct 24, 2019
 *      Author: jrsteven
 */

#include <JANA/JEventLoop.h>
using namespace jana;

#include "DTRDDigiHit.h"
#include "DGEMDigiWindowRawData.h"
#include "DTRDHit_factory.h"
#include "DGEMHit_factory.h"
#include "DTRDStripCluster_factory.h"
#include "DTRDPoint_factory.h"
#include "DGEMStripCluster_factory.h"
#include "DGEMPoint_factory.h"
#include "DPadGEMPoint_factory.h"

jerror_t TRD_init(JEventLoop *loop) {

	/// Create and register TRD data factories
	loop->AddFactory(new JFactory<DTRDDigiHit>());
	loop->AddFactory(new JFactory<DGEMDigiWindowRawData>());
	loop->AddFactory(new DTRDHit_factory());
	loop->AddFactory(new DGEMHit_factory());

	loop->AddFactory(new DTRDStripCluster_factory());
	loop->AddFactory(new DTRDPoint_factory());
	loop->AddFactory(new DGEMStripCluster_factory());
	loop->AddFactory(new DGEMPoint_factory());
	loop->AddFactory(new DPadGEMPoint_factory);

	//loop->AddFactory(new DTRDGeometry_factory());
	//loop->AddFactory(new JFactory<DTRDTruthGEMHit>());
	//loop->AddFactory(new JFactory<DTRDTruthWireHit>());

	return NOERROR;
}

