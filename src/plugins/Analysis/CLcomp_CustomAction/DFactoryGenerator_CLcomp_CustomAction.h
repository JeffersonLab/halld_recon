// $Id$
//
//    File: DFactoryGenerator_CLcomp_CustomAction.h
// Created: Tue Jan 30 11:06:19 EST 2018
// Creator: aebarnes (on Linux egbert 2.6.32-696.13.2.el6.x86_64 x86_64)
//

#ifndef _DFactoryGenerator_CLcomp_CustomAction_
#define _DFactoryGenerator_CLcomp_CustomAction_

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_p2k.h"

class DFactoryGenerator_CLcomp_CustomAction : public jana::JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_CLcomp_CustomAction";}
		
		jerror_t GenerateFactories(jana::JEventLoop* locEventLoop)
		{
			locEventLoop->AddFactory(new DReaction_factory_p2k());
			return NOERROR;
		}
};

#endif // _DFactoryGenerator_CLcomp_CustomAction_
