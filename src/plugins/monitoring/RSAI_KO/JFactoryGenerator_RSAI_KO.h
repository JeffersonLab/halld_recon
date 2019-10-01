// $Id$
//
//    File: JFactoryGenerator_RSAI_KO.h
// Created: Fri Sep 27 07:25:48 EDT 2019
// Creator: davidl (on Linux gluon46.jlab.org 3.10.0-957.21.3.el7.x86_64 x86_64)
//

#ifndef _JFactoryGenerator_RSAI_KO_
#define _JFactoryGenerator_RSAI_KO_

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "DCDCDigiHit_factory_KO.h"

class JFactoryGenerator_RSAI_KO: public jana::JFactoryGenerator{
	public:
		JFactoryGenerator_RSAI_KO(){}
		virtual ~JFactoryGenerator_RSAI_KO(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JFactoryGenerator_RSAI_KO";}
		
		jerror_t GenerateFactories(jana::JEventLoop *loop){
			loop->AddFactory(new DCDCDigiHit_factory_KO());
			return NOERROR;
		}

};

#endif // _JFactoryGenerator_RSAI_KO_

