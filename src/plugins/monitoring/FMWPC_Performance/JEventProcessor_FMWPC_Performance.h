// $Id$
//
//    File: JEventProcessor_FMWPC_Performance.h
// Created: Fri Mar  4 10:17:38 EST 2022
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_FMWPC_Performance_
#define _JEventProcessor_FMWPC_Performance_

#include <JANA/JEventProcessor.h>

class JEventProcessor_FMWPC_Performance:public jana::JEventProcessor{
	public:
		JEventProcessor_FMWPC_Performance();
		~JEventProcessor_FMWPC_Performance();
		const char* className(void){return "JEventProcessor_FMWPC_Performance";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
		

};

#endif // _JEventProcessor_FMWPC_Performance_

