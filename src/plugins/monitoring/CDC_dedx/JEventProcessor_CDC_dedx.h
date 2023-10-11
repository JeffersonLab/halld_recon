// $Id$
//
//    File: JEventProcessor_CDC_dedx.h
// Created: Thu Aug 16 16:35:42 EDT 2018
// Creator: njarvis (on Linux egbert 2.6.32-696.23.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_CDC_dedx_
#define _JEventProcessor_CDC_dedx_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "TRIGGER/DTrigger.h"
#include "PID/DVertex.h"

#include "TRACKING/DTrackTimeBased.h"

#include <vector>

#include <TDirectory.h>

#include <TH2.h>



class JEventProcessor_CDC_dedx:public JEventProcessor{
	public:
		JEventProcessor_CDC_dedx();
		~JEventProcessor_CDC_dedx();

	private:

                TH2D *dedx_p = NULL;
                TH2D *dedx_p_pos = NULL;
                TH2D *dedx_p_neg = NULL;

                TH2D *intdedx_p = NULL;
                TH2D *intdedx_p_pos = NULL;
                TH2D *intdedx_p_neg = NULL;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_CDC_dedx_

