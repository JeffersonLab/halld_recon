#ifndef _DEventWriterHDDM_factory_
#define _DEventWriterHDDM_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>

#include "HDDM/DEventWriterHDDM.h"

class DEventWriterHDDM_factory : public JFactoryT<DEventWriterHDDM>
{
	public:
		DEventWriterHDDM_factory() = default;
		~DEventWriterHDDM_factory() = default;

	private:
		void Init() override
		{
			auto app = GetApplication();
			dOutputFileBaseName = "converted";
			app->SetDefaultParameter("HDDMOUT:FILENAME", dOutputFileBaseName);
		}

		void Process(const std::shared_ptr<const JEvent>& locEvent) override
		{
			// Create single DEventWriterHDDM object and marks the factory as persistent so it doesn't get deleted every event.
			SetFactoryFlag(PERSISTENT);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			Insert(new DEventWriterHDDM(locEvent, dOutputFileBaseName));
		}

		string dOutputFileBaseName;
};

#endif // _DEventWriterHDDM_factory_

