//******************************************************************
// DPadGEMPoint_factory.h:
//******************************************************************
#ifndef DFACTORY_DPADGEMPOINT_H
#define DFACTORY_DPADGEMPOINT_H

#include <JANA/JFactory.h>
#include <JANA/JObject.h>
using namespace jana;

#include "DTRDHit.h"
#include "DGEMPoint.h"
#include "HDGEOMETRY/DGeometry.h"

///
/// class DPadGEMPoint_factory: definition for a JFactory that
/// produces points from Pad GEM detector
/// 
class DPadGEMPoint_factory : public JFactory<DGEMPoint> {
	public:
                const char* Tag(void){return "PAD";}

	        DPadGEMPoint_factory(){};
		~DPadGEMPoint_factory(){};					

	protected:
		///
		/// DPadGEMPoint_factory::evnt():
		/// this is the place that pad hits are organized into points.
		///
		jerror_t init(void);
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventNo);
		jerror_t brun(JEventLoop *loop, int32_t runnumber);
		jerror_t erun(void);
 		
	private:		
		double X0, Y0, pitch;
		vector<double>dTRDz;

};

#endif // DFACTORY_DPADGEMPOINT_H

