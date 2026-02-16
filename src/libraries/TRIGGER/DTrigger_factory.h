#ifndef _DTrigger_factory_
#define _DTrigger_factory_

#include <JANA/JFactoryT.h>
#include "DTrigger.h"
#include "DL1Trigger.h"
#include "DL3Trigger.h"
#include "DMCTrigger.h"
#include "DL1MCTrigger.h"
#include "DANA/DStatusBits.h"

using namespace std;


class DTrigger_factory : public JFactoryT<DTrigger>
{
	public:
		DTrigger_factory(){};
		virtual ~DTrigger_factory(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override; 
		void Process(const std::shared_ptr<const JEvent>& event) override;
		
		bool EMULATE_BCAL_LED_TRIGGER;
		bool EMULATE_FCAL_LED_TRIGGER;

        bool EMULATE_CAL_ENERGY_SUMS;
		
		unsigned int BCAL_LED_NHITS_THRESHOLD;
		unsigned int FCAL_LED_NHITS_THRESHOLD;
		
		string MC_TRIGGER_TAG = "";
		string DATA_SIM_TRIGGER_TAG = "DATA";
};

#endif // _DTrigger_factory_
