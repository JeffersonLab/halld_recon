#ifndef _DReaction_factory_b1pi_hists_
#define _DReaction_factory_b1pi_hists_

#include "JANA/JFactory.h"
#include <JANA/JEvent.h>

#include "particleType.h"

#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DReactionStep.h"
#include "ANALYSIS/DHistogramActions.h"
#include "ANALYSIS/DCutActions.h"

using namespace std;

class DReaction_factory_b1pi_hists : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_b1pi_hists(){
            SetFactoryName("DReaction_factory_b1pi_hists");
            SetObjectName("DReaction");
            SetTag("b1pi_hists");
            SetFactoryFlag(PERSISTENT);
		};
		~DReaction_factory_b1pi_hists(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks!
};

#endif // _DReaction_factory_b1pi_hists_

