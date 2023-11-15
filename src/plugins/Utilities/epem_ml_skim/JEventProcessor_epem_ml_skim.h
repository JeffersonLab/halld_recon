// $Id$
//
//    File: JEventProcessor_epem_ml_skim.h
// Created: Mon Jun  5 10:42:13 EDT 2023
// Creator: acschick (on Linux ifarm1801.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_epem_ml_skim_
#define _JEventProcessor_epem_ml_skim_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"
#include "TLorentzRotation.h"

#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>


class JEventProcessor_epem_ml_skim:public jana::JEventProcessor{
	public:
		JEventProcessor_epem_ml_skim();
		~JEventProcessor_epem_ml_skim();
		const char* className(void){return "JEventProcessor_epem_ml_skim";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		long int events_read, events_written;
};

#endif // _JEventProcessor_epem_ml_skim_

