//
// File: DTAGMGeometry_factory.h
// Created: Sat Jul 5 10:09:27 EST 2014
// Creator: jonesrt on gluey.phys.uconn.edu
//

#ifndef _DTAGMGeometry_factory_
#define _DTAGMGeometry_factory_

#include <string>

#include "JANA/JFactory.h"
#include "DTAGMGeometry.h"

class DTAGMGeometry_factory : public JFactory<DTAGMGeometry> {
	public:
		DTAGMGeometry_factory(){};
		~DTAGMGeometry_factory(){};

		DTAGMGeometry *tagmgeometry=nullptr;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( tagmgeometry ) delete tagmgeometry;

			tagmgeometry = new DTAGMGeometry(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DBCALGeometry object.
			if( tagmgeometry ) _data.push_back( tagmgeometry );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( tagmgeometry ) delete tagmgeometry;
			tagmgeometry = NULL;
			
			return NOERROR;
		}
};
#endif // _DTAGMGeometry_factory_
