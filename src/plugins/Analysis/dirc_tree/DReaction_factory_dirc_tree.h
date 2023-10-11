// $Id$
//
//    File: DReaction_factory_dirc_tree.h
//

#ifndef _DReaction_factory_dirc_tree_
#define _DReaction_factory_dirc_tree_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

#include "DCustomAction_dirc_tree.h"

using namespace std;

class DReaction_factory_dirc_tree : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_dirc_tree()
		{
			SetTag("dirc_tree");
			SetFactoryFlag(PERSISTENT);
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event.
		}

	private:
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void Finish() override;

		double dBeamBunchPeriod;
		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks
};

#endif // _DReaction_factory_dirc_tree_

