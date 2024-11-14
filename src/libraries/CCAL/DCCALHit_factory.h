/*
 *  File: DCCALHit_factory.h
 *
 * Created on 11/25/18 by A.S. 
 */


#ifndef _DCCALHit_factory_
#define _DCCALHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactory.h>
#include "TTAB/DTranslationTable.h"

#include "DCCALDigiHit.h"
#include "DCCALHit.h"

typedef  vector< vector<double> >  ccal_constants_t;

class DCCALHit_factory:public jana::JFactory<DCCALHit>{
	public:
		DCCALHit_factory();
		~DCCALHit_factory(){};
		
		ccal_constants_t  gains;
		ccal_constants_t  pedestals;		
		ccal_constants_t  time_offsets;
		ccal_constants_t  adc_offsets;
		
		double adc_en_scale;
		double adc_time_scale;
		
		double base_time;
		
	private:
		jerror_t init(void);
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
		jerror_t erun(void);
		jerror_t fini(void);
		
		void LoadCCALConst( ccal_constants_t &table, 
			const vector<double> &ccal_const_ch, 
			const DCCALGeometry  &ccalGeom);    
		
		unsigned int DB_PEDESTAL;
		unsigned int HIT_DEBUG;
		
};

#endif // _DCCALHit_factory_

