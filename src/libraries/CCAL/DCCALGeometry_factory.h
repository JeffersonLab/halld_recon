// $Id$
//
//    File: DCCALGeometry_factory.h
// Created: Tue Nov 30 15:42:41 EST 2010
// Creator: davidl (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DCCALGeometry_factory_
#define _DCCALGeometry_factory_

#include <JANA/JFactory.h>
#include "DCCALGeometry.h"

class DCCALGeometry_factory:public jana::JFactory<DCCALGeometry>{
	public:
		DCCALGeometry_factory(){};
		~DCCALGeometry_factory(){};

		DCCALGeometry *ccalgeometry = nullptr;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( ccalgeometry ) delete ccalgeometry;

			ccalgeometry = new DCCALGeometry();

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		{
			// Reuse existing DBCALGeometry object.
			if( ccalgeometry ) _data.push_back( ccalgeometry );
			 
			return NOERROR;
		}

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( ccalgeometry ) delete ccalgeometry;
			ccalgeometry = NULL;
			
			return NOERROR;
		}
};

#endif // _DCCALGeometry_factory_

