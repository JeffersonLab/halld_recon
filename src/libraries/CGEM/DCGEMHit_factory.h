// $Id$
//
//    File: DCGEMHit_factory.h
// Created: Tue Aug  6 12:23:43 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DCGEMHit_factory_
#define _DCGEMHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactory.h>
#include "TTAB/DTranslationTable.h"
#include "DCGEMHit.h"
#include "DCGEMDigiHit.h"


class DCGEMHit_factory:public jana::JFactory<DCGEMHit>{
	public:
		DCGEMHit_factory(){};
		~DCGEMHit_factory(){};

		static const int CGEM_MAX_CHANNELS = 1000;  // number of active channels expected

	private:
		jerror_t init(void);						///< Called once at program start.2
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DCGEMHit_factory_

