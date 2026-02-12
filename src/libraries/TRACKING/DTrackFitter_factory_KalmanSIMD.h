// $Id$
//
//    File: DTrackFitter_factory_Kalman_SIMD.h

#ifndef _DTrackFitter_factory_KalmanSIMD_
#define _DTrackFitter_factory_KalmanSIMD_

#include <JANA/JFactoryT.h>
#include <TRACKING/DTrackFitterKalmanSIMD.h>

class DTrackFitter_factory_KalmanSIMD:public JFactoryT<DTrackFitter>{
	public:
		DTrackFitter_factory_KalmanSIMD(){
			SetTag("KalmanSIMD");
		};
		~DTrackFitter_factory_KalmanSIMD() = default;

		DTrackFitter *fitter=nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			delete fitter;
			fitter = new DTrackFitterKalmanSIMD(event);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DTrackFitterKalmanSIMD object.
			if( fitter ) Insert( fitter );
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun() override
		{
			delete fitter;
			fitter = nullptr;
		}
};

#endif // _DTrackFitter_factory_KalmanSIMD_

