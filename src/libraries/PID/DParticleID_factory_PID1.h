// $Id$
//
//    File: DParticleID_factory_PID1.h
// Created: Mon Feb 28 14:12:16 EST 2011
// Creator: staylor (on Linux ifarml1 2.6.18-128.el5 x86_64)
//

#ifndef _DParticleID_factory_PID1_
#define _DParticleID_factory_PID1_

#include <JANA/JFactory.h>
#include "DParticleID_PID1.h"

class DParticleID_factory_PID1:public jana::JFactory<DParticleID>{
	public:
		DParticleID_factory_PID1(){};
		~DParticleID_factory_PID1(){};
		const char* Tag(void){return "PID1";}

		DParticleID_PID1 *particleid;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( particleid ) delete particleid;

			particleid = new DParticleID_PID1(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DBCALGeometry object.
			if( particleid ) _data.push_back( particleid );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( particleid ) delete particleid;
			particleid = NULL;
			
			return NOERROR;
		}

};

#endif // _DParticleID_factory_PID1_

