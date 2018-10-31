// $Id$
//
//    File: DAnalysisUtilities_factory.h
// Created: Mon Feb 28 14:12:16 EST 2011
// Creator: pmatt (on Linux ifarml1 2.6.18-128.el5 x86_64)
//

#ifndef _DAnalysisUtilities_factory_
#define _DAnalysisUtilities_factory_

#include <JANA/JFactory.h>
#include "DAnalysisUtilities.h"

class DAnalysisUtilities_factory : public jana::JFactory<DAnalysisUtilities>
{
	public:
		DAnalysisUtilities_factory(){};
		~DAnalysisUtilities_factory(){};

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// See note in DTAGMGeometry_factory
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);

			if( analysisutilities ) delete analysisutilities;

			analysisutilities = new DAnalysisUtilities(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		{
 			// Reuse existing DAnalysisUtilities object.
   		if( analysisutilities ) _data.push_back( analysisutilities );

			return NOERROR;
		}

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( analysisutilities ) delete analysisutilities;
			analysisutilities = NULL;

   		return NOERROR;
		}
	
		DAnalysisUtilities *analysisutilities;
};

#endif // _DAnalysisUtilities_factory_

