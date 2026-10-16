// $Id$
//
//    File: DTrackFitter_factory_ALT1.h
// Created: Mon Sep  1 10:29:51 EDT 2008
// Creator: davidl (on Darwin Amelia.local 8.11.1 i386)
//

#ifndef _DTrackFitter_factory_
#define _DTrackFitter_factory_

#include <JANA/JFactoryT.h>
#include <TRACKING/DTrackFitter.h>

class DTrackFitter_factory:public JFactoryT<DTrackFitter>{
	public:
		DTrackFitter_factory() = default;
		~DTrackFitter_factory() override = default;

	private:
		void Process(const std::shared_ptr<const JEvent>& event) override {

			// This is a trivial class that simply implements the
			// ALT1 tagged factory as the default. It is here so 
			// that the default can be changed easily by simply
			// changing the tag here or on the command line.
			vector<const DTrackFitter*> fitters;
			event->Get(fitters, "KalmanSIMD");
			for(unsigned int i=0; i< fitters.size(); i++){
				Insert(const_cast<DTrackFitter*>(fitters[i]));
			}
			SetFactoryFlag(NOT_OBJECT_OWNER);
		}
};

#endif // _DTrackFitter_factory_

