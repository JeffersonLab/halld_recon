// $Id$
//
//    File: DTrackFitter_factory_ALT1.h
// Created: Mon Sep  1 10:29:51 EDT 2008
// Creator: davidl (on Darwin Amelia.local 8.11.1 i386)
//

#ifndef _DTrackFitter_factory_ALT1_
#define _DTrackFitter_factory_ALT1_

#include <JANA/JFactory.h>
#include <TRACKING/DTrackFitterALT1.h>

class DTrackFitter_factory_ALT1:public jana::JFactory<DTrackFitter>{
	public:
		DTrackFitter_factory_ALT1(){};
		~DTrackFitter_factory_ALT1(){};
		const char* Tag(void){return "ALT1";}

		DTrackFitter *fitter=NULL;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( fitter ) delete fitter;

			fitter = new DTrackFitterALT1(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DTrackFitterALT1 object.
			if( fitter ) _data.push_back( fitter );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( fitter ) delete fitter;
			fitter = NULL;
			
			return NOERROR;
		}
};

#endif // _DTrackFitter_factory_ALT1_

