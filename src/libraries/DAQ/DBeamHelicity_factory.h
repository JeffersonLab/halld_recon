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

		bool PREFER_PROMPT_HELICITY_DATA;

};

#endif // _DBeamHelicity_factory_

