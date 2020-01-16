// $Id$
//
//    File: DDIRCPmtHit_factory.h
//

#ifndef _DDIRCPmtHit_factory_
#define _DDIRCPmtHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactory.h>
#include "TTAB/DTranslationTable.h"
#include "DDIRCTDCDigiHit.h"
#include "DDIRCPmtHit.h"

// store constants so that they can be accessed by pixel number
typedef  vector<double>  dirc_digi_constants_t;
typedef  vector<int>  dirc_digi_constants_s;

class DDIRCPmtHit_factory:public jana::JFactory<DDIRCPmtHit>{
	public:
		DDIRCPmtHit_factory(){};
		~DDIRCPmtHit_factory(){};

		static const int DIRC_MAX_CHANNELS = 6912;  // number of active channels expected

		enum dirc_status_state {
		    GOOD,
		    BAD,
		    NOISY
		};

		// calibration constants stored in channel format
		vector<dirc_digi_constants_t> time_offsets;
		vector<dirc_digi_constants_s> channel_status;

	private:
		jerror_t init(void);						///< Called once at program start.2
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		double t_base[2];
		bool DIRC_TIME_OFFSET, DIRC_TIMEWALK;
		bool DIRC_SKIP;
};

#endif // _DDIRCPmtHit_factory_

