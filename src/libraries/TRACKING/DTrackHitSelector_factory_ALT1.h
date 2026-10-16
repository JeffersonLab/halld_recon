// $Id$
//
//    File: DTrackHitSelector_factory_ALT1.h
// Created: Fri Feb  6 08:11:38 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DTrackHitSelector_factory_ALT1_
#define _DTrackHitSelector_factory_ALT1_

#include <JANA/JFactoryT.h>
#include "DTrackHitSelector.h"
#include "DTrackHitSelectorALT1.h"

class DTrackHitSelector_factory_ALT1:public JFactoryT<DTrackHitSelector>{
	public:
		DTrackHitSelector_factory_ALT1(){
			SetTag("ALT1");
		};
		~DTrackHitSelector_factory_ALT1() override = default;

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
			selector = new DTrackHitSelectorALT1(event);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& loop) override
		 {
			// Reuse existing DTrackHitSelectorALT1 object.
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

#endif // _DTrackHitSelector_factory_ALT1_

