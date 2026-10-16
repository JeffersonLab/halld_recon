// $Id$
//
//    File: JFactoryGenerator_Df250PulseIntegral.h
// Created: Thu Feb 13 12:49:12 EST 2014
// Creator: dalton (on Linux gluon104.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#ifndef _JFactoryGenerator_Df250PulseIntegral_
#define _JFactoryGenerator_Df250PulseIntegral_

#include <JANA/JFactoryGenerator.h>

#include "Df250PulseIntegral_factory.h"

class JFactoryGenerator_Df250PulseIntegral: public JFactoryGenerator{
	public:
		JFactoryGenerator_Df250PulseIntegral(){}
		virtual ~JFactoryGenerator_Df250PulseIntegral(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JFactoryGenerator_Df250PulseIntegral";}

		void GenerateFactories(JFactorySet* fs){
			fs->Add(new Df250PulseIntegral_factory());
		}
};

#endif // _JFactoryGenerator_Df250PulseIntegral_

