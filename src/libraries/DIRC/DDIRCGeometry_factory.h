// $Id$
//
//    File: DDIRCGeometry_factory.h
//

#ifndef _DDIRCGeometry_factory_
#define _DDIRCGeometry_factory_

#include "JANA/JFactory.h"
using namespace jana;

#include "DDIRCGeometry.h"

class DDIRCGeometry_factory : public JFactory<DDIRCGeometry> {

public:
	
	DDIRCGeometry_factory() {}
	~DDIRCGeometry_factory(){}

	DDIRCGeometry *dircgeometry;
	
	//------------------
	// brun
	//------------------
	jerror_t brun(JEventLoop *loop, int32_t runnumber)
	{
		// (See DTAGHGeometry_factory.h)
		SetFactoryFlag(NOT_OBJECT_OWNER);
		ClearFactoryFlag(WRITE_TO_OUTPUT);
		
		if( dircgeometry ) delete dircgeometry;
		
		dircgeometry = new DDIRCGeometry(runnumber);
		
		return NOERROR;
	}
	
	//------------------
	// evnt
	//------------------
	jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
	{
		// Reuse existing DDIRCGeometry object.
		if( dircgeometry ) _data.push_back( dircgeometry );
		
		return NOERROR;
	}
	
	//------------------
	// erun
	//------------------
	jerror_t erun(void)
	{
		if( dircgeometry ) delete dircgeometry;
		dircgeometry = NULL;
		
		return NOERROR;
	}	
};

#endif // _DDIRCGeometry_factory_

