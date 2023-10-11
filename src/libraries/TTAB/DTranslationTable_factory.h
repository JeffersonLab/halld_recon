// $Id$
//
//    File: DTranslationTable_factory.h
// Created: Thu Jun 27 15:33:38 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DTranslationTable_factory_
#define _DTranslationTable_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>

#include "DTranslationTable.h"

#include <mutex>

class DTranslationTable_factory : public JFactoryT<DTranslationTable> {

	public:
		DTranslationTable_factory(){};
		~DTranslationTable_factory(){};

		DTranslationTable *tt = nullptr;


		void BeginRun(const std::shared_ptr<const JEvent> &event) override
		{
			auto runnumber = event->GetRunNumber();

			// Print message about table, but only once for each run
			static std::mutex mtx;
			mtx.lock();
			static set<int32_t> runs_announced;
			if( runs_announced.count(runnumber)==0 ){
				jout << "Creating DTranslationTable for run " << runnumber << jendl;
				runs_announced.insert(runnumber);
			}
			mtx.unlock();
		
			// (See DTAGHGeometry_factory.h)
			SetFactoryFlag(NOT_OBJECT_OWNER);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			
			if( tt ) delete tt;

			// This is a hack, needed because TranslationTable ctor is doing lots of stuff which doesn't belong
			// in a JObject. The correct solution is to put most of the TranslationTable logic in a MultiFactory. N.B.
			// TODO: Implement the correct solution.
			JEvent* unsafe_event = &(const_cast<JEvent&>(*event));
			tt = new DTranslationTable(GetApplication(), unsafe_event);
		}

		void Process(const std::shared_ptr<const JEvent>& aEvent) override {
			// Reuse existing DTranslationTable object.
			if (tt) {
				Insert(tt);
			}
		}

		void EndRun() override {
			if( tt ) delete tt;
			tt = NULL;
		}
};

#endif // _DTranslationTable_factory_

