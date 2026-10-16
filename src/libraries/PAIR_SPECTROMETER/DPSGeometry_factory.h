
#ifndef _DPSGeometry_factory_
#define _DPSGeometry_factory_

#include <JANA/JFactoryT.h>


#include "DPSGeometry.h"

class DPSGeometry_factory:public JFactoryT<DPSGeometry> {
	public:
		DPSGeometry_factory(){psgeometry=nullptr;};
		~DPSGeometry_factory(){};

		DPSGeometry *psgeometry = nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& event) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			delete psgeometry;
			psgeometry = new DPSGeometry(event);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event) override
		 {
			// Reuse existing DBCALGeometry object.
			if( psgeometry ) Insert( psgeometry );
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun() override
		{
			delete psgeometry;
			psgeometry = nullptr;
		}
};

#endif // _DPSGeometry_factory_
