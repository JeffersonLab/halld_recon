// $Id$
//
//    File: DTrackHitSelector_factory_THROWN.h
// Created: Mon Mar  9 09:00:38 EDT 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DTrackHitSelector_factory_THROWN_
#define _DTrackHitSelector_factory_THROWN_

#include <JANA/JFactory.h>
#include "DTrackHitSelector.h"
#include "DTrackHitSelectorTHROWN.h"

class DTrackHitSelector_factory_THROWN:public jana::JFactory<DTrackHitSelector>{
	public:
		DTrackHitSelector_factory_THROWN(){};
		~DTrackHitSelector_factory_THROWN(){};
		const char* Tag(void){return "THROWN";}

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

			selector = new DTrackHitSelectorTHROWN(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DTrackHitSelectorTHROWN object.
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

#endif // _DTrackHitSelector_factory_THROWN_

