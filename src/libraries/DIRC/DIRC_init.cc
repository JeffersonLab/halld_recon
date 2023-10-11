/*
 * DIRC_init.cc
 *
 *  Created on: Oct 11, 2012
 *      Author: yqiang
 *  Modified on: 0ct 7, 2013, yqiang, added DIRCTruthHit factory
 */

#include "DDIRCLut_factory.h"
#include "DDIRCPmtHit_factory.h"
#include "DDIRCLEDRef_factory.h"
#include "DDIRCGeometry_factory.h"
#include "DDIRCTDCDigiHit.h"
#include "DDIRCTruthPmtHit.h"
#include "DDIRCTruthBarHit.h"

#include <JANA/JFactorySet.h>
#include <JANA/Compatibility/JGetObjectsFactory.h>

void DIRC_init(JFactorySet *factorySet) {

	/// Create and register DIRC data factories
	factorySet->Add(new DDIRCGeometry_factory());
	factorySet->Add(new DDIRCPmtHit_factory());
	factorySet->Add(new DDIRCLEDRef_factory());
	factorySet->Add(new DDIRCLut_factory());
	factorySet->Add(new JGetObjectsFactory<DDIRCTDCDigiHit>());
	factorySet->Add(new JGetObjectsFactory<DDIRCTruthPmtHit>());
	factorySet->Add(new JGetObjectsFactory<DDIRCTruthBarHit>());

}

