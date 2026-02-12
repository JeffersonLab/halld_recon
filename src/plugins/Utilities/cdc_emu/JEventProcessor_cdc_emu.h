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
#include <JANA/JFactoryT.h>

#include <stdint.h>
#include <vector>
#include <thread>

#include <string>
#include <iostream>
#include <map>
#include <set>



using namespace std;

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




class JEventProcessor_cdc_emu:public JEventProcessor{
	public:
		JEventProcessor_cdc_emu();
		~JEventProcessor_cdc_emu();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

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

