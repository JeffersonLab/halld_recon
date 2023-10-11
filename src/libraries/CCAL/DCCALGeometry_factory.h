// $Id$
//
//    File: DCCALGeometry_factory.h
// Created: Tue Nov 30 15:42:41 EST 2010
// Creator: davidl (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DCCALGeometry_factory_
#define _DCCALGeometry_factory_

#include <JANA/JFactoryT.h>
#include "DCCALGeometry.h"

class DCCALGeometry_factory:public JFactoryT<DCCALGeometry>{
	public:
		DCCALGeometry_factory(){};
		~DCCALGeometry_factory(){};

		DCCALGeometry *ccalgeometry = nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			delete ccalgeometry;
			ccalgeometry = new DCCALGeometry();
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DBCALGeometry object.
			if( ccalgeometry ) Insert( ccalgeometry );
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun() override
		{
			delete ccalgeometry;
			ccalgeometry = nullptr;
		}
};

#endif // _DCCALGeometry_factory_

