// $Id$
//
//    File: DReaction_factory_p4pi_hists.h
// Created: Mon Aug 29 16:20:57 EDT 2016
// Creator: aaustreg (on Linux halld01.jlab.org 2.6.32-642.3.1.el6.x86_64 x86_64)
//

#ifndef _DReaction_factory_p4pi_hists_
#define _DReaction_factory_p4pi_hists_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

using namespace std;

class DReaction_factory_p4pi_hists : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_p4pi_hists()
		{
			SetTag("p4pi_hists");
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

#endif // _DReaction_factory_p4pi_hists_

