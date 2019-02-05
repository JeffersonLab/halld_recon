// $Id$
//
//    File: DEventWriterROOT_factory_kFitPullStudies.h
// Created: Sa 14. Apr 18:30:24 EDT 2018
// Creator: dlersch (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DEventWriterROOT_factory_kFitPullStudies_
#define _DEventWriterROOT_factory_kFitPullStudies_

#include <JANA/JFactory.h>
#include <JANA/JEventLoop.h>

#include "DEventWriterROOT_kFitPullStudies.h"

class DEventWriterROOT_factory_kFitPullStudies : public jana::JFactory<DEventWriterROOT>
{
	public:
		DEventWriterROOT_factory_kFitPullStudies(){use_factory = 1;}; //prevents JANA from searching the input file for these objects
		~DEventWriterROOT_factory_kFitPullStudies(){};
		const char* Tag(void){return "kFitPullStudies";}

	private:
		jerror_t brun(jana::JEventLoop *locEventLoop, int locRunNumber)
		{
			// Create single DEventWriterROOT_kFitPullStudies object and marks the factory as persistent so it doesn't get deleted every event.
			SetFactoryFlag(PERSISTANT);
			ClearFactoryFlag(WRITE_TO_OUTPUT);

			_data.push_back(new DEventWriterROOT_kFitPullStudies());
			_data.back()->Initialize(locEventLoop);
			return NOERROR;
		}
};

#endif // _DEventWriterROOT_factory_kFitPullStudies_
