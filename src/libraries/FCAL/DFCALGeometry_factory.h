// $Id$
//
//    File: DFCALGeometry_factory.h
// Created: Wed Aug 24 10:09:27 EST 2005
// Creator: shepherd (on Darwin 129-79-159-16.dhcp-bl.indiana.edu 8.2.0 powerpc)
//

#ifndef _DFCALGeometry_factory_
#define _DFCALGeometry_factory_

#include <JANA/JFactoryT.h>

#include "DFCALGeometry.h"

class DFCALGeometry_factory : public JFactoryT<DFCALGeometry> {

public:
	
	DFCALGeometry_factory() = default;
	~DFCALGeometry_factory() override = default;

private:
	
	void BeginRun(const std::shared_ptr<const JEvent>& event) override;
	void EndRun() override;
};

#endif // _DFCALGeometry_factory_

