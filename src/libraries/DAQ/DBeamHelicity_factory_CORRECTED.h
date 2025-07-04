// $Id$
//
//    File: DBeamHelicity_factory_CORRECTED.h
//

#ifndef _DBeamHelicity_factory_CORRECTED_
#define _DBeamHelicity_factory_CORRECTED_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>
#include "DBeamHelicity.h"

class DBeamHelicity_factory_CORRECTED:public JFactoryT<DBeamHelicity>{
	public:
		DBeamHelicity_factory_CORRECTED(){
			SetTag("CORRECTED");
		};
		~DBeamHelicity_factory_CORRECTED(){};

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;							///< Called after last event of last event source has been processed.

	private:
		float dCorrectionFactor = 1.;
};

#endif // _DBeamHelicity_factory_CORRECTED_

