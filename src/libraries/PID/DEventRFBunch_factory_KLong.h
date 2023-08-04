// $Id$
//
//    File: DEventRFBunch_factory_KLong.h
//

#ifndef _DEventRFBunch_factory_KLong_
#define _DEventRFBunch_factory_KLong_

#include <iostream>
#include <iomanip>

#include <JANA/JFactory.h>
#include <PID/DEventRFBunch.h>
#include <PID/DBeamPhoton.h>
#include <RF/DRFTime.h>
#include <PID/DVertex.h>

using namespace std;
using namespace jana;

class DEventRFBunch_factory_KLong : public jana::JFactory<DEventRFBunch>
{
	public:
		DEventRFBunch_factory_KLong(){};
		~DEventRFBunch_factory_KLong(){};
		const char* Tag(void){return "KLong";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DEventRFBunch_factory_KLong_

