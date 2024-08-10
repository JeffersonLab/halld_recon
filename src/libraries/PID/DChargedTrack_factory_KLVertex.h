// $Id$
//
//    File: DChargedTrack_factory_KLVertex.h
//

#ifndef _DChargedTrack_factory_KLVertex_
#define _DChargedTrack_factory_KLVertex_

#include <JANA/JFactory.h>
#include <PID/DChargedTrack.h>
#include <TRACKING/DTrackTimeBased.h>

class DChargedTrack_factory_KLVertex:public jana::JFactory<DChargedTrack>{
	public:
		DChargedTrack_factory_KLVertex(){};
		~DChargedTrack_factory_KLVertex(){};

		const char* Tag(void){return "KLVertex";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DChargedTrack_factory_KLVertex_

