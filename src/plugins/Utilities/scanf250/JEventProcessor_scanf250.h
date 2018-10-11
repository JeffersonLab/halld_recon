// $Id$
//
//    File: JEventProcessor_scanf250.h
// Created: Tue Oct  9 21:26:13 EDT 2018
// Creator: njarvis (on Linux egbert 2.6.32-696.23.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_scanf250_
#define _JEventProcessor_scanf250_

#include <JANA/JEventProcessor.h>

#include <TTree.h>
#include "ANALYSIS/DTreeInterface.h"

class JEventProcessor_scanf250:public jana::JEventProcessor{
	public:
		JEventProcessor_scanf250();
		~JEventProcessor_scanf250();
		const char* className(void){return "JEventProcessor_scanf250";}

	private:


                //TREE
                DTreeInterface* dTreeInterface;

                //thread_local: Each thread has its own object: no lock needed
                //important: manages it's own data internally: don't want to call new/delete every event!

                static thread_local DTreeFillData dTreeFillData;





		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_scanf250_

