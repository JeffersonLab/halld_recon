// $Id$
//
//    File: JEventProcessor_BCAL_point_time.h
// Created: Fri Apr  8 12:59:18 EDT 2016
// Creator: dalton (on Linux gluon109.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_point_time_
#define _JEventProcessor_BCAL_point_time_

#include <TDirectory.h>

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <BCAL/DBCALGeometry.h>

class JEventProcessor_BCAL_point_time:public JEventProcessor{
	public:
		JEventProcessor_BCAL_point_time();
		~JEventProcessor_BCAL_point_time();

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

#endif // _JEventProcessor_BCAL_point_time_

