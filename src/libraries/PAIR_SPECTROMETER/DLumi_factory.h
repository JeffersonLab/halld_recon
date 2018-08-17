
#ifndef _DLumi_factory_
#define _DLumi_factory_

#include <JANA/JFactory.h>
using namespace jana;

#include "DLumi.h"

class DLumi_factory:public JFactory<DLumi> {
	public:
		DLumi_factory(){};
		~DLumi_factory(){};

		DLumi *lumi;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( lumi ) delete lumi;

			lumi = new DLumi(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DBCALGeometry object.
			if( lumi ) _data.push_back( lumi );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( lumi ) delete lumi;
			lumi = NULL;
			
			return NOERROR;
		}
};

#endif // _DLumi_factory_
