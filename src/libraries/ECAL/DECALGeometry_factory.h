// $Id$
/*
 *  File: DECALHit_factory.h
 *
 *  Created on 01/16/2024 by A.S.  
 */

#ifndef _DECALGeometry_factory_
#define _DECALGeometry_factory_

#include <JANA/JFactory.h>
#include "DECALGeometry.h"

class DECALGeometry_factory:public jana::JFactory<DECALGeometry>{
	public:
		DECALGeometry_factory(){};
		~DECALGeometry_factory(){};

		DECALGeometry *ecalgeometry = nullptr;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( ecalgeometry ) delete ecalgeometry;

			ecalgeometry = new DECALGeometry();

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DBCALGeometry object.
			if( ecalgeometry ) _data.push_back( ecalgeometry );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( ecalgeometry ) delete ecalgeometry;
			ecalgeometry = NULL;
			
			return NOERROR;
		}
};

#endif // _DECALGeometry_factory_

