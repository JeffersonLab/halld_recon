// $Id$
//
//    File: DDIRCGeometry_factory.h
//

#ifndef _DDIRCGeometry_factory_
#define _DDIRCGeometry_factory_

#include "JANA/JFactory.h"

#include "DDIRCGeometry.h"

class DDIRCGeometry_factory : public JFactoryT<DDIRCGeometry> {

public:
	
	DDIRCGeometry_factory() = default;
	~DDIRCGeometry_factory() override = default;

	DDIRCGeometry *dircgeometry=nullptr;
	
	//------------------
	// BeginRun
	//------------------
	void BeginRun(const std::shared_ptr<const JEvent>& event) override
	{
		// (See DTAGHGeometry_factory.h)
		SetFactoryFlag(NOT_OBJECT_OWNER);
		ClearFactoryFlag(WRITE_TO_OUTPUT);
		
		delete dircgeometry;
		dircgeometry = new DDIRCGeometry();
		dircgeometry->Initialize(event);
	}
	
	//------------------
	// Process
	//------------------
	void Process(const std::shared_ptr<const JEvent>& loop) override
	{
		// Reuse existing DDIRCGeometry object.
		if( dircgeometry ) Insert( dircgeometry );
	}
	
	//------------------
	// EndRun
	//------------------
	void EndRun() override
	{
		delete dircgeometry;
		dircgeometry = nullptr;
	}
};

#endif // _DDIRCGeometry_factory_

