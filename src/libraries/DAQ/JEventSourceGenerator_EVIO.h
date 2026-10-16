// $Id$
// $HeadURL$
//
//    File: JEventSourceGenerator_EVIO.h
// Created: Tue May 21 14:05:48 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _JEventSourceGenerator_EVIO_
#define _JEventSourceGenerator_EVIO_

#include <JANA/JEventSourceGenerator.h>

#include "JEventSource_EVIO.h"

class JEventSourceGenerator_EVIO: public JEventSourceGenerator{
	public:

		std::string GetType() const override { return "JEventSource_EVIO"; }
		std::string GetDescription() const override;

		double CheckOpenable(string source) override;
		JEventSource* MakeJEventSource(string source) override;
};

#endif // _JEventSourceGenerator_EVIO_

