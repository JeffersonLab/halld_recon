// $Id$
//
//    File: DTrackHitSelector_factory.h
// Created: Thu Feb  5 13:34:58 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DTrackHitSelector_factory_
#define _DTrackHitSelector_factory_

#include <JANA/JFactoryT.h>
#include "DTrackHitSelector.h"

class DTrackHitSelector_factory:public JFactoryT<DTrackHitSelector>{
	public:
		DTrackHitSelector_factory(){};
		~DTrackHitSelector_factory(){};


	private:
		void Process(const std::shared_ptr<const JEvent>& event) override {

			// This is a trivial class that simply implements the 
			// ALT1 tagged factory as the default. It is here so
			// that the default can be changed easily by simply
			// changing the tag here or on the command line.
			vector<const DTrackHitSelector*> selectors;
			event->Get(selectors, "ALT2");
			for(unsigned int i=0; i< selectors.size(); i++){
				Insert(const_cast<DTrackHitSelector*>(selectors[i]));  // TODO: Fix const_cast
			}
			SetFactoryFlag(NOT_OBJECT_OWNER);
		}
};

#endif // _DTrackHitSelector_factory_

