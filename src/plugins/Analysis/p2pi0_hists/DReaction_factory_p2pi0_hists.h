// $Id$
//
//    File: DReaction_factory_p2pi0_hists.h
// Created: Tue May 19 07:56:15 EDT 2015
// Creator: jrsteven (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _DReaction_factory_p2pi0_hists_
#define _DReaction_factory_p2pi0_hists_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

#include "DCustomAction_p2pi0_hists.h"

using namespace std;

class DReaction_factory_p2pi0_hists : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_p2pi0_hists()
		{
			SetTag("p2pi0_hists");
			SetFactoryFlag(PERSISTENT);
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event.
		}

	private:
		void Init() override;
		void Finish() override;

		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks
};

#endif // _DReaction_factory_p2pi0_hists_

