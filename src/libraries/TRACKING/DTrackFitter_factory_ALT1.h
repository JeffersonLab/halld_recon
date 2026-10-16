// $Id$
//
//    File: DTrackFitter_factory_ALT1.h
// Created: Mon Sep  1 10:29:51 EDT 2008
// Creator: davidl (on Darwin Amelia.local 8.11.1 i386)
//

#ifndef _DTrackFitter_factory_ALT1_
#define _DTrackFitter_factory_ALT1_

#include <JANA/JFactoryT.h>
#include <TRACKING/DTrackFitterALT1.h>

class DTrackFitter_factory_ALT1:public JFactoryT<DTrackFitter>{
	public:
		DTrackFitter_factory_ALT1(){
			SetTag("ALT1");
		};
		~DTrackFitter_factory_ALT1() override = default;

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
			fitter = new DTrackFitterALT1(event);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DTrackFitterALT1 object.
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

#endif // _DTrackFitter_factory_ALT1_

