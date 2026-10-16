// $Id$
//
//    File: DNeutralParticle_factory.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DNeutralParticle_factory_
#define _DNeutralParticle_factory_

#include <JANA/JFactoryT.h>
#include <PID/DNeutralParticle.h>
#include <PID/DNeutralShower.h>

class DNeutralParticle_factory:public JFactoryT<DNeutralParticle>{
	public:
		DNeutralParticle_factory(){};
		~DNeutralParticle_factory(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DNeutralParticle_factory_

