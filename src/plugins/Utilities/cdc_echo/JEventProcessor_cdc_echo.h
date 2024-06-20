// $Id$
//
//    File: JEventProcessor_cdc_echo.h
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_cdc_echo_
#define _JEventProcessor_cdc_echo_

#include <JANA/JEventProcessor.h>



class JEventProcessor_cdc_echo:public jana::JEventProcessor{
	public:
		JEventProcessor_cdc_echo();
		~JEventProcessor_cdc_echo();
		const char* className(void){return "JEventProcessor_cdc_echo";}

		//                TTree *cdctree;

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.


                int EMU; // set to 1 to run emulation if window raw data is present
                int FDC; // set to 0 to skip FDC data

		/*
  unsigned int ECHO_ORIGIN; // threshold to consider large pulses as possibly causing afterpulses (adc range 0-4095), "511" would be 4088
  unsigned int ECHO_MAX_A; // do not suppress possible afterpulses larger than this (adc range 0-4095)
  unsigned int ECHO_MAX_T;  // search up to this many samples after the main pulse for afterpulses
		*/
};

#endif // _JEventProcessor_cdc_echo_

