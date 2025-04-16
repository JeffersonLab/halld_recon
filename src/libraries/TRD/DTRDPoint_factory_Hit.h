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
		
		double StripToPosition(int iplane, const DTRDHit *hit);

	protected:
		///
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
 		
	private:		
  		double TIME_DIFF_MAX;
   		double dE_DIFF_MAX;
		//vector<double>dTRDz;
		//double DRIFT_VELOCITY;
 	   	double dTRDx=0.,dTRDy=0.,dTRDz=0.;
		
		const int NUM_X_STRIPS = 720;
  		//const int NUM_Y_STRIPS = 432;
  		const int NUM_Y_STRIPS = 528; // but only 432 are instrumented as of Spring 2025
  		const double STRIP_PITCH=0.1; //1mm

};

#endif // DFACTORY_DTRDPOINT_HIT_H

