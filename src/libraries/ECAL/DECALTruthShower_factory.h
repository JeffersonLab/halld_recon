// $Id$
//
//    File: DECALTruthShower_factory.h
//

#ifndef _DECALTruthShower_factory_
#define _DECALTruthShower_factory_

#include <JANA/JFactoryT.h>
#include "DECALTruthShower.h"

class DECALTruthShower_factory:public JFactoryT<DECALTruthShower>{
	public:
		DECALTruthShower_factory(){};
		~DECALTruthShower_factory(){};


	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DECALTruthShower_factory_

