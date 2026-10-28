// $Id$
//
//

#ifndef _DBeamParticle_factory_MCGEN_
#define _DBeamParticle_factory_MCGEN_

#include <JANA/JFactoryT.h>
#include <PID/DBeamParticle.h>
#include <DANA/DApplication.h>
#include "DResourcePool.h"

class DBeamParticle_factory_MCGEN:public JFactoryT<DBeamParticle>
{
	public:
		DBeamParticle_factory_MCGEN(void)
		{
	SetTag("MCGEN");
		}

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;

	private:
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void Finish() override
		{
			// OBJECT DELETION HANDLED BY JANA
		}

		double dSkipTaggedPhotons = true;    // default for KLF, fix this before merging
		double dSkipKLongBeam = false;


		//RESOURCE POOL
		vector<DBeamParticle*> dCreated;

};

#endif // _DBeamParticle_factory_MCGEN_

