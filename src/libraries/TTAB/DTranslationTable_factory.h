// $Id$
//
//    File: DTranslationTable_factory.h
// Created: Thu Jun 27 15:33:38 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DTranslationTable_factory_
#define _DTranslationTable_factory_

#include <JANA/JFactory.h>
#include "DTranslationTable.h"

#include <mutex>

class DTranslationTable_factory:public jana::JFactory<DTranslationTable>{

	public:
		DTranslationTable_factory(){};
		~DTranslationTable_factory(){};

		DTranslationTable *tt = nullptr;

		//------------------
		// brun
		//------------------
		jerror_t brun(JEventLoop *loop, int32_t runnumber)
		{
			// Print message about table, but only once for each run
			static std::mutex mtx;
			mtx.lock();
			static set<int32_t> runs_announced;
			if( runs_announced.count(runnumber)==0 ){
				jout << "Creating DTranslationTable for run " << runnumber << endl;
				runs_announced.insert(runnumber);
			}
			mtx.unlock();
		
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( tt ) delete tt;

			tt = new DTranslationTable(loop);

			// If restricting parsing, make sure it is set for this source
			tt->SetSystemsToParse(loop->GetJEvent().GetJEventSource());

			return NOERROR;
		}

		//------------------
		// evnt
		//------------------
		 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
		 {
			// Reuse existing DTranslationTable object.
			if( tt ) _data.push_back( tt );
			 
			 return NOERROR;
		 }

		//------------------
		// erun
		//------------------
		jerror_t erun(void)
		{
			if( tt ) delete tt;
			tt = NULL;
			
			return NOERROR;
		}
};

#endif // _DTranslationTable_factory_

