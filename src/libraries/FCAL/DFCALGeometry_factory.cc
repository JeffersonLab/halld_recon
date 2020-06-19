// $Id$
//
//    File: DFCALGeometry_factory.cc
// Created: Wed Aug 24 10:09:27 EST 2005
// Creator: shepherd (on Darwin 129-79-159-16.dhcp-bl.indiana.edu 8.2.0 powerpc)
//

#include <cassert>

#include "DFCALGeometry_factory.h"
#include "DFCALGeometry.h"
#include <HDGEOMETRY/DGeometry.h>

//------------------
// brun
//------------------
jerror_t DFCALGeometry_factory::brun(JEventLoop *loop, int32_t runnumber)
{
	assert( _data.size() == 0 );

	DApplication *dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
	const DGeometry *geom = dapp->GetDGeometry(runnumber);
  
	// Check for presence of PbWO4 insert
	int insert_row_size=0;
	geom->Get("//composition[@name='LeadTungstateFullRow']/mposX[@volume='LTBLwrapped']/@ncopy",insert_row_size);

	flags = PERSISTANT;
	_data.push_back( new DFCALGeometry(insert_row_size) );
	
	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DFCALGeometry_factory::erun(void)
{
	for(unsigned int i=0; i<_data.size(); i++)delete _data[i];
	_data.clear();
	
	return NOERROR;
}

