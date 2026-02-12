// $Id$
//
//    File: DBeamKLong_factory_MCGEN.h
// Created: Mon Aug  5 14:29:24 EST 2014
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DBeamKLong_factory_MCGEN_
#define _DBeamKLong_factory_MCGEN_

#include <JANA/JFactoryT.h>
#include <PID/DBeamKLong.h>
#include <PID/DMCReaction.h>
#include "DANA/DStatusBits.h"

class DBeamKLong_factory_MCGEN : public JFactoryT<DBeamKLong> {
	public:
	   DBeamKLong_factory_MCGEN(void);
 		//~DBeamKLong_factory_MCGEN(void) {}


	private:
// 		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override; 
		void Process(const std::shared_ptr<const JEvent>& event) override; 
// 		void EndRun() override; 
// 		void Finish() override; 
};


#endif // _DBeamPhoton_factory_MCGEN_

