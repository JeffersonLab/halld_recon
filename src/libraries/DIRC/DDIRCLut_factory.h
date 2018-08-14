// $Id$
//
//    File: DDIRCLut_factory.h
//

#ifndef _DDIRCLut_factory_
#define _DDIRCLut_factory_

#include <JANA/JFactory.h>
#include "DDIRCLut.h"

class DDIRCLut_factory:public JFactory<DDIRCLut> {

public:
	
	DDIRCLut_factory(){};
	~DDIRCLut_factory(){};

private:
	jerror_t brun(JEventLoop *loop, int32_t runnumber){
		
		assert( _data.size() == 0 );

		flags = PERSISTANT;
		_data.push_back( new DDIRCLut(loop) );
		
		return NOERROR;
	}

        jerror_t erun(void){
		
		for(unsigned int i=0; i<_data.size(); i++)delete _data[i];
		_data.clear();
		
		return NOERROR;
	}

	jerror_t evnt(jana::JEventLoop *loop, uint64_t eventnumber){
		
		return NOERROR;
	}
};

#endif // _DDIRCLut_factory_

