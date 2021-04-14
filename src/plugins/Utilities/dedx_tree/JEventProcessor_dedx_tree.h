// $Id$
//
//    File: JEventProcessor_dedx_tree.h
// Created: Wed Aug 28 14:48:25 EDT 2019
// Creator: njarvis (on Linux albert.phys.cmu.edu 3.10.0-693.5.2.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_dedx_tree_
#define _JEventProcessor_dedx_tree_

#include <JANA/JEventProcessor.h>

#include "TRIGGER/DTrigger.h"
#include "TRACKING/DTrackTimeBased.h"
#include "PID/DVertex.h"
#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"

#include "ANALYSIS/DTreeInterface.h"

#include <vector>


class JEventProcessor_dedx_tree:public jana::JEventProcessor{
	public:
		JEventProcessor_dedx_tree();
		~JEventProcessor_dedx_tree();
		const char* className(void){return "JEventProcessor_dedx_tree";}

	private:




		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.


                //TREE
                DTreeInterface* dTreeInterface;

                //thread_local: Each thread has its own object: no lock needed
                //important: manages it's own data internally: don't want to call new/delete every event!

                static thread_local DTreeFillData dTreeFillData;


};

#endif // _JEventProcessor_dedx_tree_

