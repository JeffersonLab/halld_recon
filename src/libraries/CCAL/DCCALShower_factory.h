/*
 *  File: DCCALHit_factory.h
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */

#ifndef _DCCALShower_factory_
#define _DCCALShower_factory_

#include <JANA/JFactory.h>
#include <JANA/JEventLoop.h>

#include "DCCALShower.h"

using namespace jana;

class DCCALShower_factory:public JFactory<DCCALShower>{

	public:
		DCCALShower_factory();
		~DCCALShower_factory(){};
			
	private:

		jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	

		int           SHOWER_DEBUG;
		int           MIN_CLUSTER_BLOCK_COUNT;
		float         MIN_CLUSTER_SEED_ENERGY;
		float         TIME_CUT;
		unsigned int  MAX_HITS_FOR_CLUSTERING;

};

#endif // _DCCALShower_factory_

