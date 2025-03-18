// $Id$
//
//    File: DReaction_factory_ppi0gamma_hists.h
// Created: Wed Mar 11 20:34:22 EDT 2015
// Creator: jrsteven (on Linux halldw1.jlab.org 2.6.32-504.8.1.el6.x86_64 x86_64)
//

#ifndef _DReaction_factory_ppi0gamma_hists_
#define _DReaction_factory_ppi0gamma_hists_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

#include "DCustomAction_ppi0gamma_Pi0Cuts.h"
#include "DCustomAction_ppi0gamma_hists.h"

using namespace std;

class DReaction_factory_ppi0gamma_hists : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_ppi0gamma_hists()
		{
			SetTag("ppi0gamma_hists");
			SetFactoryFlag(PERSISTENT);
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event.
		}

	private:
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void Finish() override;

		double dBeamBunchPeriod;
		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks

		void PIDCuts(DReaction* locReaction);
};

#endif // _DReaction_factory_ppi0gamma_hists_

