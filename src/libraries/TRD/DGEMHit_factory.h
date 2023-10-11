// $Id$
//
//    File: DGEMHit_factory.h
//

#ifndef _DGEMHit_factory_
#define _DGEMHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include "TTAB/DTranslationTable.h"
#include "DGEMDigiWindowRawData.h"
#include "DGEMHit.h"

// store constants so that they can be accessed by pixel number
typedef  vector<double>  gem_digi_constants_t;

class DGEMHit_factory:public JFactoryT<DGEMHit>{
	public:
		DGEMHit_factory(){};
		~DGEMHit_factory(){};

		// calibration constants stored in channel format
		vector<gem_digi_constants_t> time_offsets;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		double t_base[2];
		double pulse_peak_threshold;
};

#endif // _DGEMHit_factory_

