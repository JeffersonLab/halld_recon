
#ifndef _DLumi_factory_
#define _DLumi_factory_

#include <JANA/JFactoryT.h>


#include "DLumi.h"

class DLumi_factory:public JFactoryT<DLumi> {
	public:
		DLumi_factory(){};
		~DLumi_factory(){};

		DLumi *lumi = nullptr;

		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& loop)
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( lumi ) delete lumi;

			lumi = new DLumi(loop);
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& event)
		 {
			// Reuse existing DBCALGeometry object.
			if( lumi ) Insert( lumi );
		 }

		//------------------
		// EndRun
		//------------------
		void EndRun()
		{
			if( lumi ) delete lumi;
			lumi = NULL;
		}
};

#endif // _DLumi_factory_
