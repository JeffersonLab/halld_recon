// $Id$
//
//    File: DBCALGeometry_factory.h
// Created: Thu Nov 17 15:10:51 CST 2005
// Creator: gluexuser (on Linux hydra.phys.uregina.ca 2.4.20-8smp i686)
//

#ifndef _DBCALGeometry_factory_
#define _DBCALGeometry_factory_

#include <JANA/JFactoryT.h>

#include <BCAL/DBCALGeometry.h>


class DBCALGeometry_factory:public JFactoryT<DBCALGeometry>{
public:
		DBCALGeometry_factory(){bcalgeometry=nullptr;};
		~DBCALGeometry_factory() override = default;

		DBCALGeometry *bcalgeometry = nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			delete bcalgeometry;
			bcalgeometry = new DBCALGeometry(event);
		}

		//------------------
		// Process
		//------------------
		void Process(const std::shared_ptr<const JEvent>& event) override
		{
			// Reuse existing DBCALGeometry object.
			if( bcalgeometry ) Insert( bcalgeometry );
		}

		//------------------
		// EndRun
		//------------------
		void EndRun() override
		{
			delete bcalgeometry;
			bcalgeometry = nullptr;
		}
};

#endif // _DBCALGeometry_factory_

