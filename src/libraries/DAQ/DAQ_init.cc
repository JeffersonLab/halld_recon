
#include <JANA/JFactoryGenerator.h>

#include "DBeamCurrent_factory.h"
#include "DBeamHelicity_factory.h"
#include "DBeamHelicity_factory_CORRECTED.h"
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
#include "DHELIDigiHit.h"
#include "DHelicityData.h"
#include "DHelicityDataTriggerTime.h"
#include "Df125EmulatorAlgorithm_factory.h"
#include "Df125EmulatorAlgorithm_factory_v2.h"
#include "Df250EmulatorAlgorithm_factory.h"
#include "Df250EmulatorAlgorithm_factory_v1.h"
#include "Df250EmulatorAlgorithm_factory_v2.h"
#include "Df250EmulatorAlgorithm_factory_v3.h"

#include <JANA/JFactorySet.h>

#define MyTypes(X) \

void DAQ_init(JFactorySet *factorySet) {

    factorySet->Add(new DBeamCurrent_factory());
	factorySet->Add(new DBeamHelicity_factory());
	factorySet->Add(new DBeamHelicity_factory_CORRECTED());
    factorySet->Add(new Df125EmulatorAlgorithm_factory());
    factorySet->Add(new Df125EmulatorAlgorithm_factory_v2());
    factorySet->Add(new Df250EmulatorAlgorithm_factory());
    factorySet->Add(new Df250EmulatorAlgorithm_factory_v1());
    factorySet->Add(new Df250EmulatorAlgorithm_factory_v2());
    factorySet->Add(new Df250EmulatorAlgorithm_factory_v3());

    factorySet->Add(new JFactoryT<Df250Config>());
    factorySet->Add(new JFactoryT<Df250PulseIntegral>());
    factorySet->Add(new JFactoryT<Df250StreamingRawData>());
    factorySet->Add(new JFactoryT<Df250WindowSum>());
    factorySet->Add(new JFactoryT<Df250PulseRawData>());
    factorySet->Add(new JFactoryT<Df250TriggerTime>());
    factorySet->Add(new JFactoryT<Df250PulseTime>());
    factorySet->Add(new JFactoryT<Df250PulsePedestal>());
    factorySet->Add(new JFactoryT<Df250PulseData>());
    factorySet->Add(new JFactoryT<Df250WindowRawData>());
    factorySet->Add(new JFactoryT<Df125Config>());
    factorySet->Add(new JFactoryT<Df125TriggerTime>());
    factorySet->Add(new JFactoryT<Df125PulseIntegral>());
    factorySet->Add(new JFactoryT<Df125PulseTime>());
    factorySet->Add(new JFactoryT<Df125PulsePedestal>());
    factorySet->Add(new JFactoryT<Df125PulseRawData>());
    factorySet->Add(new JFactoryT<Df125WindowRawData>());
    factorySet->Add(new JFactoryT<Df125CDCPulse>());
    factorySet->Add(new JFactoryT<Df125FDCPulse>());
    factorySet->Add(new JFactoryT<DF1TDCHit>());
    factorySet->Add(new JFactoryT<DF1TDCConfig>());
    factorySet->Add(new JFactoryT<DF1TDCTriggerTime>());
    factorySet->Add(new JFactoryT<DCAEN1290TDCConfig>());
    factorySet->Add(new JFactoryT<DCAEN1290TDCHit>());
    factorySet->Add(new JFactoryT<DCODAEventInfo>());
    factorySet->Add(new JFactoryT<DCODAControlEvent>());
    factorySet->Add(new JFactoryT<DCODAROCInfo>());
    factorySet->Add(new JFactoryT<DTSscalers>());
    factorySet->Add(new JFactoryT<DEPICSvalue>());
    factorySet->Add(new JFactoryT<DEventTag>());
	factorySet->Add(new JFactoryT<DHELIDigiHit>());
    factorySet->Add(new JFactoryT<Df250BORConfig>());
    factorySet->Add(new JFactoryT<Df125BORConfig>());
    factorySet->Add(new JFactoryT<DF1TDCBORConfig>());
    factorySet->Add(new JFactoryT<DCAEN1290TDCBORConfig>());
    factorySet->Add(new JFactoryT<DTSGBORConfig>());
    factorySet->Add(new JFactoryT<DL1Info>());
    factorySet->Add(new JFactoryT<Df250Scaler>());
    factorySet->Add(new JFactoryT<Df250AsyncPedestal>());
    factorySet->Add(new JFactoryT<DDIRCTriggerTime>());
    factorySet->Add(new JFactoryT<DDIRCTDCHit>());
    factorySet->Add(new JFactoryT<DDIRCADCHit>());
    factorySet->Add(new JFactoryT<DGEMSRSWindowRawData>());
	factorySet->Add(new JFactoryT<DHelicityData>());
	factorySet->Add(new JFactoryT<DHelicityDataTriggerTime>());

}

