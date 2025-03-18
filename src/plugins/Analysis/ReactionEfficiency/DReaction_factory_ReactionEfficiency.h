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

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

#include "DCustomAction_RecoilMass.h"
#include "DCustomAction_MissingMatch.h"

using namespace std;

class DReaction_factory_ReactionEfficiency : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_ReactionEfficiency()
		{
			SetTag("ReactionEfficiency");
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event.
			SetFactoryFlag(PERSISTENT);
		}

	private:
		void registerReaction(DReaction* locReaction, const vector<string>& locReactionsToWrite);
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void Finish() override;

		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks
};

#endif // _DReaction_factory_ReactionEfficiency_

