// $Id$
//
//    File: Df250EmulatorAlgorithm_factory.h
// Created: Mar 20, 2016
// Creator: mstaib
//

#ifndef _Df250EmulatorAlgorithm_factory_v3_
#define _Df250EmulatorAlgorithm_factory_v3_

#include <JANA/JFactory.h>
#include <DAQ/Df250EmulatorAlgorithm_v3.h>

class Df250EmulatorAlgorithm_factory_v3:public jana::JFactory<Df250EmulatorAlgorithm>{
	public:
		Df250EmulatorAlgorithm_factory_v3(){};
		~Df250EmulatorAlgorithm_factory_v3(){};
		const char* Tag(void){return "v3";}

		Df250EmulatorAlgorithm *emulator = nullptr;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( emulator ) delete emulator;

			emulator = new Df250EmulatorAlgorithm_v3(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DBCALGeometry object.
			if( emulator ) _data.push_back( emulator );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( emulator ) delete emulator;
			emulator = NULL;
			
			return NOERROR;
		}
	
};

#endif // _Df250EmulatorAlgorithm_factory_v3_

