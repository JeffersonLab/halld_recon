// $Id$
//
//    File: DFDCIntersection_factory.h
// Created: Tue Oct 30 11:24:53 EDT 2007
// Creator: davidl (on Darwin fwing-dhcp95.jlab.org 8.10.1 i386)
//

#ifndef _DFDCIntersection_factory_
#define _DFDCIntersection_factory_

#include <JANA/JFactoryT.h>
#include <DVector2.h>

#include "DFDCIntersection.h"
#include "DFDCGeometry.h"

class DFDCIntersection_factory:public JFactoryT<DFDCIntersection>{
	public:
		DFDCIntersection_factory(){};
		~DFDCIntersection_factory(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		//void EndRun() override;
		//void Finish() override;

		void MakeIntersectionPoints(vector<vector<const DFDCHit*> >&hits_by_layer);
		void MakeRestrictedIntersectionPoints(vector<vector<const DFDCHit*> >&hits_by_layer);
		
		void FindIntersections(vector<const DFDCHit*> &layer1, vector<const DFDCHit*> &layer2, vector<DFDCIntersection*> &intersections);

		vector<vector<vector<const DFDCHit*> > > fdchits_by_package; ///< fdchits_by_package[package][layer][hit]
		double MAX_DIST2;

	vector<vector<DFDCWire*> >fdcwires;
	bool USE_FDC, DEBUG_LEVEL;

};

#endif // _DFDCIntersection_factory_

