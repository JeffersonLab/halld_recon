// $Id$
//
//    File: DCCALTruthShower_factory.h
// Created: Tue Nov 30 15:02:26 EST 2010
// Creator: davidl (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DCCALTruthShower_factory_
#define _DCCALTruthShower_factory_

#include <JANA/JFactoryT.h>
#include "DCCALTruthShower.h"

class DCCALTruthShower_factory:public JFactoryT<DCCALTruthShower>{
	public:
		DCCALTruthShower_factory() = default;
		~DCCALTruthShower_factory() override = default;


	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DCCALTruthShower_factory_

