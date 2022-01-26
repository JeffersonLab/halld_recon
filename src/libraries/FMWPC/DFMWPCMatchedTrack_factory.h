// $Id$
//
//    File: DFMWPCMatchedTrack_factory.h
// Created: Sat Jan 22 08:53:49 EST 2022
// Creator: davidl (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DFMWPCMatchedTrack_factory_
#define _DFMWPCMatchedTrack_factory_

#include <JANA/JFactory.h>
#include "DFMWPCMatchedTrack.h"

class DFMWPCMatchedTrack_factory:public jana::JFactory<DFMWPCMatchedTrack>{
	public:
		DFMWPCMatchedTrack_factory(){};
		~DFMWPCMatchedTrack_factory(){};
        
        double MIN_DELTA_T_FCAL_PROJECTION;  // min. time between track projection and FCAL hit to consider them matched
        double MIN_DELTA_T_FMWPC_PROJECTION; // min. time between track projection and FMWPC hit to consider them matched
        double FMWPC_WIRE_SPACING;           // distance between wires in FMWPC in cm

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DFMWPCMatchedTrack_factory_

