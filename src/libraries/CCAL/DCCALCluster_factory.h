/*
 *  File: DCCALHit_factory.h
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */

#ifndef _DCCALCluster_factory_
#define _DCCALCluster_factory_

#include <JANA/JFactory.h>
#include <JANA/JEventLoop.h>

#include "DCCALCluster.h"

using namespace jana;

class DCCALCluster_factory:public JFactory<DCCALCluster>{

	public:
		DCCALCluster_factory();
		~DCCALCluster_factory(){};
			
	private:

		jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	
		//< Invoked via JEventProcessor virtual method

		unsigned int MIN_CLUSTER_BLOCK_COUNT;
		float  MIN_CLUSTER_SEED_ENERGY;
		float  TIME_CUT;
		uint32_t MAX_HITS_FOR_CLUSTERING;

};

#endif // _DCCALCluster_factory_

