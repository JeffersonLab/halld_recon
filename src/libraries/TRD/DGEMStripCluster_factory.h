//********************************************************************
// DGEMStripCluster_factory.h: modeled after DFDCCathodeCluster_factory
//********************************************************************

#ifndef DFACTORY_DGEMSTRIPCLUSTER_H
#define DFACTORY_DGEMSTRIPCLUSTER_H

#include <JANA/JFactoryT.h>
using namespace std;

#include "DGEMStripCluster.h"
#include "DGEMHit.h"

#include <algorithm>
#include <map>
#include <cmath>

///
/// class DGEMStripCluster_factory: 
/// defines a JFactory for producing groups of cathode strips that form a cluster
///  
class DGEMStripCluster_factory:public JFactoryT<DGEMStripCluster> {
	public:
		DGEMStripCluster_factory(){};
		~DGEMStripCluster_factory(){};
		
		///
		/// DGEMStripCluster_factory::pique():
		/// takes a single layer's worth of cathode hits and attempts to 
		/// create DGEMStripClusters
		/// by grouping together hits with consecutive strip numbers.
		///
		void pique(vector<const DGEMHit*>& h);
			
	protected:
		///
		/// DGEMStripCluster_factory::evnt():
		/// This (along with DGEMStripCluster_factory::pique()) 
		/// is the place cathode hits are associated into cathode clusters. This function 
		/// should eventually be modified to do more sophisticated peak finding. 
		///
		void Process(const std::shared_ptr<const JEvent>& event) override;	
		void Init() override;
	private:
		double TIME_SLICE;
};

#endif // DFACTORY_DGEMSTRIPCLUSTER_H

