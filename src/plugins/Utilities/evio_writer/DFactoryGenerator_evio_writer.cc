#include <JANA/JApplication.h>
#include "DFactoryGenerator_evio_writer.h"

extern "C"
{
	void InitPlugin(JApplication *app)
	{
		InitJANAPlugin(app);
		app->Add(new DFactoryGenerator_evio_writer());
	}
} // "C"

