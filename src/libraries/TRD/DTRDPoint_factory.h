//******************************************************************
// DTRDPoint_factory.h: modeled after DFDCPeudo_factory
//******************************************************************
#ifndef DFACTORY_DTRDPOINT_H
#define DFACTORY_DTRDPOINT_H

#include <JANA/JFactoryT.h>

#include "DTRDPoint.h"
#include "HDGEOMETRY/DGeometry.h"

///
/// class DTRDPoint_factory: definition for a JFactory that
/// produces points from strip hits
/// 
class DTRDPoint_factory : public JFactoryT<DTRDPoint> {
	public:
		
		DTRDPoint_factory(){};
		~DTRDPoint_factory(){};					

	protected:
		///
		/// DTRDPoint_factory::Process:
		/// this is the place that wire hits and GEM strip clusters 
		/// are organized into points.
		///
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;

// 		double calcClusterTime(const DTRDStripCluster *clus);
// 		double calcClusterPosition(const DTRDStripCluster *clus);
 		
	private:		
  		double TIME_DIFF_MAX;
   		double dE_DIFF_MAX;

		vector<double>dTRDz;

};

#endif // DFACTORY_DTRDPOINT_H

