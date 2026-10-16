// $Id$
//
//    File: DBCALDigiHit_factory_KO.h
// Created: Mon Sep 30 22:45:50 EDT 2019
// Creator: davidl (on Linux gluon113.jlab.org 3.10.0-957.21.3.el7.x86_64 x86_64)
//

#ifndef _DBCALDigiHit_factory_KO_
#define _DBCALDigiHit_factory_KO_

#include <tuple>
#include <random>

#include <JANA/JFactoryT.h>
#include <BCAL/DBCALDigiHit.h>

class DBCALDigiHit_factory_KO:public JFactoryT<DBCALDigiHit>{
	public:
		DBCALDigiHit_factory_KO():distribution(0.0, 1.0){
			SetTag("KO");
		};
		~DBCALDigiHit_factory_KO(){};

	std::map< std::tuple<int,int,int,int>, double > BCAL_adc_cell_eff;
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution;

	void FillEfficiencyMap(string config_prefix, std::map< std::tuple<int,int,int,int>, double > &eff_map);
	void CheckRange(int module, int layer, int sector, int end);

private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DBCALDigiHit_factory_KO_

