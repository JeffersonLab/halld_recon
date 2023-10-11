// $Id$
//
//    File: JEventProcessor_pedestal_online.h
// Created: Thu Aug  7 09:37:01 EDT 2014
// Creator: dalton (on Linux gluon05.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_pedestal_online_
#define _JEventProcessor_pedestal_online_

#include <TDirectory.h>

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

class JEventProcessor_pedestal_online:public JEventProcessor{
	public:

		JEventProcessor_pedestal_online();
		~JEventProcessor_pedestal_online();

		TDirectory *maindir;
		TDirectory *peddir;

	private:
		uint32_t VERBOSE;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_pedestal_online_

