/*
 *  File: DECALHit_factory.h
 *
 *  Created on 01/16/2024 by A.S.  
 */


#ifndef _DECALHit_factory_
#define _DECALHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include "TTAB/DTranslationTable.h"

#include "DECALDigiHit.h"
#include "DECALHit.h"

typedef  vector< vector<double> >  ecal_constants_t;

class DECALHit_factory:public JFactoryT<DECALHit>{
	public:
                DECALHit_factory();
		~DECALHit_factory(){};

		ecal_constants_t  gains;
		ecal_constants_t  pedestals;		
		ecal_constants_t  time_offsets;
		ecal_constants_t  adc_offsets;

		double adc_en_scale;
		double adc_time_scale;
		
		double base_time;

        private:
			void Init() override;
			void BeginRun(const std::shared_ptr<const JEvent>& event) override;
			void Process(const std::shared_ptr<const JEvent>& event) override;
			void EndRun() override;
			void Finish() override;

		void LoadECALConst( ecal_constants_t &table, 
                                    const vector<double> &ecal_const_ch, 
                                    const DECALGeometry  &ecalGeom);    


		unsigned int DB_PEDESTAL;
		unsigned int HIT_DEBUG;

};

#endif // _DECALHit_factory_

