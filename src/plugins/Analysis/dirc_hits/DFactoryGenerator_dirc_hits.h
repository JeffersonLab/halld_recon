// $Id$
//
//    File: DFactoryGenerator_dirc_hits.h
// Created: Fri Apr 12 18:47:09 CEST 2019
// Creator: aali (on Linux lxbk0198 3.16.0-7-amd64 unknown)
//

#ifndef _DFactoryGenerator_dirc_hits_
#define _DFactoryGenerator_dirc_hits_

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

//#include "dirc_hits_factory.h"

#include "DReaction_factory_dirc_hits.h"

class DFactoryGenerator_dirc_hits: public jana::JFactoryGenerator{
	public:
		DFactoryGenerator_dirc_hits(){}
		virtual ~DFactoryGenerator_dirc_hits(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_dirc_hits";}
		
		jerror_t GenerateFactories(jana::JEventLoop *loop){
			//loop->AddFactory(new dirc_hits_factory());
			loop->AddFactory(new DReaction_factory_dirc_hits());
			return NOERROR;
		}

};

#endif // _DFactoryGenerator_dirc_hits_

