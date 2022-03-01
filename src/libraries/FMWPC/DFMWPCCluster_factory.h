// $Id$
//
//    File: DFMWPCCluster_factory.h
// Created: Wed Oct 27 10:23:28 EDT 2021
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DFMWPCCluster_factory_
#define _DFMWPCCluster_factory_

#include <JANA/JFactory.h>

#include "DFMWPCCluster.h"
#include "DFMWPCHit.h"
#include "HDGEOMETRY/DGeometry.h"

class DFMWPCCluster_factory:public jana::JFactory<DFMWPCCluster>{
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
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
		double TIME_SLICE;
		double FMWPC_WIRE_SPACING;

		DGeometry *dgeom;
		vector<double> zvec;
		vector<double> xvec;
		vector<double> yvec;
		vector<double> rot;
};

#endif // _DFMWPCCluster_factory_

