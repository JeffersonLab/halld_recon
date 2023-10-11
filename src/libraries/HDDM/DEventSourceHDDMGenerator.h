// $Id$
//
//    File: DEventSourceHDDMGenerator.h
// Created: Sat Jul  1 19:23:54 EDT 2006
// Creator: davidl (on Darwin Harriet.local 8.6.0 powerpc)
//

#ifndef _DEventSourceHDDMGenerator_
#define _DEventSourceHDDMGenerator_

#include <JANA/JEventSourceGenerator.h>

class DEventSourceHDDMGenerator:public JEventSourceGenerator{
	public:
		std::string GetType() const override { return "DEventSourceHDDM"; }
		std::string GetDescription() const override;

		double CheckOpenable(std::string source) override;
		JEventSource* MakeJEventSource(std::string source) override;
};

#endif // _DEventSourceHDDMGenerator_

