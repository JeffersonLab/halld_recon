// $Id$
//
//    File: DECALTruthShower_factory.h
//

#ifndef _DECALTruthShower_factory_
#define _DECALTruthShower_factory_

#include <JANA/JFactory.h>
#include "DECALTruthShower.h"

class DECALTruthShower_factory:public jana::JFactory<DECALTruthShower>{
	public:
		DECALTruthShower_factory(){};
		~DECALTruthShower_factory(){};


	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DECALTruthShower_factory_

