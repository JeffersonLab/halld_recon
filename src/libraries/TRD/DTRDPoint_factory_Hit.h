//******************************************************************
// DTRDPoint_Hit_factory.h: modeled after DFDCPeudo_factory
//******************************************************************
#ifndef DFACTORY_DTRDPOINT_HIT_H
#define DFACTORY_DTRDPOINT_HIT_H

#include <JANA/JFactoryT.h>
#include "DTRDPoint.h"
#include "HDGEOMETRY/DGeometry.h"
#include "DTRDHit.h"

///
/// class DTRDPoint_factory_Hit: definition for a JFactory that
/// produces points from GEMTRD strip hits (x,y) in time
///
class DTRDPoint_factory_Hit : public JFactoryT<DTRDPoint> {
	public:
		
	        DTRDPoint_factory_Hit(){ SetTag("Hit"); };
		~DTRDPoint_factory_Hit(){};					

	protected:
		///
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
 		
	private:		
  		double TIME_DIFF_MAX;
   		double dE_DIFF_MAX;
		vector<double>dTRDz;

};

#endif // DFACTORY_DTRDPOINT_HIT_H

