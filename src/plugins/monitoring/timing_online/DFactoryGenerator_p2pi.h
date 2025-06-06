// $Id$
//
//    File: DFactoryGenerator_p2pi.h
// Created: Thu May  7 16:22:04 EDT 2015
// Creator: mstaib (on Linux gluon109.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#ifndef _DFactoryGenerator_p2pi_
#define _DFactoryGenerator_p2pi_

#include <DANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_p2pi.h"

class DFactoryGenerator_p2pi : public JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_p2pi";}
		
		void GenerateFactories(JFactorySet* fs) override
		{
			fs->Add(new DReaction_factory_p2pi());
		}
};

#endif // _DFactoryGenerator_p2pi_

