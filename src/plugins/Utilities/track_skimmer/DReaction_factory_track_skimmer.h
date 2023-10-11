// $Id$
//
//    File: DReaction_factory_track_skimmer.h
// Created: Tue Jan 13 11:08:16 EST 2015
// Creator: Paul (on Darwin Pauls-MacBook-Pro.local 14.0.0 i386)
//

#ifndef _DReaction_factory_track_skimmer_
#define _DReaction_factory_track_skimmer_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

using namespace std;

class DReaction_factory_track_skimmer : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_track_skimmer()
		{
			SetTag("track_skimmer");
			SetFactoryFlag(PERSISTENT);
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event.
		}

	private:
		void Init() override;
		void Finish() override;

		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks
};

#endif // _DReaction_factory_track_skimmer_

