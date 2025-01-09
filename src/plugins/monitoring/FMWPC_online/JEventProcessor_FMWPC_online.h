// $Id$
//
//    File: JEventProcessor_FMWPC_online.h
// Created: Fri Oct 22 13:34:04 EDT 2021
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_FMWPC_online_
#define _JEventProcessor_FMWPC_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

class JEventProcessor_FMWPC_online:public JEventProcessor{
	public:
		JEventProcessor_FMWPC_online();
		~JEventProcessor_FMWPC_online();

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		double ctof_t_base_adc,ctof_t_base_tdc;
		vector<double> ctof_adc_time_offsets, ctof_tdc_time_offsets;
};

#endif // _JEventProcessor_FMWPC_online_

