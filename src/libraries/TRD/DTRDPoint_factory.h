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
/// produces points from wire hits and GEM strips
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

		double calcClusterTime(const DTRDStripCluster *clus);
		double calcClusterPosition(const DTRDStripCluster *clus);
 		
	private:		
		double wireX0, stripY0, wireX_pitch, stripY_pitch;
		double wire_time_max;
		double gemX0, gemY0, gem_pitch;
		double gem_time_max, gem_dE_max;
		vector<double>dTRDz;

};

#endif // DFACTORY_DTRDPOINT_H

