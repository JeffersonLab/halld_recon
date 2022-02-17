// $Id$
//
//    File: DTOFGeometry_factory.h
// Created: Mon Jul 18 11:43:31 EST 2005
// Creator: remitche (on Linux mantrid00 2.4.20-18.8 i686)
//
/*! \file DTOFGeometry_factory.h defines the factory to update the TOF geometry any time the
 * run number will change.  
 */


#ifndef _DTOFGeometry_factory_
#define _DTOFGeometry_factory_

#include <JANA/JFactory.h>
using namespace jana;

#include "DTOFGeometry.h"

/*! \class DTOFGeometry_factory is a JFactory class
 * initialize the pointer tofgeometry of DTOFGeometry type
 * and in the function brun() update the content of TOF Geometry according to the current run number
 * Question here??: how to use the "tofgeometry" in a code like a plugin.

 */
class DTOFGeometry_factory:public JFactory<DTOFGeometry>{
	public:
		DTOFGeometry_factory(){};
		~DTOFGeometry_factory(){};

		DTOFGeometry *tofgeometry=nullptr;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( tofgeometry ) delete tofgeometry;

			// Get the geometry
			DApplication* dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
			const DGeometry* locGeometry = dapp->GetDGeometry(runnumber);
			tofgeometry = new DTOFGeometry(locGeometry);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DBCALGeometry object.
			if( tofgeometry ) _data.push_back( tofgeometry );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( tofgeometry ) delete tofgeometry;
			tofgeometry = NULL;
			
			return NOERROR;
		}
};

#endif // _DTOFGeometry_factory_

