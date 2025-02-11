// $Id$
//
//    File: DReaction_factory_trackeff_missing.h
// Created: Wed Feb 25 08:58:19 EST 2015
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-504.8.1.el6.x86_64 x86_64)
//

#ifndef _DReaction_factory_trackeff_missing_
#define _DReaction_factory_trackeff_missing_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>
#include "ANALYSIS/DSourceComboP4Handler.h"
#include "ANALYSIS/DSourceComboTimeHandler.h"

using namespace std;

class DReaction_factory_trackeff_missing : public JFactoryT<DReaction>
{
	public:
		DReaction_factory_trackeff_missing()
		{
			SetTag("trackeff_missing");
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event.
			SetFactoryFlag(PERSISTENT);
		}

	private:
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void Finish() override;

		bool dDebugFlag = false;

		//ACTIONS
		void Add_MassHistograms(DReaction* locReaction, bool locUseKinFitResultsFlag, string locBaseUniqueName = "");
		void Create_InvariantMassHistogram(DReaction* locReaction, Particle_t locPID, bool locUseKinFitResultsFlag, string locBaseUniqueName);
		void Create_MissingMassSquaredHistogram(DReaction* locReaction, Particle_t locPID, bool locUseKinFitResultsFlag, string locBaseUniqueName, int locMissingMassOffOfStepIndex, const deque<Particle_t>& locMissingMassOffOfPIDs);
		void Add_PostKinfitTimingCuts(DReaction* locReaction);

		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks

		DSourceComboP4Handler* dSourceComboP4Handler = nullptr;
		DSourceComboTimeHandler* dSourceComboTimeHandler = nullptr;
};

#endif // _DReaction_factory_trackeff_missing_
