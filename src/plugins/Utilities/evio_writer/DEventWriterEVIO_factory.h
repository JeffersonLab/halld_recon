#ifndef _DEventWriterEVIO_factory_
#define _DEventWriterEVIO_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>

#include "DEventWriterEVIO.h"

class DEventWriterEVIO_factory : public JFactoryT<DEventWriterEVIO>
{
	public:
		DEventWriterEVIO_factory() = default;
		~DEventWriterEVIO_factory() override = default;

	private:
		void Process(const std::shared_ptr<const JEvent>& locEvent, uint64_t locEventNumber)
		{
			// Create single DEventWriterEVIO object and marks the factory as persistent so it doesn't get deleted every event.
			SetFactoryFlag(PERSISTENT);
			ClearFactoryFlag(WRITE_TO_OUTPUT);
			Insert(new DEventWriterEVIO(locEvent));
		}
};

#endif // _DEventWriterEVIO_factory_

