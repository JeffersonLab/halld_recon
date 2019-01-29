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
		
		// store these values before passing them on to the Fortran routines
		// keep them simply public and static for now
		//float acell[2][501][501] = { { {0.} } };
		//float ad2c[2][501][501] = { { {0.} } };

	private:

		jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	
		
		bool LoadCCALProfileData(JApplication *japp, int32_t runnumber);

		JApplication *japp;
		
		int           SHOWER_DEBUG;
		int           MIN_CLUSTER_BLOCK_COUNT;
		float         MIN_CLUSTER_SEED_ENERGY;
		float         TIME_CUT;
		unsigned int  MAX_HITS_FOR_CLUSTERING;
};

#endif // _DCCALShower_factory_

