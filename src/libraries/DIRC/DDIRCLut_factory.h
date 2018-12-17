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

	DDIRCLut *dirclut;

	jerror_t init(void){ 
		//dirclut = new DDIRCLut();
		return NOERROR;
	}
	
	jerror_t brun(JEventLoop *loop, int32_t runnumber){
		
		//dirclut->brun(loop);

		assert( _data.size() == 0 );

		SetFactoryFlag(NOT_OBJECT_OWNER);
                ClearFactoryFlag(WRITE_TO_OUTPUT);

                if( dirclut ) delete dirclut;
                dirclut = new DDIRCLut();
		dirclut->brun(loop);
		dirclut->CreateDebugHistograms();

        	return NOERROR;
	}

        jerror_t erun(void){
		
		if( dirclut ) delete dirclut;
                dirclut = NULL;
		
		return NOERROR;
	}

	jerror_t evnt(jana::JEventLoop *loop, uint64_t eventnumber){

		// Reuse existing DDIRCLut object
		if( dirclut ) _data.push_back( dirclut );	
	
		return NOERROR;
	}
};

#endif // _DDIRCLut_factory_

