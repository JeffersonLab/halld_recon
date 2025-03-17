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

class DTRDHit_factory:public JFactoryT<DTRDHit>{
	public:
		DTRDHit_factory(){};
		~DTRDHit_factory(){};

		bool IS_XY_TIME_DIFF_CUT;
		double XY_TIME_DIFF;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		double pulse_peak_threshold;
};

#endif // _DTRDHit_factory_

