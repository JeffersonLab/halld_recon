// $Id$
//
//    File: JEventProcessor_cdc_scan.h
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_cdc_scan_
#define _JEventProcessor_cdc_scan_

#include <JANA/JEventProcessor.h>



class JEventProcessor_cdc_scan:public jana::JEventProcessor{
	public:
		JEventProcessor_cdc_scan();
		~JEventProcessor_cdc_scan();
		const char* className(void){return "JEventProcessor_cdc_scan";}

		//                TTree *cdctree;

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.


                int SHORT_MODE; // set to 0 to include window raw data

};

#endif // _JEventProcessor_cdc_scan_

