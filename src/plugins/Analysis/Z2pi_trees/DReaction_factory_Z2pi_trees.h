// $Id$
//  DReaction_factory_Z2pi_trees.h, modeled after DReaction_factory_p2pi_trees.h
//
//    File: DReaction_factory_p2pi_trees.h
// Created: Wed Mar 29 16:34:58 EDT 2017
// Creator: elton (on Linux ifarm1401.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DReaction_factory_Z2pi_trees_
#define _DReaction_factory_Z2pi_trees_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

using namespace std;

class DReaction_factory_Z2pi_trees : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_Z2pi_trees()
		{
			SetTag("Z2pi_trees");
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

#endif // _DReaction_factory_Z2pi_trees_

