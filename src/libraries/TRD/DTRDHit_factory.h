// $Id$
//
//    File: DTRDHit_factory.h
//

#ifndef _DTRDHit_factory_
#define _DTRDHit_factory_

#include <array>
#include <vector>
using namespace std;

#include <JANA/JFactory.h>
#include "TTAB/DTranslationTable.h"
#include "DTRDDigiHit.h"
#include "DTRDHit.h"

// store constants so that they can be accessed by pixel number
typedef  vector<double>  trd_digi_constants_t;

class DTRDHit_factory:public jana::JFactory<DTRDHit>{
	public:
		DTRDHit_factory(){};
		~DTRDHit_factory(){};

		// overall scale factors
		double a_scale;
		double t_scale;
		array<double,2> t_base;
		
		double PEAK_THRESHOLD;

		// calibration constants stored in channel format
		vector<trd_digi_constants_t> time_offsets;

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		double pulse_peak_threshold;
};

#endif // _DTRDHit_factory_

