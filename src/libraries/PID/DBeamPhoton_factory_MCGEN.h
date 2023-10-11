// $Id$
//
//    File: DBeamPhoton_factory_MCGEN.h
// Created: Mon Aug  5 14:29:24 EST 2014
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DBeamPhoton_factory_MCGEN_
#define _DBeamPhoton_factory_MCGEN_

#include <JANA/JFactoryT.h>
#include <PID/DBeamPhoton.h>
#include <PID/DMCReaction.h>
#include "DANA/DStatusBits.h"
#include "TAGGER/DTAGHHit.h"
#include "TAGGER/DTAGMHit.h"

class DBeamPhoton_factory_MCGEN:public JFactoryT<DBeamPhoton>{
	public:
		DBeamPhoton_factory_MCGEN() {
			SetTag("MCGEN");
		}

	private:
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void Finish() override;
};

#endif // _DBeamPhoton_factory_MCGEN_

