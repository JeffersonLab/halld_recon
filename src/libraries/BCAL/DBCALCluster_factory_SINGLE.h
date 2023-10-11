// $Id$
//
//    File: DBCALCluster_factory_SINGLE.h
// Created: Fri Sep  7 12:13:07 EDT 2012
// Creator: davidl (on Darwin harriet.jlab.org 11.4.0 i386)
//

#ifndef _DBCALCluster_factory_SINGLE_
#define _DBCALCluster_factory_SINGLE_

#include <JANA/JFactoryT.h>
#include "DBCALCluster.h"

class DBCALCluster_factory_SINGLE:public JFactoryT<DBCALCluster>{

	/// This factory will create a single DBCALCluster objects from
	/// all of the DBCALPoint objects. It is intended only for
	/// debugging of simulated data where a single cluster is expected.
	///
	/// If there are no DBCALPoint objects, then the DBCALCluster
	/// object is not created.

	public:
		DBCALCluster_factory_SINGLE(){
			SetTag("SINGLE");
		};
		~DBCALCluster_factory_SINGLE(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		double m_z_target_center;
		const DBCALGeometry *dBCALGeom;

};

#endif // _DBCALCluster_factory_SINGLE_

