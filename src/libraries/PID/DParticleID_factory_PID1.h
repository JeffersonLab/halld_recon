// $Id$
//
//    File: DParticleID_factory_PID1.h
// Created: Mon Feb 28 14:12:16 EST 2011
// Creator: staylor (on Linux ifarml1 2.6.18-128.el5 x86_64)
//

#ifndef _DParticleID_factory_PID1_
#define _DParticleID_factory_PID1_

#include <JANA/JFactoryT.h>
#include "DParticleID_PID1.h"

class DParticleID_factory_PID1:public JFactoryT<DParticleID>{
	public:
		DParticleID_factory_PID1(){
			SetTag("PID1");
		};
		~DParticleID_factory_PID1(){};

		DParticleID_PID1 *particleid = nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			delete particleid;
			particleid = new DParticleID_PID1(event);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DBCALGeometry object.
			if( particleid ) mData.push_back( particleid );
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun() override
		{
			delete particleid;
			particleid = nullptr;
		}

};

#endif // _DParticleID_factory_PID1_

