#ifndef _DEventWriterREST_factory_
#define _DEventWriterREST_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>

#include "HDDM/DEventWriterREST.h"

class DEventWriterREST_factory : public JFactoryT<DEventWriterREST>
{
	public:
		DEventWriterREST_factory() = default;
		~DEventWriterREST_factory() = default;

	private:
		void Init() override
		{
			auto app = GetApplication();
			dOutputFileBaseName = "dana_rest";
			app->SetDefaultParameter("rest:FILENAME", dOutputFileBaseName);
			std::string locDummyString = "";
			app->SetDefaultParameter("REST:DATAVERSIONSTRING", locDummyString);
		}

		void Process(const std::shared_ptr<const JEvent>& locEvent) override
		{
			// Create single DEventWriterREST object and marks the factory as persistent so it doesn't get deleted every event.
			SetFactoryFlag(PERSISTENT);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			Insert(new DEventWriterREST(locEvent, dOutputFileBaseName));
		}
		std::string dOutputFileBaseName;
};

#endif // _DEventWriterREST_factory_

