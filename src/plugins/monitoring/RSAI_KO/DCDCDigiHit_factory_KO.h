// $Id$
//
//    File: DCDCDigiHit_factory_KO.h
// Created: Thu Sep 26 22:51:16 EDT 2019
// Creator: davidl (on Linux gluon46.jlab.org 3.10.0-957.21.3.el7.x86_64 x86_64)
//

#ifndef _DCDCDigiHit_factory_KO_
#define _DCDCDigiHit_factory_KO_

#include <JANA/JFactoryT.h>
#include <CDC/DCDCDigiHit.h>

class DCDCDigiHit_factory_KO:public JFactoryT<DCDCDigiHit>{
	public:
		DCDCDigiHit_factory_KO(){
			SetTag("KO");
		};
		~DCDCDigiHit_factory_KO(){};

		std::map< std::pair<int,int>, double > CDC_pin_eff;

		int SetBoardEfficiency(string boardname, double eff); // returns number of pins in board

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DCDCDigiHit_factory_KO_

