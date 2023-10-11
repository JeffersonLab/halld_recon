// $Id$
//
//    File: Df250PulseIntegral_factory.h
// Created: Thu Feb 13 12:49:12 EST 2014
// Creator: dalton (on Linux gluon104.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#ifndef _Df250PulseIntegral_factory_
#define _Df250PulseIntegral_factory_

#include <JANA/JFactoryT.h>

#include <DAQ/Df250PulseIntegral.h>

class Df250PulseIntegral_factory:public JFactoryT<Df250PulseIntegral>{
	public:
                Df250PulseIntegral_factory(){
		  use_factory=1;
		  ped_samples=5;
		};
		~Df250PulseIntegral_factory(){};
		/* uint32_t mypulse_number; */
		/* uint32_t myquality_factor; */
		/* uint32_t myintegral; */
		/* uint32_t pedestalsum; */
		uint32_t ped_samples;
		/* uint32_t nsamples; */

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _Df250PulseIntegral_factory_

