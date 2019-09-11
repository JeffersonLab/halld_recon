#ifndef _DEventWriterROOT_factory_
#define _DEventWriterROOT_factory_

#include <JANA/JFactory.h>
#include <JANA/JEventLoop.h>

#include "ANALYSIS/DEventWriterROOT.h"

class DEventWriterROOT_factory : public jana::JFactory<DEventWriterROOT>
{
	public:
		DEventWriterROOT_factory(){use_factory = 1;}; //prevents JANA from searching the input file for these objects
		~DEventWriterROOT_factory(){};
		
		DEventWriterROOT *dROOTEventWriter = nullptr;

	private:
	
		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
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

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DBCALGeometry object.
			if( dROOTEventWriter ) _data.push_back( dROOTEventWriter );
			 
			return NOERROR;
		 }


		//------------------
		// fini
		//------------------
		jerror_t fini(void)
		{
			// Delete object: Must be "this" thread so that interfaces deleted properly
			delete dROOTEventWriter;
			return NOERROR;
		}
};

#endif // _DEventWriterROOT_factory_

