// $Id$
//
//    File: DTrackFitter_factory_Riemann.h

#ifndef _DTrackFitter_factory_Riemann_
#define _DTrackFitter_factory_Riemann_

#include <JANA/JFactoryT.h>
#include <TRACKING/DTrackFitterRiemann.h>

class DTrackFitter_factory_Riemann:public JFactoryT<DTrackFitter>{
	public:
		DTrackFitter_factory_Riemann(){
			SetTag("Riemann");
		};
		~DTrackFitter_factory_Riemann() override = default;

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
			fitter = new DTrackFitterRiemann(event);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DTrackFitterRiemann object.
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

#endif // _DTrackFitter_factory_Riemann_

