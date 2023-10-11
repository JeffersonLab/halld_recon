// $Id$
//
//    File: DTrackFitter_factory_LSLM.h
// Created: Wed Jan 14 08:59:27 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DTrackFitter_factory_LSLM_
#define _DTrackFitter_factory_LSLM_

#include <JANA/JFactoryT.h>
#include "DTrackLSFitter.h"

class DTrackFitter_factory_LSLM:public JFactoryT<DTrackFitter>{
	public:
		DTrackFitter_factory_LSLM(){};
		~DTrackFitter_factory_LSLM(){};
		SetTag("LSLM")

	private:
		void Process(const std::shared_ptr<const JEvent>& loop, uint64_t eventnumber){

			// Create single DTrackFitter object and mark the factory as
			// persistent so it doesn't get deleted every event.
			DTrackFitter *fitter = new DTrackLSFitter(loop);
			SetFactoryFlag(PERSISTENT);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			Insert(fitter);
			
			return;
		}
};

#endif // _DTrackFitter_factory_LSLM_

