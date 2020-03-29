
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


class JFactoryGenerator_DAQ : public JFactoryGenerator{
public:

    void GenerateFactories(JFactorySet *factory_set) override {

        factory_set->Add(new JFactoryT<Df250Config>());
        factory_set->Add(new JFactoryT<Df250PulseIntegral>());
        factory_set->Add(new JFactoryT<Df250StreamingRawData>());
        factory_set->Add(new JFactoryT<Df250WindowSum>());
        factory_set->Add(new JFactoryT<Df250PulseRawData>());
        factory_set->Add(new JFactoryT<Df250TriggerTime>());
        factory_set->Add(new JFactoryT<Df250PulseTime>());
        factory_set->Add(new JFactoryT<Df250PulsePedestal>());
        factory_set->Add(new JFactoryT<Df250PulseData>());
        factory_set->Add(new JFactoryT<Df250WindowRawData>());
        factory_set->Add(new JFactoryT<Df125Config>());
        factory_set->Add(new JFactoryT<Df125TriggerTime>());
        factory_set->Add(new JFactoryT<Df125PulseIntegral>());
        factory_set->Add(new JFactoryT<Df125PulseTime>());
        factory_set->Add(new JFactoryT<Df125PulsePedestal>());
        factory_set->Add(new JFactoryT<Df125PulseRawData>());
        factory_set->Add(new JFactoryT<Df125WindowRawData>());
        factory_set->Add(new JFactoryT<Df125CDCPulse>());
        factory_set->Add(new JFactoryT<Df125FDCPulse>());
        factory_set->Add(new JFactoryT<DF1TDCHit>());
        factory_set->Add(new JFactoryT<DF1TDCConfig>());
        factory_set->Add(new JFactoryT<DF1TDCTriggerTime>());
        factory_set->Add(new JFactoryT<DCAEN1290TDCConfig>());
        factory_set->Add(new JFactoryT<DCAEN1290TDCHit>());
        factory_set->Add(new JFactoryT<DCODAEventInfo>());
        factory_set->Add(new JFactoryT<DCODAControlEvent>());
        factory_set->Add(new JFactoryT<DCODAROCInfo>());
        factory_set->Add(new JFactoryT<DTSscalers>());
        factory_set->Add(new JFactoryT<DEPICSvalue>());
        factory_set->Add(new JFactoryT<DEventTag>());
        factory_set->Add(new JFactoryT<Df250BORConfig>());
        factory_set->Add(new JFactoryT<Df125BORConfig>());
        factory_set->Add(new JFactoryT<DF1TDCBORConfig>());
        factory_set->Add(new JFactoryT<DCAEN1290TDCBORConfig>());
        factory_set->Add(new JFactoryT<DTSGBORConfig>());
        factory_set->Add(new JFactoryT<DL1Info>());
        factory_set->Add(new JFactoryT<Df250Scaler>());
        factory_set->Add(new JFactoryT<Df250AsyncPedestal>());
        factory_set->Add(new JFactoryT<DDIRCTriggerTime>());
        factory_set->Add(new JFactoryT<DDIRCTDCHit>());
        factory_set->Add(new JFactoryT<DDIRCADCHit>());
        factory_set->Add(new JFactoryT<DGEMSRSWindowRawData>());

        factory_set->Add(new DBeamCurrent_factory());
        factory_set->Add(new Df125EmulatorAlgorithm_factory());
        factory_set->Add(new Df125EmulatorAlgorithm_factory_v2());
        factory_set->Add(new Df250EmulatorAlgorithm_factory());
        factory_set->Add(new Df250EmulatorAlgorithm_factory_v1());
        factory_set->Add(new Df250EmulatorAlgorithm_factory_v2());
        factory_set->Add(new Df250EmulatorAlgorithm_factory_v3());
    }
};

