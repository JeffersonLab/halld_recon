// $Id$
//
//    File: JEventProcessor_fmwpc_scan.h
// Created: Wed Nov 24 19:47:15 EST 2021
// Creator: njarvis (on Linux ifarm1802.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_fmwpc_scan_
#define _JEventProcessor_fmwpc_scan_

#include <JANA/JEventProcessor.h>
#include <TTree.h>

class JEventProcessor_fmwpc_scan:public jana::JEventProcessor{
	public:
		JEventProcessor_fmwpc_scan();
		~JEventProcessor_fmwpc_scan();
		const char* className(void){return "JEventProcessor_fmwpc_scan";}
                TTree *t;

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_fmwpc_scan_

