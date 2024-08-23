// $Id$
//
//    File: JEventProcessor_CDC_dedx.h
// Created: Thu Aug 16 16:35:42 EDT 2018
// Creator: njarvis (on Linux egbert 2.6.32-696.23.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_CDC_dedx_
#define _JEventProcessor_CDC_dedx_

#include <JANA/JEventProcessor.h>

#include "TRIGGER/DTrigger.h"
#include "PID/DVertex.h"

#include "TRACKING/DTrackTimeBased.h"
#include <PID/DChargedTrackHypothesis.h>
#include <PID/DChargedTrack.h>
#include <TRACKING/DTrackFitter.h>

#include <vector>

#include <TDirectory.h>

#include <TH2.h>



class JEventProcessor_CDC_dedx:public jana::JEventProcessor{
	public:
		JEventProcessor_CDC_dedx();
		~JEventProcessor_CDC_dedx();
		const char* className(void){return "JEventProcessor_CDC_dedx";}

	private:

                TH2D *bestfom_dedx_p = NULL;
                TH2D *bestfom_dedx_p_pos = NULL;
                TH2D *bestfom_dedx_p_neg = NULL;

                TH2D *dedx_p = NULL;
                TH2D *dedx_p_pos = NULL;
                TH2D *dedx_p_neg = NULL;

                TH2D *intdedx_p = NULL;
                TH2D *intdedx_p_pos = NULL;
                TH2D *intdedx_p_neg = NULL;

		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_CDC_dedx_

