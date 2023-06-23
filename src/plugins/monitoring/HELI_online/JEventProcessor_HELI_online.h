// $Id$
//
//    File: JEventProcessor_HELI_online.h
// Created: Thu Jun 22 16:59:57 EDT 2023
// Creator: jrsteven (on Linux ifarm1801.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_HELI_online_
#define _JEventProcessor_HELI_online_

#include "TH1F.h"

#include <JANA/JEventProcessor.h>

#include <DAQ/DBeamHelicity.h>

class JEventProcessor_HELI_online:public jana::JEventProcessor{
	public:
		JEventProcessor_HELI_online();
		~JEventProcessor_HELI_online();
		const char* className(void){return "JEventProcessor_HELI_online";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		TH1F *dBeamHelicity;
		
};

#endif // _JEventProcessor_HELI_online_

