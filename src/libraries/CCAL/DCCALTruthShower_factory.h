// $Id$
//
//    File: DCCALTruthShower_factory.h
// Created: Tue Nov 30 15:02:26 EST 2010
// Creator: davidl (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DCCALTruthShower_factory_
#define _DCCALTruthShower_factory_

#include <JANA/JFactory.h>
#include "DCCALTruthShower.h"

class DCCALTruthShower_factory:public jana::JFactory<DCCALTruthShower>{
	public:
		DCCALTruthShower_factory(){};
		~DCCALTruthShower_factory(){};
		
	private:
		jerror_t init(void);
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
		jerror_t erun(void);
		jerror_t fini(void);
};

#endif // _DCCALTruthShower_factory_

