// $Id$
//
//    File: DTrackFitter_factory_Kalman_SIMD_ALT1.h

#ifndef _DTrackFitter_factory_KalmanSIMD_ALT1_
#define _DTrackFitter_factory_KalmanSIMD_ALT1_

#include <JANA/JFactoryT.h>
#include <TRACKING/DTrackFitterKalmanSIMD_ALT1.h>

class DTrackFitter_factory_KalmanSIMD_ALT1:public JFactoryT<DTrackFitter>{
	public:
		DTrackFitter_factory_KalmanSIMD_ALT1() {
			SetTag("KalmanSIMD_ALT1");
		}
		~DTrackFitter_factory_KalmanSIMD_ALT1() override = default;

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
			fitter = new DTrackFitterKalmanSIMD_ALT1(event);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DTrackFitter_factory_KalmanSIMD_ALT1 object.
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

#endif // _DTrackFitter_factory_KalmanSIMD_ALT1_

