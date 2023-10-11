// $Id$
//
//    File: DFMWPCMatchedTrack_factory.h
// Created: Sat Jan 22 08:53:49 EST 2022
// Creator: davidl (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DFMWPCMatchedTrack_factory_
#define _DFMWPCMatchedTrack_factory_

#include <JANA/JFactory.h>
#include <HDGEOMETRY/DGeometry.h>
#include "DFMWPCMatchedTrack.h"

class DFMWPCMatchedTrack_factory:public JFactoryT<DFMWPCMatchedTrack>{
	public:
		DFMWPCMatchedTrack_factory(){};
		~DFMWPCMatchedTrack_factory(){};
        
        double MIN_DELTA_T_FCAL_PROJECTION;  // min. time between track projection and FCAL hit to consider them matched
        double MIN_DELTA_T_FMWPC_PROJECTION; // min. time between track projection and FMWPC hit to consider them matched
        DGeometry *dgeom;
        vector<double> xvec;                 // x-offset for each layer in cm (should be close to 0)
        vector<double> yvec;                 // y-offset for each layer in cm (should be close to 0)
        double FMWPC_WIRE_SPACING;           // distance between wires in FMWPC in cm
        vector<DGeometry::fmwpc_wire_orientation_t> fmwpc_wire_orientation;  // Wire orientation for each layer

	private:
        void Init() override;
        void BeginRun(const std::shared_ptr<const JEvent>& event) override;
        void Process(const std::shared_ptr<const JEvent>& event) override;
        void EndRun() override;
        void Finish() override;
};

#endif // _DFMWPCMatchedTrack_factory_

