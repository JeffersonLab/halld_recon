// $Id$
//
//    File: DFMWPCCluster_factory.h
// Created: Wed Oct 27 10:23:28 EDT 2021
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DFMWPCCluster_factory_
#define _DFMWPCCluster_factory_

#include <JANA/JFactoryT.h>

#include "DFMWPCCluster.h"
#include "DFMWPCHit.h"
#include "HDGEOMETRY/DGeometry.h"

class DFMWPCCluster_factory:public JFactoryT<DFMWPCCluster>{
	public:
		DFMWPCCluster_factory(){};
		~DFMWPCCluster_factory(){};

		///
		/// DFDCCathodeCluster_factory::pique():
		/// takes a single layer's worth of wire hits and attempts to 
		/// create DFMWPCClusters
		/// by grouping together hits with consecutive strip numbers.
		///
		void pique(vector<const DFMWPCHit*>& h);

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		double TIME_SLICE;

		DGeometry *dgeom;
		vector<double> zvec;
		vector<double> xvec;
		vector<double> yvec;
        double FMWPC_WIRE_SPACING;
        vector<DGeometry::fmwpc_wire_orientation_t> fmwpc_wire_orientation;
};

#endif // _DFMWPCCluster_factory_

