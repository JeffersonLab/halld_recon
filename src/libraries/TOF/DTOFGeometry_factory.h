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

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>
#include "DANA/DGeometryManager.h"

#include "DTOFGeometry.h"

/*! \class DTOFGeometry_factory is a JFactory class
 * initialize the pointer tofgeometry of DTOFGeometry type
 * and in the function brun() update the content of TOF Geometry according to the current run number
 * Question here??: how to use the "tofgeometry" in a code like a plugin.

 */
class DTOFGeometry_factory:public JFactoryT<DTOFGeometry>{
	public:
		DTOFGeometry_factory(){};
		~DTOFGeometry_factory(){};

		DTOFGeometry *tofgeometry=nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);

			auto event_number = event->GetEventNumber();
			auto run_number = event->GetRunNumber();
			auto app = event->GetJApplication();
			auto geo_manager = app->GetService<DGeometryManager>();
			const DGeometry* locGeometry = geo_manager->GetDGeometry(run_number);

			if( tofgeometry ) delete tofgeometry;
			tofgeometry = new DTOFGeometry(locGeometry);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DBCALGeometry object.
			if( tofgeometry ) Insert( tofgeometry );
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun() override
		{
			delete tofgeometry;
			tofgeometry = nullptr;
		}
};

#endif // _DTOFGeometry_factory_

