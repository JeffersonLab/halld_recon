#ifndef _DTrigger_factory_
#define _DTrigger_factory_

#include <JANA/JFactory.h>
#include "DTrigger.h"
#include "DL1Trigger.h"
#include "DL3Trigger.h"
#include "DMCTrigger.h"
#include "DL1MCTrigger.h"
#include "DANA/DStatusBits.h"

using namespace std;
using namespace jana;

class DTrigger_factory : public jana::JFactory<DTrigger>
{
	public:
		DTrigger_factory(){};
		virtual ~DTrigger_factory(){};

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t evnt(JEventLoop* locEventLoop, uint64_t locEventNumber);
		
		bool EMULATE_BCAL_LED_TRIGGER;
		bool EMULATE_FCAL_LED_TRIGGER;

        bool EMULATE_CAL_ENERGY_SUMS;
		
		unsigned int BCAL_LED_NHITS_THRESHOLD;
		unsigned int FCAL_LED_NHITS_THRESHOLD;
};

#endif // _DTrigger_factory_
