// $Id$
//
//    File: TRIGGER_init.cc
// Created: Wed Oct 24 06:29:48 EDT 2012
// Creator: davidl (on Darwin eleanor.jlab.org 12.2.0 i386)
//

#include <JANA/JEventLoop.h>
using namespace jana;

#include "DMCTrigger_factory.h"
#include "DL1MCTrigger_factory.h"
#include "DL1MCTrigger_factory_DATA.h"
#include "DL3Trigger_factory.h"
#include "DL1Trigger_factory.h"
#include "DTrigger_factory.h"

jerror_t TRIGGER_init(JEventLoop *loop) {

	loop->AddFactory(new DMCTrigger_factory());
	loop->AddFactory(new DL1MCTrigger_factory());
	loop->AddFactory(new DL1MCTrigger_factory_DATA());
	loop->AddFactory(new DL3Trigger_factory());
	loop->AddFactory(new DL1Trigger_factory());
	loop->AddFactory(new DTrigger_factory());

	return NOERROR;
}



