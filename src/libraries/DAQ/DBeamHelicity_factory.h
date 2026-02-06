// $Id$
//
//    File: DBeamHelicity_factory.h
//

#ifndef _DBeamHelicity_factory_
#define _DBeamHelicity_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>
#include "DBeamHelicity.h"

#include <DAQ/DHELIDigiHit.h>
#include <DAQ/DHelicityData.h>

class DBeamHelicity_factory:public JFactoryT<DBeamHelicity>{
	public:
		DBeamHelicity_factory(){};
		~DBeamHelicity_factory(){};

		static int   dIHWP;
		static int   dBeamOn;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;							///< Called after last event of last event source has been processed.


	private:
		DBeamHelicity *Make_DBeamHelicity(vector<const DHELIDigiHit*> &locHELIDigiHits);
		DBeamHelicity *Make_DBeamHelicity(const DHelicityData *locHelicityData);

		uint32_t advanceSeed(uint32_t seed) const;
		bool checkPredictor(uint32_t testval) const;
		void reportPredictorError(uint32_t testval) const;
		uint32_t helicityDecoderCalcPolarity(uint32_t event_polarity, uint32_t seed, uint32_t delay);

		bool PREFER_PROMPT_HELICITY_DATA;
		uint32_t dHDBoardDelay;
};

#endif // _DBeamHelicity_factory_

