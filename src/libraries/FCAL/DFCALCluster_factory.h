// $Id: DFCALShower_factory.h 1899 2006-07-13 16:29:56Z davidl $
//
//    File: DFCALCluster_factory.h
// Created: Tue Nov 15 11:57:50 EST 2006
// Creator: MK (on Linux stan )
//

#ifndef _DFCALCluster_factory_
#define _DFCALCluster_factory_

#include <JANA/JFactoryT.h>

#include "DFCALCluster.h"


class DFCALCluster_factory:public JFactoryT<DFCALCluster>{

	public:
		DFCALCluster_factory();
		~DFCALCluster_factory(){};
			
	private:

		void BeginRun(const std::shared_ptr<const JEvent>& event) override;	
		void Process(const std::shared_ptr<const JEvent>& event) override;	

		unsigned int MIN_CLUSTER_BLOCK_COUNT;
		float MIN_CLUSTER_SEED_ENERGY;
		float TIME_CUT;
		uint32_t MAX_HITS_FOR_CLUSTERING;

		// this is the location of the front 
		// of the FCAL in a coordinate system 
		// where z = 0 is the center of the target

		double fcalFaceZ_TargetIsZeq0;
};

#endif // _DFCALCluster_factory_

