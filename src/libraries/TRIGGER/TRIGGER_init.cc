// $Id$
//
//    File: TRIGGER_init.cc
// Created: Wed Oct 24 06:29:48 EDT 2012
// Creator: davidl (on Darwin eleanor.jlab.org 12.2.0 i386)
//

#include <JANA/JFactorySet.h>

#include "DMCTrigger_factory.h"
#include "DL1MCTrigger_factory.h"
#include "DL3Trigger_factory.h"
#include "DL1Trigger_factory.h"
#include "DTrigger_factory.h"

void TRIGGER_init(JFactorySet *factorySet) {

	factorySet->Add(new DMCTrigger_factory());
	factorySet->Add(new DL1MCTrigger_factory());
	factorySet->Add(new DL3Trigger_factory());
	factorySet->Add(new DL1Trigger_factory());
	factorySet->Add(new DTrigger_factory());
}



