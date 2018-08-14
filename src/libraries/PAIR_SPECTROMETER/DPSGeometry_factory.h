
#ifndef _DPSGeometry_factory_
#define _DPSGeometry_factory_

#include <JANA/JFactory.h>
using namespace jana;

#include "DPSGeometry.h"

class DPSGeometry_factory:public JFactory<DPSGeometry> {
	public:
		DPSGeometry_factory(){};
		~DPSGeometry_factory(){};

		DPSGeometry *psgeometry;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( psgeometry ) delete psgeometry;

			psgeometry = new DPSGeometry(loop);

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DBCALGeometry object.
			if( psgeometry ) _data.push_back( psgeometry );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( psgeometry ) delete psgeometry;
			psgeometry = NULL;
			
			return NOERROR;
		}
};

#endif // _DPSGeometry_factory_
