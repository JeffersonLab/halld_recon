// $Id$
//
//    File: DReaction_factory_ReactionEfficiency.h
// Created: Wed Jun 19 16:52:57 EDT 2019
// Creator: jrsteven (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DReaction_factory_ReactionEfficiency_
#define _DReaction_factory_ReactionEfficiency_

#include <iostream>
#include <iomanip>

#include <JANA/JFactory.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

#include "DCustomAction_RecoilMass.h"
#include "DCustomAction_MissingMatch.h"

using namespace std;
using namespace jana;

class DReaction_factory_ReactionEfficiency : public jana::JFactory<DReaction>
{
	public:
		DReaction_factory_ReactionEfficiency()
		{
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event. 
			SetFactoryFlag(PERSISTANT);
		}
		const char* Tag(void){return "ReactionEfficiency";}

	private:
		void registerReaction(DReaction* locReaction, const vector<string>& locReactionsToWrite);
		jerror_t evnt(JEventLoop* locEventLoop, uint64_t locEventNumber);
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks
};

#endif // _DReaction_factory_ReactionEfficiency_

