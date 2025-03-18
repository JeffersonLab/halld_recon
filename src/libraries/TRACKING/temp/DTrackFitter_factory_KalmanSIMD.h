// $Id$
//
//    File: DTrackFitter_factory_Kalman_SIMD.h

#ifndef _DTrackFitter_factory_KalmanSIMD_
#define _DTrackFitter_factory_KalmanSIMD_

#include <JANA/JFactory.h>
#include <TRACKING/DTrackFitterKalmanSIMD.h>

class DTrackFitter_factory_KalmanSIMD:public jana::JFactory<DTrackFitter>{
	public:
		DTrackFitter_factory_KalmanSIMD(){};
		~DTrackFitter_factory_KalmanSIMD(){};
		const char* Tag(void){return "KalmanSIMD";}

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

			fitter = new DTrackFitterKalmanSIMD(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DTrackFitterKalmanSIMD object.
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

#endif // _DTrackFitter_factory_KalmanSIMD_

