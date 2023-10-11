// $Id$
//
//    File: DReaction_factory_p2pi.h
// Created: Thu May  7 16:22:04 EDT 2015
// Creator: mstaib (on Linux gluon109.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#ifndef _DReaction_factory_p2pi_
#define _DReaction_factory_p2pi_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

using namespace std;

class DReaction_factory_p2pi : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_p2pi()
		{
			SetTag("p2pi");
			SetFactoryFlag(PERSISTENT);
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event.
		}

	private:
		void Init() override;
		void Finish() override;

		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks
};

#endif // _DReaction_factory_p2pi_

