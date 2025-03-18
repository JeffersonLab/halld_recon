// $Id$
//
//    File: DTrackFitter_factory_Riemann.h

#ifndef _DTrackFitter_factory_Riemann_
#define _DTrackFitter_factory_Riemann_

#include <JANA/JFactory.h>
#include <TRACKING/DTrackFitterRiemann.h>

class DTrackFitter_factory_Riemann:public jana::JFactory<DTrackFitter>{
	public:
		DTrackFitter_factory_Riemann(){};
		~DTrackFitter_factory_Riemann(){};
		const char* Tag(void){return "Riemann";}

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

			fitter = new DTrackFitterRiemann(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DTrackFitterRiemann object.
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

#endif // _DTrackFitter_factory_Riemann_

