// $Id$
//
//    File: DFactory_DTrack.h
// Created: Sun Apr  3 12:28:45 EDT 2005
// Creator: davidl (on Darwin Harriet.local 7.8.0 powerpc)
//

#ifndef _DFactory_DTrack_
#define _DFactory_DTrack_

#include "DFactory.h"
#include "DTrack.h"
#include "DMagneticFieldMap.h"
#include "DGeometry.h"


class DFactory_DTrack:public DFactory<DTrack>{
	public:
		DFactory_DTrack(){};
		~DFactory_DTrack(){};
		const string toString(void);
	
	private:
		derror_t init(void);
		derror_t brun(DEventLoop *loop, int runnumber);
		derror_t evnt(DEventLoop *eventLoop, int eventnumber);	///< Invoked via DEventProcessor virtual method

		const DGeometry *dgeom;
		const DMagneticFieldMap *bfield;
};

#endif // _DFactory_DTrack_

