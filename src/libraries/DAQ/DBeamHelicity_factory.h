// $Id$
//
//    File: DBeamHelicity_factory.h
//

#ifndef _DBeamHelicity_factory_
#define _DBeamHelicity_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>
#include "DBeamHelicity.h"

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
};

#endif // _DBeamHelicity_factory_

