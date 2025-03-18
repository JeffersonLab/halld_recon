// $Id$
//
//    File: DReaction_factory_B3pi_eff_missgamma.h
// Created: Fri Jun 30 00:38:22 EDT 2017
// Creator: jmhardin (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DReaction_factory_B3pi_eff_missgamma_
#define _DReaction_factory_B3pi_eff_missgamma_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

using namespace std;

class DReaction_factory_B3pi_eff_missgamma : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_B3pi_eff_missgamma()
		{
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event. 
			SetFactoryFlag(PERSISTENT);
			SetTag("B3pi_eff_missgamma");
		}

	private:
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void Finish() override;

		double dBeamBunchPeriod;
		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks
};

#endif // _DReaction_factory_B3pi_eff_missgamma_

