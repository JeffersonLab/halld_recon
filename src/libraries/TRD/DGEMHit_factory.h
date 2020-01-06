// $Id$
//
//    File: DGEMHit_factory.h
//

#ifndef _DGEMHit_factory_
#define _DGEMHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactory.h>
#include "TTAB/DTranslationTable.h"
#include "DGEMDigiWindowRawData.h"
#include "DGEMHit.h"

// store constants so that they can be accessed by pixel number
typedef  vector<double>  gem_digi_constants_t;

class DGEMHit_factory:public jana::JFactory<DGEMHit>{
	public:
		DGEMHit_factory(){};
		~DGEMHit_factory(){};

		// calibration constants stored in channel format
		vector<gem_digi_constants_t> time_offsets;

	private:
		jerror_t init(void);						///< Called once at program start.2
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		double t_base[2];
		double pulse_peak_threshold;
};

#endif // _DGEMHit_factory_

