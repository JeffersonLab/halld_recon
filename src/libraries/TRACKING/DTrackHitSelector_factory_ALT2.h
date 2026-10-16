// $Id$
//
//    File: DTrackHitSelector_factory_ALT2.h
// Created: Wed Jan 19 08:28:53 EST 2011
// Creator: davidl (on Darwin eleanor.jlab.org 10.6.0 i386)
//

#ifndef _DTrackHitSelector_factory_ALT2_
#define _DTrackHitSelector_factory_ALT2_

#include <JANA/JFactoryT.h>
#include "DTrackHitSelector.h"
#include "DTrackHitSelectorALT2.h"

class DTrackHitSelector_factory_ALT2:public JFactoryT<DTrackHitSelector>{

	public:
		DTrackHitSelector_factory_ALT2():runnumber(1){
			SetTag("ALT2");
		};
		~DTrackHitSelector_factory_ALT2() override = default;

		DTrackHitSelector *selector=nullptr;
		int32_t runnumber;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			delete selector;
			selector = new DTrackHitSelectorALT2(event);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DTrackHitSelectorALT2 object.
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

#endif // _DTrackHitSelector_factory_ALT2_

