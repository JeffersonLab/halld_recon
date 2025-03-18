// $Id$
//
//    File: DTrackHitSelector_factory_THROWN.h
// Created: Mon Mar  9 09:00:38 EDT 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DTrackHitSelector_factory_THROWN_
#define _DTrackHitSelector_factory_THROWN_

#include <JANA/JFactoryT.h>
#include "DTrackHitSelector.h"
#include "DTrackHitSelectorTHROWN.h"

class DTrackHitSelector_factory_THROWN:public JFactoryT<DTrackHitSelector>{
	public:
		DTrackHitSelector_factory_THROWN(){
			SetTag("THROWN");
		};
		~DTrackHitSelector_factory_THROWN() override = default;

		DTrackHitSelector *selector=nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			delete selector;
			selector = new DTrackHitSelectorTHROWN(event);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DTrackHitSelectorTHROWN object.
			if( selector ) Insert( selector );
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun() override
		{
			delete selector;
			selector = nullptr;
		}
};

#endif // _DTrackHitSelector_factory_THROWN_

