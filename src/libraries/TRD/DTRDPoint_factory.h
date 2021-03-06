//******************************************************************
// DTRDPoint_factory.h: modeled after DFDCPeudo_factory
//******************************************************************
#ifndef DFACTORY_DTRDPOINT_H
#define DFACTORY_DTRDPOINT_H

#include <JANA/JFactory.h>
#include <JANA/JObject.h>
using namespace jana;

#include "DTRDPoint.h"
#include "HDGEOMETRY/DGeometry.h"

///
/// class DTRDPoint_factory: definition for a JFactory that
/// produces points from wire hits and GEM strips
/// 
class DTRDPoint_factory : public JFactory<DTRDPoint> {
	public:
		
	        DTRDPoint_factory(){};
		~DTRDPoint_factory(){};					

	protected:
		///
		/// DTRDPoint_factory::evnt():
		/// this is the place that wire hits and GEM strip clusters 
		/// are organized into points.
		///
		jerror_t init(void);
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventNo);
		jerror_t brun(JEventLoop *loop, int32_t runnumber);
		jerror_t erun(void);

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

