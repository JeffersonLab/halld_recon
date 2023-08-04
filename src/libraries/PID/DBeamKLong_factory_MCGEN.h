// $Id$
//
//    File: DBeamKLong_factory_MCGEN.h
// Created: Mon Aug  5 14:29:24 EST 2014
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DBeamKLong_factory_MCGEN_
#define _DBeamKLong_factory_MCGEN_

#include <JANA/JFactory.h>
#include <PID/DBeamKLong.h>
#include <PID/DMCReaction.h>
#include "DANA/DStatusBits.h"

class DBeamKLong_factory_MCGEN:public jana::JFactory<DBeamKLong>{
	public:
		const char* Tag(void){return "MCGEN";}

	private:
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DBeamPhoton_factory_MCGEN_

