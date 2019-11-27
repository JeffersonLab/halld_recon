// $Id$
//
//    File: DBCALTDCDigiHit_factory_KO.h
// Created: Mon Sep 30 22:45:50 EDT 2019
// Creator: davidl (on Linux gluon113.jlab.org 3.10.0-957.21.3.el7.x86_64 x86_64)
//

#ifndef _DBCALTDCDigiHit_factory_KO_
#define _DBCALTDCDigiHit_factory_KO_

#include <tuple>
#include <random>

#include <JANA/JFactory.h>
#include <BCAL/DBCALTDCDigiHit.h>

class DBCALTDCDigiHit_factory_KO:public jana::JFactory<DBCALTDCDigiHit>{
	public:
		DBCALTDCDigiHit_factory_KO():distribution(0.0, 1.0){};
		~DBCALTDCDigiHit_factory_KO(){};
		const char* Tag(void){return "KO";}

	std::map< std::tuple<int,int,int,int>, double > BCAL_tdc_cell_eff;
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution;

	void FillEfficiencyMap(string config_prefix, std::map< std::tuple<int,int,int,int>, double > &eff_map);
	void CheckRange(int module, int layer, int sector, int end);

private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DBCALTDCDigiHit_factory_KO_

