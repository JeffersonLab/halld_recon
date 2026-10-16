// $Id$
//
//    File: DAnalysisUtilities_factory.h
// Created: Mon Feb 28 14:12:16 EST 2011
// Creator: pmatt (on Linux ifarml1 2.6.18-128.el5 x86_64)
//

#ifndef _DAnalysisUtilities_factory_
#define _DAnalysisUtilities_factory_

#include <JANA/JFactoryT.h>
#include "DAnalysisUtilities.h"

class DAnalysisUtilities_factory : public JFactoryT<DAnalysisUtilities>
{
	public:
		DAnalysisUtilities_factory(){analysisutilities=NULL;};
		~DAnalysisUtilities_factory(){};

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// See note in DTAGMGeometry_factory
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);

			delete analysisutilities;
			analysisutilities = new DAnalysisUtilities(event);
		}

		//------------------
		// Process
		//------------------
		void Process(const std::shared_ptr<const JEvent>& event) override
		{
 			// Reuse existing DAnalysisUtilities object.
			if( analysisutilities ) Insert( analysisutilities );
		}

		//------------------
		// EndRun
		//------------------
		void EndRun() override
		{
			if( analysisutilities ) delete analysisutilities;
			analysisutilities = NULL;
		}
	
		DAnalysisUtilities *analysisutilities;
};

#endif // _DAnalysisUtilities_factory_

