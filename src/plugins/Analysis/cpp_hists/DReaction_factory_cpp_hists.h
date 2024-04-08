// $Id$
//
//    File: DReaction_factory_cpp_hists.h
// Created: Tue May  3 09:24:43 EDT 2022
// Creator: aaustreg (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DReaction_factory_cpp_hists_
#define _DReaction_factory_cpp_hists_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

#include "DCustomAction_cpp_hists.h"

using namespace std;

class DReaction_factory_cpp_hists : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_cpp_hists()
		{
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event. 
			SetFactoryFlag(PERSISTENT);
		}
		const char* Tag(void){return "cpp_hists";}

	private:
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void Finish() override;

		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks
};

#endif // _DReaction_factory_cpp_hists_

