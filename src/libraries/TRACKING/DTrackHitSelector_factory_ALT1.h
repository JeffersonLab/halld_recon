// $Id$
//
//    File: DTrackHitSelector_factory_ALT1.h
// Created: Fri Feb  6 08:11:38 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DTrackHitSelector_factory_ALT1_
#define _DTrackHitSelector_factory_ALT1_

#include <JANA/JFactory.h>
#include "DTrackHitSelector.h"
#include "DTrackHitSelectorALT1.h"

class DTrackHitSelector_factory_ALT1:public jana::JFactory<DTrackHitSelector>{
	public:
		DTrackHitSelector_factory_ALT1(){};
		~DTrackHitSelector_factory_ALT1(){};
		const char* Tag(void){return "ALT1";}

		DTrackHitSelector *selector=NULL;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( selector ) delete selector;

			selector = new DTrackHitSelectorALT1(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DTrackHitSelectorALT1 object.
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

#endif // _DTrackHitSelector_factory_ALT1_

