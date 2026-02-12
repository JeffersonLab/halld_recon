//
// Author: Richard Jones  June 29, 2012
//
// DEventSourceRESTGenerator.h
//
/// Implements JEventSourceGenerator for REST files

#ifndef _DEventSourceRESTGenerator_
#define _DEventSourceRESTGenerator_

#include <JANA/JEventSourceGenerator.h>

class DEventSourceRESTGenerator: public JEventSourceGenerator
{
 public:
	std::string GetType() const override { return "DEventSourceREST"; }
	std::string GetDescription() const override;

	double CheckOpenable(std::string source) override;
	JEventSource* MakeJEventSource(std::string source) override;
};

#endif // _DEventSourceRESTGenerator_

