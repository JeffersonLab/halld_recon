// $Id$
//
//    File: Df250EmulatorAlgorithm_factory.h
// Created: Mar 20, 2016
// Creator: mstaib
//

#ifndef _Df250EmulatorAlgorithm_factory_v2_
#define _Df250EmulatorAlgorithm_factory_v2_

#include <JANA/JFactoryT.h>
#include <DAQ/Df250EmulatorAlgorithm_v2.h>

class Df250EmulatorAlgorithm_factory_v2:public JFactoryT<Df250EmulatorAlgorithm>{
	public:
		Df250EmulatorAlgorithm_factory_v2(){
			SetTag("v2");
		};
		~Df250EmulatorAlgorithm_factory_v2() override = default;

		Df250EmulatorAlgorithm *emulator = nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			delete emulator;
			emulator = new Df250EmulatorAlgorithm_v2(event);
		}

		//------------------
		// Process
		//------------------
		void Process(const std::shared_ptr<const JEvent>& event) override
		{
			// Reuse existing DBCALGeometry object.
			if( emulator ) Insert( emulator );
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun() override
		{
			delete emulator;
			emulator = nullptr;
		}
};

#endif // _Df250EmulatorAlgorithm_factory_v2_

