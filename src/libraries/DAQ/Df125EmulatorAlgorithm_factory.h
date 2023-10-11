// $Id$
//
//    File: Df125EmulatorAlgorithm_factory.h
// Created: Mar 20, 2016
// Creator: mstaib
//

#ifndef _Df125EmulatorAlgorithm_factory_
#define _Df125EmulatorAlgorithm_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>

#include <DAQ/Df125EmulatorAlgorithm.h>

class Df125EmulatorAlgorithm_factory:public JFactoryT<Df125EmulatorAlgorithm>{
	public:
		Df125EmulatorAlgorithm_factory(){};
		~Df125EmulatorAlgorithm_factory(){};

		void Process(const std::shared_ptr<const JEvent>& event) override {

            // This is a trivial class that simply implements the
            // v2 tagged factory as the default. It is here so
            // that the default can be changed easily by simply
            // changing the tag here or on the command line.

            // v1 = ported f250 code (has not been implemented)
            // v2 = firmware using the upsampling technique.

            vector<Df125EmulatorAlgorithm*> emulators;
            auto f125EmV2 = event->Get<Df125EmulatorAlgorithm>("v2");
            for (auto emulator : f125EmV2) {
            	emulators.push_back(const_cast<Df125EmulatorAlgorithm*>(emulator));
            }
            Set(emulators);
            SetFactoryFlag(NOT_OBJECT_OWNER);
        }
};

#endif // _Df125EmulatorAlgorithm_factory_

