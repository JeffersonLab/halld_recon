// $Id$
//
//    File: JEventProcessor_fa125_itrig.h
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_fa125_itrig_
#define _JEventProcessor_fa125_itrig_

#include <JANA/JEventProcessor.h>



class JEventProcessor_fa125_itrig:public jana::JEventProcessor{
	public:
		JEventProcessor_fa125_itrig();
		~JEventProcessor_fa125_itrig();
		const char* className(void){return "JEventProcessor_fa125_itrig";}

		//                TTree *cdctree;

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

                bool MAKE_TREE;
};

#endif // _JEventProcessor_fa125_itrig_

