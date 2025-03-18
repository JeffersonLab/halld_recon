#ifndef _DL1Trigger_factory_
#define _DL1Trigger_factory_

#include <JANA/JFactoryT.h>
#include "DL1Trigger.h"

class DL1Trigger_factory:public JFactoryT<DL1Trigger>{
	public:
		DL1Trigger_factory(){};
		~DL1Trigger_factory(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DL1Trigger_factory_

