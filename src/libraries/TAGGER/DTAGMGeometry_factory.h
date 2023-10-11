//
// File: DTAGMGeometry_factory.h
// Created: Sat Jul 5 10:09:27 EST 2014
// Creator: jonesrt on gluey.phys.uconn.edu
//

#ifndef _DTAGMGeometry_factory_
#define _DTAGMGeometry_factory_

#include <string>

#include <JANA/JFactoryT.h>

#include "DTAGMGeometry.h"

class DTAGMGeometry_factory : public JFactoryT<DTAGMGeometry> {
	public:
		DTAGMGeometry_factory(){};
		~DTAGMGeometry_factory(){};

		DTAGMGeometry *tagmgeometry=nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& loop)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			delete tagmgeometry;
			tagmgeometry = new DTAGMGeometry(loop);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& loop)
		 {
			// Reuse existing DBCALGeometry object.
			if( tagmgeometry ) Insert( tagmgeometry );
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun()
		{
			delete tagmgeometry;
			tagmgeometry = nullptr;
		}
};
#endif // _DTAGMGeometry_factory_
