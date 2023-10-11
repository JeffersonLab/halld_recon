
#include <JANA/JFactoryGenerator.h>

#include "DBeamCurrent_factory.h"
#include "Df250Config.h"
#include "Df250PulseIntegral.h"
#include "Df250StreamingRawData.h"
#include "Df250WindowSum.h"
#include "Df250PulseRawData.h"
#include "Df250TriggerTime.h"
#include "Df250PulseTime.h"
#include "Df250PulsePedestal.h"
#include "Df250PulseData.h"
#include "Df250WindowRawData.h"
#include "Df125Config.h"
#include "Df125TriggerTime.h"
#include "Df125PulseIntegral.h"
#include "Df125PulseTime.h"
#include "Df125PulsePedestal.h"
#include "Df125PulseRawData.h"
#include "Df125WindowRawData.h"
#include "Df125CDCPulse.h"
#include "Df125FDCPulse.h"
#include "DF1TDCConfig.h"
#include "DF1TDCHit.h"
#include "DF1TDCTriggerTime.h"
#include "DCAEN1290TDCConfig.h"
#include "DCAEN1290TDCHit.h"
#include "DCODAEventInfo.h"
#include "DCODAControlEvent.h"
#include "DCODAROCInfo.h"
#include "DTSscalers.h"
#include "DEPICSvalue.h"
#include "DEventTag.h"
#include "Df250BORConfig.h"
#include "Df125BORConfig.h"
#include "DF1TDCBORConfig.h"
#include "DCAEN1290TDCBORConfig.h"
#include "DTSGBORConfig.h"
#include "DL1Info.h"
#include "Df250Scaler.h"
#include "Df250AsyncPedestal.h"
#include "DDIRCTriggerTime.h"
#include "DDIRCTDCHit.h"
#include "DDIRCADCHit.h"
#include "DGEMSRSWindowRawData.h"
#include "Df125EmulatorAlgorithm_factory.h"
#include "Df125EmulatorAlgorithm_factory_v2.h"
#include "Df250EmulatorAlgorithm_factory.h"
#include "Df250EmulatorAlgorithm_factory_v1.h"
#include "Df250EmulatorAlgorithm_factory_v2.h"
#include "Df250EmulatorAlgorithm_factory_v3.h"

#include <JANA/Compatibility/JGetObjectsFactory.h>

#define MyTypes(X) \

void DAQ_init(JFactorySet *factorySet) {

    factorySet->Add(new DBeamCurrent_factory());
    factorySet->Add(new Df125EmulatorAlgorithm_factory());
    factorySet->Add(new Df125EmulatorAlgorithm_factory_v2());
    factorySet->Add(new Df250EmulatorAlgorithm_factory());
    factorySet->Add(new Df250EmulatorAlgorithm_factory_v1());
    factorySet->Add(new Df250EmulatorAlgorithm_factory_v2());
    factorySet->Add(new Df250EmulatorAlgorithm_factory_v3());

    factorySet->Add(new JGetObjectsFactory<Df250Config>());
    factorySet->Add(new JGetObjectsFactory<Df250PulseIntegral>());
    factorySet->Add(new JGetObjectsFactory<Df250StreamingRawData>());
    factorySet->Add(new JGetObjectsFactory<Df250WindowSum>());
    factorySet->Add(new JGetObjectsFactory<Df250PulseRawData>());
    factorySet->Add(new JGetObjectsFactory<Df250TriggerTime>());
    factorySet->Add(new JGetObjectsFactory<Df250PulseTime>());
    factorySet->Add(new JGetObjectsFactory<Df250PulsePedestal>());
    factorySet->Add(new JGetObjectsFactory<Df250PulseData>());
    factorySet->Add(new JGetObjectsFactory<Df250WindowRawData>());
    factorySet->Add(new JGetObjectsFactory<Df125Config>());
    factorySet->Add(new JGetObjectsFactory<Df125TriggerTime>());
    factorySet->Add(new JGetObjectsFactory<Df125PulseIntegral>());
    factorySet->Add(new JGetObjectsFactory<Df125PulseTime>());
    factorySet->Add(new JGetObjectsFactory<Df125PulsePedestal>());
    factorySet->Add(new JGetObjectsFactory<Df125PulseRawData>());
    factorySet->Add(new JGetObjectsFactory<Df125WindowRawData>());
    factorySet->Add(new JGetObjectsFactory<Df125CDCPulse>());
    factorySet->Add(new JGetObjectsFactory<Df125FDCPulse>());
    factorySet->Add(new JGetObjectsFactory<DF1TDCHit>());
    factorySet->Add(new JGetObjectsFactory<DF1TDCConfig>());
    factorySet->Add(new JGetObjectsFactory<DF1TDCTriggerTime>());
    factorySet->Add(new JGetObjectsFactory<DCAEN1290TDCConfig>());
    factorySet->Add(new JGetObjectsFactory<DCAEN1290TDCHit>());
    factorySet->Add(new JGetObjectsFactory<DCODAEventInfo>());
    factorySet->Add(new JGetObjectsFactory<DCODAControlEvent>());
    factorySet->Add(new JGetObjectsFactory<DCODAROCInfo>());
    factorySet->Add(new JGetObjectsFactory<DTSscalers>());
    factorySet->Add(new JGetObjectsFactory<DEPICSvalue>());
    factorySet->Add(new JGetObjectsFactory<DEventTag>());
    factorySet->Add(new JGetObjectsFactory<Df250BORConfig>());
    factorySet->Add(new JGetObjectsFactory<Df125BORConfig>());
    factorySet->Add(new JGetObjectsFactory<DF1TDCBORConfig>());
    factorySet->Add(new JGetObjectsFactory<DCAEN1290TDCBORConfig>());
    factorySet->Add(new JGetObjectsFactory<DTSGBORConfig>());
    factorySet->Add(new JGetObjectsFactory<DL1Info>());
    factorySet->Add(new JGetObjectsFactory<Df250Scaler>());
    factorySet->Add(new JGetObjectsFactory<Df250AsyncPedestal>());
    factorySet->Add(new JGetObjectsFactory<DDIRCTriggerTime>());
    factorySet->Add(new JGetObjectsFactory<DDIRCTDCHit>());
    factorySet->Add(new JGetObjectsFactory<DDIRCADCHit>());
    factorySet->Add(new JGetObjectsFactory<DGEMSRSWindowRawData>());
}

