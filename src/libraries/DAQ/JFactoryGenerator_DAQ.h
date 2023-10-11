// $Id$
// $HeadURL$
//
//    File: JFactoryGenerator_DAQ.h
// Created: Thu Aug  9 12:40:08 EDT 2012
// Creator: davidl (on Darwin harriet.jlab.org 11.4.0 i386)
//

#ifndef _JFactoryGenerator_DAQ_
#define _JFactoryGenerator_DAQ_

#include <JANA/Compatibility/JGetObjectsFactory.h>
#include <JANA/JFactoryGenerator.h>

#include "Df250Config.h"
#include "Df250PulseIntegral.h"
#include "Df250StreamingRawData.h"
#include "Df250WindowSum.h"
#include "Df250PulseRawData.h"
#include "Df250TriggerTime.h"
#include "Df250PulseTime.h"
#include "Df250PulsePedestal.h"
#include "Df250WindowRawData.h"
#include "Df125Config.h"
#include "Df125TriggerTime.h"
#include "Df125PulseIntegral.h"
#include "Df125PulseTime.h"
#include "Df125PulsePedestal.h"
#include "Df125PulseRawData.h"
#include "Df125WindowRawData.h"
#include "DF1TDCConfig.h"
#include "DF1TDCHit.h"
#include "DF1TDCTriggerTime.h"
#include "DCAEN1290TDCConfig.h"
#include "DCAEN1290TDCHit.h"
#include "DCODAEventInfo.h"
#include "DCODAROCInfo.h"
#include "DEPICSvalue.h"
#include "DL1Info.h"
#include "Df250Scaler.h"
#include "Df250AsyncPedestal.h"


class JFactoryGenerator_DAQ: public JFactoryGenerator{
	/// This is only necessary because we are using JEventSource::GetObjects instead of JEvent::Insert.
	/// I strongly recommend refactoring JEventSource_EVIOpp, and getting rid of this in the process.
	public:

		void GenerateFactories(JFactorySet *factory_set) override {
			factory_set->Add(new JGetObjectsFactory<Df250Config>());
			factory_set->Add(new JGetObjectsFactory<Df250PulseIntegral>());
			factory_set->Add(new JGetObjectsFactory<Df250StreamingRawData>());
			factory_set->Add(new JGetObjectsFactory<Df250WindowSum>());
			factory_set->Add(new JGetObjectsFactory<Df250PulseRawData>());
			factory_set->Add(new JGetObjectsFactory<Df250TriggerTime>());
			factory_set->Add(new JGetObjectsFactory<Df250PulseTime>());
			factory_set->Add(new JGetObjectsFactory<Df250PulsePedestal>());
			factory_set->Add(new JGetObjectsFactory<Df250WindowRawData>());
			factory_set->Add(new JGetObjectsFactory<Df125Config>());
			factory_set->Add(new JGetObjectsFactory<Df125TriggerTime>());
			factory_set->Add(new JGetObjectsFactory<Df125PulseIntegral>());
			factory_set->Add(new JGetObjectsFactory<Df125PulseTime>());
			factory_set->Add(new JGetObjectsFactory<Df125PulsePedestal>());
			factory_set->Add(new JGetObjectsFactory<Df125PulseRawData>());
			factory_set->Add(new JGetObjectsFactory<Df125WindowRawData>());
			factory_set->Add(new JGetObjectsFactory<DF1TDCHit>());
			factory_set->Add(new JGetObjectsFactory<DF1TDCConfig>());
			factory_set->Add(new JGetObjectsFactory<DF1TDCTriggerTime>());
			factory_set->Add(new JGetObjectsFactory<DCAEN1290TDCConfig>());
			factory_set->Add(new JGetObjectsFactory<DCAEN1290TDCHit>());
			factory_set->Add(new JGetObjectsFactory<DCODAEventInfo>());
			factory_set->Add(new JGetObjectsFactory<DCODAROCInfo>());
			factory_set->Add(new JGetObjectsFactory<DEPICSvalue>());
			factory_set->Add(new JGetObjectsFactory<DL1Info>());
			factory_set->Add(new JGetObjectsFactory<Df250Scaler>());
			factory_set->Add(new JGetObjectsFactory<Df250AsyncPedestal>());
		}
};

#endif // _JFactoryGenerator_DAQ_

