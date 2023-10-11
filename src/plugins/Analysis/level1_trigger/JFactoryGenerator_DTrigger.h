// $Id$
//
//    File: JFactoryGenerator_DTrigger.h
// Created: Tue Jun  7 10:14:40 EDT 2011
// Creator: davidl (on Darwin eleanor.jlab.org 10.7.0 i386)
//

#ifndef _JFactoryGenerator_DTrigger_
#define _JFactoryGenerator_DTrigger_

#include <JANA/JFactoryGenerator.h>

#include "DTrigger_factory.h"

class JFactoryGenerator_DTrigger: public JFactoryGenerator{
	public:
		JFactoryGenerator_DTrigger(){}
		virtual ~JFactoryGenerator_DTrigger(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JFactoryGenerator_DTrigger";}
		
		void GenerateFactories(JFactorySet* fs) override{
			fs->Add(new DTrigger_factory());
		}

};

#endif // _JFactoryGenerator_DTrigger_

