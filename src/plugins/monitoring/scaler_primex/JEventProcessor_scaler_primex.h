// $Id$
//
//    File: JEventProcessor_scaler_primex.h
//    A.S.
// 
//

#ifndef _JEventProcessor_scaler_primex_
#define _JEventProcessor_scaler_primex_

#include <JANA/JEventProcessor.h>

using namespace jana;


#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include "TProfile.h"
#include <TTree.h>


#include "ANALYSIS/DTreeInterface.h"

class JEventProcessor_scaler_primex:public jana::JEventProcessor{
	public:
		JEventProcessor_scaler_primex(){};
		~JEventProcessor_scaler_primex(){};
		const char* className(void){return "JEventProcessor_scaler_primex";}

		//		enum { khit = 32 };

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(JEventLoop *eventLoop, int32_t  runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
		

		//TREE
                DTreeInterface* dTreeInterface;
                //thread_local: Each thread has its own object: no lock needed
		//important: manages it's own data internally: don't want to call new/delete every event!

                static thread_local DTreeFillData dTreeFillData;



};

#endif // _JEventProcessor_scaler_primex_

