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

class JEventProcessor_cpp_skim:public JEventProcessor{
	public:
		JEventProcessor_cpp_skim();
		~JEventProcessor_cpp_skim();
		const char* className(void) {return "JEventProcessor_cpp_skim";}
    int num_epics_events;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

};

#endif // _JEventProcessor_cpp_skim_

