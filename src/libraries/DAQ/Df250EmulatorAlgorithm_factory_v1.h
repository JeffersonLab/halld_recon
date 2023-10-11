// $Id$
//
//    File: Df250EmulatorAlgorithm_factory.h
// Created: Mar 20, 2016
// Creator: mstaib
//

#ifndef _Df250EmulatorAlgorithm_factory_v1_
#define _Df250EmulatorAlgorithm_factory_v1_

#include <JANA/JFactoryT.h>
#include <DAQ/Df250EmulatorAlgorithm_v1.h>

class Df250EmulatorAlgorithm_factory_v1:public JFactoryT<Df250EmulatorAlgorithm>{
	public:
		Df250EmulatorAlgorithm_factory_v1(){
			SetTag("v1");
		};
		~Df250EmulatorAlgorithm_factory_v1() = default;

	private:
		void Process(const std::shared_ptr<const JEvent>& event) override {

			// Create single Df250EmulatorAlgorithm object and mark the factory as
			// persistent so it doesn't get deleted every event.
			Df250EmulatorAlgorithm *emulator = new Df250EmulatorAlgorithm_v1(event->GetJApplication());
			SetFactoryFlag(PERSISTENT);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			Insert(emulator);
		}
};

#endif // _Df250EmulatorAlgorithm_factory_v1_

