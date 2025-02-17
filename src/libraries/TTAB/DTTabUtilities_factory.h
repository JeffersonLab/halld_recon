// $Id$
//
//    File: DTTabUtilities_factory.h
// Created: Fri Apr  3 09:41:29 EDT 2015
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-504.12.2.el6.x86_64 x86_64)
//

#ifndef _DTTabUtilities_factory_
#define _DTTabUtilities_factory_

#include <JANA/JFactoryT.h>
#include "DTTabUtilities.h"
#include "TRACKING/DMCThrown.h"

class DTTabUtilities_factory : public JFactoryT<DTTabUtilities>
{
	public:
		DTTabUtilities_factory(){};
		virtual ~DTTabUtilities_factory(){};

	private:
		void BeginRun(const std::shared_ptr<const JEvent> &aEvent) override;
		void Process(const std::shared_ptr<const JEvent>& aEvent) override;

		//F1TDCs: Old System ONLY //Early Fall 2014 Commissioning data ONLY
		uint64_t dRolloverTimeWindowLength; //"T" or "T_{frame}"
		uint64_t dNumTDCTicksInRolloverTimeWindow; //"N" or "N_{frame}"

		//CAEN1290s:
		int dCAENTIPhaseDifference; //0 -> 5
};

#endif // _DTTabUtilities_factory_
