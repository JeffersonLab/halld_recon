//********************************************************************
// DFDCCathodeCluster_factory.h: Class definition for a factory that
// associates cathode hits into clusters.
// Author: Craig Bookwalter
//********************************************************************

#ifndef DFACTORY_DFDCCATHODECLUSTER_H
#define DFACTORY_DFDCCATHODECLUSTER_H

#include <JANA/JFactoryT.h>
#include <JANA/JException.h>
#include <JANA/Compatibility/JStreamLog.h>


#include "DFDCCathodeCluster.h"
#include "DFDCHit.h"
#include "DFDCGeometry.h"

#include <algorithm>
#include <map>
#include <cmath>

///
/// class DFDCCathodeCluster_factory: 
/// defines a JFactory for producing groups of cathode strips that form a cluster
///  
class DFDCCathodeCluster_factory : public JFactoryT<DFDCCathodeCluster> {
	public:
		///
		/// DFDCCathodeCluster_factory::DFDCCathodeCluster_factory():
		///	default constructor--initializes log file
		///
		DFDCCathodeCluster_factory();
		
		///
		/// DFDCCathodeCluster_factory::~DFDCCathodeCluster_factory():
		/// default destructor--closes log file.
		///
		~DFDCCathodeCluster_factory();
		
		///
		/// DFDCCathodeCluster_factory::pique():
		/// takes a single layer's worth of cathode hits and attempts to 
		/// create DFDCCathodeClusters
		/// by grouping together hits with consecutive strip numbers.
		///
		void pique(vector<const DFDCHit*>& h);
			
	protected:
		///
		/// DFDCCathodeCluster_factory::evnt():
		/// This (along with DFDCCathodeCluster_factory::pique()) 
		/// is the place cathode hits are associated into cathode clusters. This function 
		/// should eventually be modified to do more sophisticated peak finding. 
		///
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void Init() override;
	private:
		JStreamLog* _log;
		double TIME_SLICE;
};

#endif // DFACTORY_DFDCCATHODECLUSTER_H

