//******************************************************************
// DTRDPoint_Hit_factory.h: modeled after DFDCPeudo_factory
//******************************************************************
#ifndef DFACTORY_DTRDPOINT_HIT_H
#define DFACTORY_DTRDPOINT_HIT_H

//#include <JANA/JFactory.h>
//#include <JANA/JObject.h>
//using namespace jana;
#include <JANA/JFactoryT.h>

#include "DTRDPoint_Hit.h"
#include "HDGEOMETRY/DGeometry.h"
#include "DTRDHit.h"

///
/// class DTRDPoint_Hit_factory: definition for a JFactory that
/// produces points from GEMTRD strip hits (x,y) in time
/// 
class DTRDPoint_Hit_factory : public JFactoryT<DTRDPoint_Hit> {
	public:
		
	        //DTRDPoint_factory_Hit(){ SetTag("Hit"); };
			DTRDPoint_Hit_factory(){};
		~DTRDPoint_Hit_factory(){};					

	protected:
		///
		/// DTRDPoint_Hit_factory::evnt():
		/// this is the place that GEMTRD hits  
		/// are organized into points.
		///
		//jerror_t init(void);
		void Init() override;
		//jerror_t evnt(JEventLoop *eventLoop, uint64_t eventNo);
		void Process(const std::shared_ptr<const JEvent>& event) override;
		//jerror_t brun(JEventLoop *loop, int32_t runnumber);
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		//jerror_t erun(void);
		void EndRun() override;

// 		double calcClusterTime(const DTRDStripCluster *clus);
// 		double calcClusterPosition(const DTRDStripCluster *clus);
 		
	private:		
  		double TIME_DIFF_MAX;
//   		double DIST_DIFF_MAX;

		vector<double>dTRDz;

};

#endif // DFACTORY_DTRDPOINT_HIT_H

