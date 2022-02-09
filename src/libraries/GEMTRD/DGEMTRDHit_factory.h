// $Id$
//
//    File: DGEMTRDHit_factory.h
// Created: Mon Feb  7 16:39:32 EST 2022
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DGEMTRDHit_factory_
#define _DGEMTRDHit_factory_

#include <JANA/JFactory.h>
#include "DGEMTRDHit.h"

class DGEMTRDHit_factory:public jana::JFactory<DGEMTRDHit>{
	public:
		DGEMTRDHit_factory(){};
		~DGEMTRDHit_factory(){};


	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DGEMTRDHit_factory_

