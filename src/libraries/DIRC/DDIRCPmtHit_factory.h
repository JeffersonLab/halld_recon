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
//#include "DDIRCDigiHit.h"
#include "DDIRCPmtHit.h"

// store constants so that they can be accessed by pixel number
typedef  vector<double>  dirc_digi_constants_t;

class DDIRCPmtHit_factory:public jana::JFactory<DDIRCPmtHit>{
	public:
		DDIRCPmtHit_factory(){};
		~DDIRCPmtHit_factory(){};

		static const int DIRC_MAX_CHANNELS = 13824;  // number of active channels expected

		enum dirc_quality_state {
		    GOOD,
		    BAD,
		    NOISY
		};

		// overall scale factors
		double t_scale;
                double t_base;

		// calibration constants stored in row, column format
		dirc_digi_constants_t time_offsets;
		dirc_digi_constants_t block_qualities;

		//const double GetConstant(const dirc_digi_constants_t &the_table,
		//			 const int in_row, const int in_column) const;
		//const double GetConstant(const dirc_digi_constants_t &the_table,
		//			 const DDIRCDigiHit *the_digihit) const;
		//const double GetConstant(const dirc_digi_constants_t &the_table,
		//			 const DDIRCPmtHit *the_hit) const;
		//const double GetConstant(const fcal_digi_constants_t &the_table,
		//			 const DTranslationTable *ttab,
		//			 const int in_rocid, const int in_slot, const int in_channel) const;
	private:
		jerror_t init(void);						///< Called once at program start.2
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		//void FillCalibTable( fcal_digi_constants_t &table, 
		//		     const vector<double> &raw_table, 
		//		     const DDIRCGeometry &dircGeom);

};

#endif // _DDIRCPmtHit_factory_

