// $Id$
//
//    File: DBeamHelicity_factory.h
//

#ifndef _DBeamHelicity_factory_
#define _DBeamHelicity_factory_

#include <JANA/JFactory.h>
#include "DBeamHelicity.h"

class DBeamHelicity_factory:public jana::JFactory<DBeamHelicity>{
	public:
		DBeamHelicity_factory(){};
		~DBeamHelicity_factory(){};

		static int   dIHWP;
		static int   dBeamOn;

		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop,  int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DBeamHelicity_factory_

