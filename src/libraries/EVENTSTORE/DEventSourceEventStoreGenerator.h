// $Id$
//
//    File: DEventSourceEventStoreGenerator.h
// Created: Sat May  8 13:54:46 EDT 2010
// Creator: davidl (on Darwin Amelia.local 9.8.0 i386)
//

#ifndef _DEventSourceEventStoreGenerator_
#define _DEventSourceEventStoreGenerator_

#include <string>
using namespace std;

#include <JANA/JEventSourceGenerator.h>

class DEventSourceEventStoreGenerator: public JEventSourceGenerator{
	public:
		DEventSourceEventStoreGenerator();
		~DEventSourceEventStoreGenerator() override;

		std::string GetType() const override;
		std::string GetDescription(void) const override;
		double CheckOpenable(string source) override;
		JEventSource* MakeJEventSource(string source) override;
};

#endif // _DEventSourceEventStoreGenerator_

