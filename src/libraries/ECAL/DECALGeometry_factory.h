// $Id$
/*
 *  File: DECALHit_factory.h
 *
 *  Created on 01/16/2024 by A.S.  
 */

#ifndef _DECALGeometry_factory_
#define _DECALGeometry_factory_

#include <JANA/JFactoryT.h>
#include "DECALGeometry.h"

class DECALGeometry_factory:public JFactoryT<DECALGeometry>{
	public:
		DECALGeometry_factory(){};
		~DECALGeometry_factory(){};

		DECALGeometry *ecalgeometry = nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event)
		{
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( ecalgeometry ) delete ecalgeometry;

			ecalgeometry = new DECALGeometry();

			return; //NOERROR;
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event)
		 {
			// Reuse existing DBCALGeometry object.
			if( ecalgeometry ) Insert( ecalgeometry );
			 
			 return; //NOERROR;
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun(void)
		{
			if( ecalgeometry ) delete ecalgeometry;
			ecalgeometry = NULL;
			
			return; //NOERROR;
		}
};

#endif // _DECALGeometry_factory_

