// $Id$
//
//    File: JEventProcessor_TAGGER_online.h
// Created: Thu Feb 18 07:45:18 EST 2016
// Creator: jrsteven (on Linux gluon110.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGGER_online_
#define _JEventProcessor_TAGGER_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <TAGGER/DTAGMHit.h>
#include <START_COUNTER/DSCHit.h>
#include <PID/DBeamPhoton.h>

#include "TH1.h"
#include "TH2.h"
#include "TDirectoryFile.h"

class JEventProcessor_TAGGER_online:public JEventProcessor{
	public:
		JEventProcessor_TAGGER_online();
		~JEventProcessor_TAGGER_online();

	private:
		TH2D *dTAGMPulsePeak_Column, *dTAGMIntegral_Column;
		TH2D *dTaggerEnergy_DeltaTSC;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_TAGGER_online_

