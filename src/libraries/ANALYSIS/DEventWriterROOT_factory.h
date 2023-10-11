#ifndef _DEventWriterROOT_factory_
#define _DEventWriterROOT_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>

#include "ANALYSIS/DEventWriterROOT.h"

class DEventWriterROOT_factory : public JFactoryT<DEventWriterROOT>
{
	public:
		DEventWriterROOT_factory() = default;
		~DEventWriterROOT_factory() = default;
		
		DEventWriterROOT *dROOTEventWriter = nullptr;

	private:
	
		//------------------
		// BeginRun
		//------------------
		void BeginRun(const std::shared_ptr<const JEvent>& loop) override
		{
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( dROOTEventWriter == nullptr ) {
				dROOTEventWriter = new DEventWriterROOT();
				dROOTEventWriter->Initialize(loop);
			} else {
				dROOTEventWriter->Run_Update(loop);
			}
		}

		//------------------
		// Process
		//------------------
		 void Process(const std::shared_ptr<const JEvent>& loop) override
		 {
			// Reuse existing DBCALGeometry object.
			if( dROOTEventWriter ) Insert( dROOTEventWriter );
		 }


		//------------------
		// Finish
		//------------------
		void Finish() override
		{
			// Delete object: Must be "this" thread so that interfaces deleted properly
			delete dROOTEventWriter;
		}
};

#endif // _DEventWriterROOT_factory_

