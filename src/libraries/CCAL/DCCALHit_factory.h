/*
 *  File: DCCALHit_factory.h
 *
 * Created on 11/25/18 by A.S. 
 */


#ifndef _DCCALHit_factory_
#define _DCCALHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include "TTAB/DTranslationTable.h"

#include "DCCALDigiHit.h"
#include "DCCALHit.h"

typedef  vector< vector<double> >  ccal_constants_t;

class DCCALHit_factory:public JFactoryT<DCCALHit>{
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
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		void LoadCCALConst( ccal_constants_t &table, 
			const vector<double> &ccal_const_ch, 
			const DCCALGeometry  &ccalGeom);    
		
		unsigned int DB_PEDESTAL;
		unsigned int HIT_DEBUG;
		
};

#endif // _DCCALHit_factory_

