// $Id$
//
//    File: JEventProcessor_FMWPC_Performance.h
// Created: Fri Mar  4 10:17:38 EST 2022
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_FMWPC_Performance_
#define _JEventProcessor_FMWPC_Performance_

#include <JANA/JEventProcessor.h>

class JEventProcessor_FMWPC_Performance:public JEventProcessor{
	public:
		JEventProcessor_FMWPC_Performance();
		~JEventProcessor_FMWPC_Performance();
		const char* className(void){return "JEventProcessor_FMWPC_Performance";}

	private:
	void Init() override;
	void BeginRun(const std::shared_ptr<const JEvent>& event) override;
	void Process(const std::shared_ptr<const JEvent>& event) override;
	void EndRun() override;
	void Finish() override;
};

#endif // _JEventProcessor_FMWPC_Performance_

