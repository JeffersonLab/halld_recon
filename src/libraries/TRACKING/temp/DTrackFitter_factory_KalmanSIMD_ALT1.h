// $Id$
//
//    File: DTrackFitter_factory_Kalman_SIMD_ALT1.h

#ifndef _DTrackFitter_factory_KalmanSIMD_ALT1_
#define _DTrackFitter_factory_KalmanSIMD_ALT1_

#include <JANA/JFactory.h>
#include <TRACKING/DTrackFitterKalmanSIMD_ALT1.h>

class DTrackFitter_factory_KalmanSIMD_ALT1:public jana::JFactory<DTrackFitter>{
	public:
		DTrackFitter_factory_KalmanSIMD_ALT1(){};
		~DTrackFitter_factory_KalmanSIMD_ALT1(){};
		const char* Tag(void){return "KalmanSIMD_ALT1";}

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

			fitter = new DTrackFitterKalmanSIMD_ALT1(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DTrackFitter_factory_KalmanSIMD_ALT1 object.
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

#endif // _DTrackFitter_factory_KalmanSIMD_ALT1_

