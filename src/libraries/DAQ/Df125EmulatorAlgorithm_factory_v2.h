// $Id$
//
//    File: Df125EmulatorAlgorithm_factory.h
// Created: Mar 20, 2016
// Creator: mstaib
//

#ifndef _Df125EmulatorAlgorithm_factory_v2_
#define _Df125EmulatorAlgorithm_factory_v2_

#include <JANA/JFactoryT.h>
#include <DAQ/Df125EmulatorAlgorithm_v2.h>

class Df125EmulatorAlgorithm_factory_v2:public JFactoryT<Df125EmulatorAlgorithm>{
	public:
		Df125EmulatorAlgorithm_factory_v2(){
			// Create single Df125EmulatorAlgorithm object and mark the factory as
			// persistent so it doesn't get deleted every event.
			SetTag("v2");
			SetFactoryFlag(PERSISTENT);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			Df125EmulatorAlgorithm *emulator = new Df125EmulatorAlgorithm_v2();
			Insert(emulator);
		};

		~Df125EmulatorAlgorithm_factory_v2(){};

};

#endif // _Df125EmulatorAlgorithm_factory_v2_

