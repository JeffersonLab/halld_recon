// $Id$
//
//    File: DTrackHitSelector_factory_ALT2.h
// Created: Wed Jan 19 08:28:53 EST 2011
// Creator: davidl (on Darwin eleanor.jlab.org 10.6.0 i386)
//

#ifndef _DTrackHitSelector_factory_ALT2_
#define _DTrackHitSelector_factory_ALT2_

#include <JANA/JFactory.h>
#include "DTrackHitSelector.h"
#include "DTrackHitSelectorALT2.h"

class DTrackHitSelector_factory_ALT2:public jana::JFactory<DTrackHitSelector>{

	public:
		DTrackHitSelector_factory_ALT2():runnumber(1){};
		~DTrackHitSelector_factory_ALT2(){};
		const char* Tag(void){return "ALT2";}

		DTrackHitSelector *selector=NULL;
		int32_t runnumber;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( selector ) delete selector;

			selector = new DTrackHitSelectorALT2(loop,runnumber);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DTrackHitSelectorALT2 object.
			if( selector ) _data.push_back( selector );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( selector ) delete selector;
			selector = NULL;
			
			return NOERROR;
		}
};

#endif // _DTrackHitSelector_factory_ALT2_

