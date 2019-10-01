// $Id$
//
//    File: DCDCDigiHit_factory_KO.h
// Created: Thu Sep 26 22:51:16 EDT 2019
// Creator: davidl (on Linux gluon46.jlab.org 3.10.0-957.21.3.el7.x86_64 x86_64)
//

#ifndef _DCDCDigiHit_factory_KO_
#define _DCDCDigiHit_factory_KO_

#include <JANA/JFactory.h>
#include <CDC/DCDCDigiHit.h>

class DCDCDigiHit_factory_KO:public jana::JFactory<DCDCDigiHit>{
	public:
		DCDCDigiHit_factory_KO(){};
		~DCDCDigiHit_factory_KO(){};
		const char* Tag(void){return "KO";}

		std::map< std::pair<int,int>, double > CDC_pin_eff;

		void Usage(void);
		int SetBoardEfficiency(string boardname, double eff); // returns number of pins in board

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DCDCDigiHit_factory_KO_

