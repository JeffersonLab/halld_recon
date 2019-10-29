/*
 * TRD_init.cc
 *
 *  Created on: Oct 24, 2019
 *      Author: jrsteven
 */

#include <JANA/JEventLoop.h>
using namespace jana;

#include "DTRDDigiHit.h"
//#include "DTRDGEMHit.h"
//#include "DTRDWireHit.h"

jerror_t TRD_init(JEventLoop *loop) {

	/// Create and register TRD data factories
	loop->AddFactory(new JFactory<DTRDDigiHit>());
	//loop->AddFactory(new DTRDGeometry_factory());
	//loop->AddFactory(new DTRDGEMHit_factory());
	//loop->AddFactory(new DTRDWireHit_factory());
	//loop->AddFactory(new JFactory<DTRDTruthGEMHit>());
	//loop->AddFactory(new JFactory<DTRDTruthWireHit>());

	return NOERROR;
}

