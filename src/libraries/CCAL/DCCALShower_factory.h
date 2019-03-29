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
#include "DCCALGeometry.h"
#include "DCCALHit.h"
#include "hycal.h"

using namespace jana;

class DCCALShower_factory:public JFactory<DCCALShower>{

	public:
		DCCALShower_factory();
		~DCCALShower_factory(){};
		

	private:

		jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	
		
		void cleanHitPattern( vector< const DCCALHit* > hitarray, vector< const DCCALHit* > &hitarrayClean );
		void final_cluster_processing( vector< ccalcluster_t > &ccalcluster, int n_h_clusters );
		float getEnergyWeightedTime( cluster_t cluster_storage, int nHits );
		float getCorrectedTime( float time, float energy );
		float shower_depth( float energy );
		float energy_correct( float energy, int id );
		float f_nonlin( float e, int id );
		
		bool LoadCCALProfileData(JApplication *japp, int32_t runnumber);
	
		double m_zTarget;
		double m_CCALfront;

		JApplication *japp;
		
		int           SHOWER_DEBUG;
		int           MIN_CLUSTER_BLOCK_COUNT;
		float         MIN_CLUSTER_SEED_ENERGY;
		float         MIN_CLUSTER_ENERGY;
		float         MAX_CLUSTER_ENERGY;
		float         TIME_CUT;
		int           MAX_HITS_FOR_CLUSTERING;
		int           DO_NONLINEAR_CORRECTION;
		
		float         CCAL_RADIATION_LENGTH;
		float         CCAL_CRITICAL_ENERGY;
		
		int CCAL_CHANS = DCCALGeometry::kCCALMaxChannels;
		vector< float > Nonlin_p0;
		vector< float > Nonlin_p1;
		vector< float > Nonlin_p2;
		vector< float > Nonlin_p3;

		vector< float > timewalk_p0;
		vector< float > timewalk_p1;
		vector< float > timewalk_p2;
		vector< float > timewalk_p3;
		
};

#endif // _DCCALShower_factory_

