// $Id$
//
//    File: JEventProcessor_cpp_skim.h
// Created: Tue Apr 16 10:14:04 EDT 2024
// Creator: ilarin (on Linux ifarm1802.jlab.org 3.10.0-1160.102.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_cpp_skim_
#define _JEventProcessor_cpp_skim_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"

#include <TRACKING/DTrackWireBased.h>

#include <TH1.h>
#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>

class JEventProcessor_cpp_skim:public jana::JEventProcessor{
	public:
		JEventProcessor_cpp_skim();
		~JEventProcessor_cpp_skim();
		const char* className(void) {return "JEventProcessor_cpp_skim";}
    int num_epics_events;

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

};

#endif // _JEventProcessor_cpp_skim_

