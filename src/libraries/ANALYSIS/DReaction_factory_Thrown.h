#ifndef _DReaction_factory_Thrown_
#define _DReaction_factory_Thrown_

#include <iostream>
#include <deque>

#include "JANA/JFactory.h"
#include "particleType.h"

#include "ANALYSIS/DReaction.h"
#include "PID/DMCReaction.h"
#include "TRACKING/DMCThrown.h"
#include "ANALYSIS/DAnalysisUtilities.h"

using namespace std;
using namespace DAnalysis;

class DAnalysisUtilities;

class DReaction_factory_Thrown:public JFactoryT<DReaction>
{
	public:
		DReaction_factory_Thrown(){
			SetObjectName("DReaction"); // As opposed to "DAnalysis::DReaction"
			SetTag("Thrown");
		}
		~DReaction_factory_Thrown(){};

		DReaction* Build_ThrownReaction(const std::shared_ptr<const JEvent>& locEvent, deque<pair<const DMCThrown*, deque<const DMCThrown*> > >& locThrownSteps);

		void Recycle_Reaction(DReaction* locReaction); //deletes reaction, but recycles steps

		void BeginRun(const std::shared_ptr<const JEvent>& event) override;

	private:
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		const DAnalysisUtilities* dAnalysisUtilities = nullptr;

		DReactionStep* Get_ReactionStepResource(void);

		deque<DReactionStep*> dReactionStepPool_All;
		deque<DReactionStep*> dReactionStepPool_Available;

		size_t MAX_dReactionStepPoolSize = 10;
};

#endif // _DReaction_factory_Thrown_

