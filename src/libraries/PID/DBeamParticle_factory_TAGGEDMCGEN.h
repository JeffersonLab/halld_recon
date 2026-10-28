// $Id$
//
//

#ifndef _DBeamParticle_factory_TAGGEDMCGEN_
#define _DBeamParticle_factory_TAGGEDMCGEN_

#include <JANA/JFactoryT.h>
#include <PID/DBeamParticle.h>
#include <DANA/DApplication.h>
#include "DResourcePool.h"

class DBeamParticle_factory_TAGGEDMCGEN:public JFactoryT<DBeamParticle>
{
	public:
		DBeamParticle_factory_TAGGEDMCGEN(void)
		{
			SetTag("TAGGEDMCGEN");
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

#endif // _DBeamParticle_factory_TAGGEDMCGEN_

