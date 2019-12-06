//******************************************************************
// DGEMPoint_factory.h: modeled after DFDCPeudo_factory
//******************************************************************
#ifndef DFACTORY_DGEMPOINT_H
#define DFACTORY_DGEMPOINT_H

#include <JANA/JFactory.h>
#include <JANA/JObject.h>
using namespace jana;

#include "DGEMPoint.h"
#include "HDGEOMETRY/DGeometry.h"

///
/// class DGEMPoint_factory: definition for a JFactory that
/// produces points from wire hits and GEM strips
/// 
class DGEMPoint_factory : public JFactory<DGEMPoint> {
	public:
		
	        DGEMPoint_factory(){};
		~DGEMPoint_factory(){};					

	protected:
		///
		/// DGEMPoint_factory::evnt():
		/// this is the place that wire hits and GEM strip clusters 
		/// are organized into points.
		///
		jerror_t init(void);
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventNo);
		jerror_t brun(JEventLoop *loop, int32_t runnumber);
		jerror_t erun(void);

		double calcClusterTime(const DGEMStripCluster *clus);
		double calcClusterPosition(const DGEMStripCluster *clus);
 		
	private:		
		double wireX0, stripY0, wireX_pitch, stripY_pitch;
		double wire_time_max;
		double gemX0, gemY0, gem_pitch;
		double gem_time_max, gem_dE_max;
		vector<double>dTRDz;

};

#endif // DFACTORY_DGEMPOINT_H

