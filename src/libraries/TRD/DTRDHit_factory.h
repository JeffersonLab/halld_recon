// $Id$
//
//    File: DTRDHit_factory.h
//

#ifndef _DTRDHit_factory_
#define _DTRDHit_factory_

#include <array>
#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include "TTAB/DTranslationTable.h"
#include "DTRDDigiHit.h"
#include "DTRDHit.h"

// store constants so that they can be accessed by pixel number
typedef  vector<double>  trd_digi_constants_t;

class DTRDHit_factory:public JFactoryT<DTRDHit>{
	public:
		DTRDHit_factory(){};
		~DTRDHit_factory(){};

		// overall scale factors
		double a_scale;
		double t_scale;
		array<double,2> t_base;
		
		double PEAK_THRESHOLD;
  		double LOW_TCUT;
  		double HIGH_TCUT;


		// calibration constants stored in channel format
		vector<trd_digi_constants_t> time_offsets;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		double pulse_peak_threshold;
};

#endif // _DTRDHit_factory_

