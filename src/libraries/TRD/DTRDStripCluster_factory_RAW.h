//********************************************************************
// DTRDStripCluster_factory.h: modeled after DFDCCathodeCluster_factory
// version that analyzes the raw data
//********************************************************************

#ifndef DFACTORY_DTRDSTRIPCLUSTER_RAW_H
#define DFACTORY_DTRDSTRIPCLUSTER_RAW_H

#include <JANA/JFactoryT.h>
using namespace std;

#include "DTRDStripCluster.h"
#include "DTRDHit.h"

#include <algorithm>
#include <vector>
#include <map>
#include <cmath>

///
/// class DTRDStripCluster_factory_RAW: 
/// defines a JFactory for producing groups of cathode strips that form a cluster
///  
class DTRDStripCluster_factory_RAW:public JFactoryT<DTRDStripCluster> {
	public:
	    DTRDStripCluster_factory_RAW(){  SetTag("RAW");  };
		~DTRDStripCluster_factory_RAW(){};
		
		///
		/// DTRDStripCluster_factory_RAW::pique():
		/// takes a single layer's worth of cathode hits and attempts to 
		/// create DTRDStripClusters
		/// by grouping together hits with consecutive strip numbers.
		///
		//void pique(vector<const DTRDHit*>& h);
			
	protected:
		///
		/// DTRDStripCluster_factory_RAW::evnt():
		/// This (along with DTRDStripCluster_factory::pique()) 
		/// is the place cathode hits are associated into cathode clusters. This function 
		/// should eventually be modified to do more sophisticated peak finding. 
		///
		void Process(const std::shared_ptr<const JEvent>& event) override;	
		void Init() override;

	private:
		//double TIME_SLICE;

		// hit detection algorithm parameters, in ADC units
		int TimeWindowStart;
		int TimeMin;
		int TimeMax;
		int THRESHOLD;
		int MINIMUM_HITS_FOR_CLUSTERING;
		double CLUSTERING_THRESHOLD;
		int MinClustSize;
		double MinClustWidth;
		double MinClustLength;
		double zStart;
		double zEnd;
		
		
		const int MAX_ADC_SAMPLES = 100;
		const int NUM_X_PLANES = 720;
		const int NUM_Y_PLANES = 432;
		
		vector< vector< vector<double> > >   pulse_data;
};

#endif // DFACTORY_DTRDSTRIPCLUSTER_RAW_H

