// $Id$
//
//    File: JEventProcessor_lumi_mon.h
//    A.S.
// 
//

#ifndef _JEventProcessor_lumi_mon_
#define _JEventProcessor_lumi_mon_

#include <JANA/JEventProcessor.h>
using namespace jana;


#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include "TProfile.h"
#include <TTree.h>


class JEventProcessor_lumi_mon:public jana::JEventProcessor{
	public:
		JEventProcessor_lumi_mon(){};
		~JEventProcessor_lumi_mon(){};
		const char* className(void){return "JEventProcessor_lumi_mon";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(JEventLoop *eventLoop, int32_t  runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
		


};

#endif // _JEventProcessor_lumi_mon_

