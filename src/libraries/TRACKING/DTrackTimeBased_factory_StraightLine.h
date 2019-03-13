// $Id$
//
//    File: DTrackTimeBased_factory_StraightLine.h
// Created: Wed Mar 13 10:00:17 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DTrackTimeBased_factory_StraightLine_
#define _DTrackTimeBased_factory_StraightLine_

#include <JANA/JFactory.h>
#include "DTrackTimeBased.h"

class DTrackTimeBased_factory_StraightLine:public jana::JFactory<DTrackTimeBased>{
	public:
		DTrackTimeBased_factory_StraightLine(){};
		~DTrackTimeBased_factory_StraightLine(){};
		const char* Tag(void){return "StraightLine";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DTrackTimeBased_factory_StraightLine_

