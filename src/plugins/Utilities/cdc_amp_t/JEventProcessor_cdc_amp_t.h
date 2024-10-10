// $Id$
//
//    File: JEventProcessor_cdc_amp_t.h
// Created: Wed Aug 14 12:10:19 PM EDT 2024
// Creator: njarvis (on Linux gluon01 5.14.0-427.22.1.el9_4.x86_64 x86_64)
//

#ifndef _JEventProcessor_cdc_amp_t_
#define _JEventProcessor_cdc_amp_t_

#include <JANA/JEventProcessor.h>
#include <TH2I.h>

class JEventProcessor_cdc_amp_t:public jana::JEventProcessor{
	public:
		JEventProcessor_cdc_amp_t();
		~JEventProcessor_cdc_amp_t();
		const char* className(void){return "JEventProcessor_cdc_amp_t";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
 
                TH2I *amp_t = NULL;
                TH2I *amp_tt = NULL;
                TH2I *hitamp_t = NULL;
                TH2I *hitamp_tt = NULL;
};

#endif // _JEventProcessor_cdc_amp_t_

