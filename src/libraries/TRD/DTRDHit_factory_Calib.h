// $Id$
//
//    File: DTRDHit_factory_Calib.h
//

#ifndef _DTRDHit_factory_Calib_
#define _DTRDHit_factory_Calib_

#include <array>
#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include "TTAB/DTranslationTable.h"
#include "DTRDDigiHit.h"
#include "DTRDHit.h"

// store constants so that they can be accessed by strip number
typedef  vector<double>  trd_digi_constants_t;

class DTRDHit_factory_Calib:public JFactoryT<DTRDHit>{
	public:
		DTRDHit_factory_Calib() {  SetTag("Calib");  };
		~DTRDHit_factory_Calib(){};

        static const int k_counter_bad = 0;
        static const int k_counter_good = 1;
        static const int k_counter_noisy = 2;

		// overall scale factors
		double a_scale;
		double t_scale;
		array<double,2> t_base;
		
		double PEAK_THRESHOLD;
  		double LOW_TCUT;

		// calibration constants stored in channel format
		vector<trd_digi_constants_t> time_offsets;
		vector< vector<int>> strip_quality;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		
		bool INSTALLED;

};

#endif // _DTRDHit_factory_Calib_

