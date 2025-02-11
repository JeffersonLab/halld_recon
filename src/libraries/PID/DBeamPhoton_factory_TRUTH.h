// $Id$
//
//    File: DBeamPhoton_factory_TRUTH.h
// Created: Mon Aug  5 14:29:24 EST 2014
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DBeamPhoton_factory_TRUTH_
#define _DBeamPhoton_factory_TRUTH_

#include <JANA/JFactoryT.h>
#include <PID/DBeamPhoton.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGHHit.h>

class DBeamPhoton_factory_TRUTH:public JFactoryT<DBeamPhoton>{
	public:
		DBeamPhoton_factory_TRUTH(){
			SetTag("TRUTH");
		};
		~DBeamPhoton_factory_TRUTH(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		double dTargetCenterZ;
};

#endif // _DBeamPhoton_factory_TRUTH_

