// $Id$
//
//    File: DBCALGeometry_factory.h
// Created: Thu Nov 17 15:10:51 CST 2005
// Creator: gluexuser (on Linux hydra.phys.uregina.ca 2.4.20-8smp i686)
//

#ifndef _DBCALGeometry_factory_
#define _DBCALGeometry_factory_

#include <JANA/JFactory.h>
using namespace jana;

#include <BCAL/DBCALGeometry.h>

class DBCALGeometry_factory:public JFactory<DBCALGeometry>{
	public:
		DBCALGeometry_factory(){};
		~DBCALGeometry_factory(){};

		DBCALGeometry *bcalgeometry;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( bcalgeometry ) delete bcalgeometry;

			bcalgeometry = new DBCALGeometry(runnumber);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DBCALGeometry object.
			if( bcalgeometry ) _data.push_back( bcalgeometry );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( bcalgeometry ) delete bcalgeometry;
			bcalgeometry = NULL;
			
			return NOERROR;
		}
};

#endif // _DBCALGeometry_factory_

