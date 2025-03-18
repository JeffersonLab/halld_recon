//********************************************************************
// DFDCCathodeCluster_factory.h: Class definition for a factory that
// associates cathode hits into clusters.
// Author: Craig Bookwalter
//********************************************************************

#ifndef DFACTORY_DFDCCATHODECLUSTER_H
#define DFACTORY_DFDCCATHODECLUSTER_H

#include <JANA/JFactory.h>
#include <JANA/JException.h>
#include <JANA/JStreamLog.h>
using namespace jana;

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
class DFDCCathodeCluster_factory : public JFactory<DFDCCathodeCluster> {
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
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventNo);	
		jerror_t init(void);
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
	private:
		JStreamLog* _log;
		double TIME_SLICE;
  bool PROFILE_TIME;
  double cumulative_time=0.,cumulative_events=0.;
};

#endif // DFACTORY_DFDCCATHODECLUSTER_H

