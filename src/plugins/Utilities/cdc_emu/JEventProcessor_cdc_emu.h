// $Id$
//
//    File: JEventProcessor_cdc_emu.h
// Created: Thu Apr 21 10:38:27 EDT 2016
// Creator: njarvis (on Linux egbert 2.6.32-573.3.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_cdc_emu_
#define _JEventProcessor_cdc_emu_

#include <JANA/JEventProcessor.h>

#include <JANA/JApplication.h>
#include <JANA/JFactory.h>

#include <stdint.h>
#include <vector>
#include <thread>

#include <string>
#include <iostream>
#include <map>
#include <set>



using namespace std;
using namespace jana;

#include "ANALYSIS/DTreeInterface.h"

#include "CDC/DCDCHit.h"
#include "CDC/DCDCDigiHit.h"
#include "DAQ/Df125WindowRawData.h"     
#include "DAQ/Df125CDCPulse.h"
#include "DAQ/Df125Config.h"
#include "DAQ/Df125TriggerTime.h"
#include "TRIGGER/DTrigger.h"
//#include "DAQ/DCODAEventInfo.h"

#include "fa125fns.h"




class JEventProcessor_cdc_emu:public jana::JEventProcessor{
	public:
		JEventProcessor_cdc_emu();
		~JEventProcessor_cdc_emu();
		const char* className(void){return "JEventProcessor_cdc_emu";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

                //TREE
                DTreeInterface* dTreeInterface;

                //thread_local: Each thread has its own object: no lock needed
                //important: manages it's own data internally: don't want to call new/delete every event!

                static thread_local DTreeFillData dTreeFillData;

        protected:

                int RUN_CONFIG; //run number to take config files from 
                int DIFFS_ONLY; //record (0) all events or (1) only events with diffs

};

#endif // _JEventProcessor_cdc_emu_

