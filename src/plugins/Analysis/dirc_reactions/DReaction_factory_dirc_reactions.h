// $Id$
//
//    File: DReaction_factory_dirc_reactions.h
//

#ifndef _DReaction_factory_dirc_reactions_
#define _DReaction_factory_dirc_reactions_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

#include "DCustomAction_dirc_reactions.h"
#include "DCustomAction_dirc_track_pair.h"

using namespace std;

class DReaction_factory_dirc_reactions : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_dirc_reactions()
		{
			SetTag("dirc_reactions");
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

#endif // _DReaction_factory_dirc_reactions_

