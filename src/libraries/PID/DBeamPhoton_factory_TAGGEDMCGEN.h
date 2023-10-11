// $Id$
//
//    File: DBeamPhoton_factory_TAGGEDMCGEN.h
// Created: Mon Aug  5 14:29:24 EST 2014
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DBeamPhoton_factory_TAGGEDMCGEN_
#define _DBeamPhoton_factory_TAGGEDMCGEN_

#include <JANA/JFactoryT.h>
#include <PID/DBeamPhoton.h>
#include "PID/DMCReaction.h"
#include "DANA/DStatusBits.h"

class DBeamPhoton_factory_TAGGEDMCGEN:public JFactoryT<DBeamPhoton>{
	public:
		DBeamPhoton_factory_TAGGEDMCGEN() {
			SetTag("TAGGEDMCGEN");
		}

	private:

		void Process(const std::shared_ptr<const JEvent>& event) override;
};

#endif // _DBeamPhoton_factory_TAGGEDMCGEN_

