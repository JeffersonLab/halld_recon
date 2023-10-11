//******************************************************************
// DGEMPoint_factory.h: modeled after DFDCPeudo_factory
//******************************************************************
#ifndef DFACTORY_DGEMPOINT_H
#define DFACTORY_DGEMPOINT_H

#include <JANA/JFactoryT.h>

#include "DGEMPoint.h"

///
/// class DGEMPoint_factory: definition for a JFactory that
/// produces points from wire hits and GEM strips
/// 
class DGEMPoint_factory : public JFactoryT<DGEMPoint> {
	public:
		
	    DGEMPoint_factory(){};
		~DGEMPoint_factory(){};					

	protected:
		///
		/// DGEMPoint_factory::evnt():
		/// this is the place that wire hits and GEM strip clusters 
		/// are organized into points.
		///
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;

		double calcClusterTime(const DGEMStripCluster *clus);
		double calcClusterPosition(const DGEMStripCluster *clus);
 		
	private:		
		double wireX0, stripY0, wireX_pitch, stripY_pitch;
		double wire_time_max;
		double gemX0, gemY0, gem_pitch;
		double gem_time_max, gem_dE_max;
		std::vector<double>dTRDz;

};

#endif // DFACTORY_DGEMPOINT_H

