// $Id$
//
//    File: JEventProcessor_PS_E_calib.h
// Created: Thu Jul  9 17:44:32 EDT 2015
// Creator: aebarnes (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_PS_E_calib_
#define _JEventProcessor_PS_E_calib_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

class JEventProcessor_PS_E_calib:public JEventProcessor{
	public:
		JEventProcessor_PS_E_calib();
		~JEventProcessor_PS_E_calib();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_PS_E_calib_

