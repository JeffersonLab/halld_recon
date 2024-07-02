// $Id$
//
//    File: DFactoryGenerator_ReactionEfficiency.h
// Created: Wed Jun 19 16:52:58 EDT 2019
// Creator: jrsteven (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DFactoryGenerator_ReactionEfficiency_
#define _DFactoryGenerator_ReactionEfficiency_

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_ReactionEfficiency.h"
#include "DEventWriterROOT_factory_ReactionEfficiency.h"

class DFactoryGenerator_ReactionEfficiency : public jana::JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_ReactionEfficiency";}
		
		jerror_t GenerateFactories(jana::JEventLoop* locEventLoop)
		{
			locEventLoop->AddFactory(new DReaction_factory_ReactionEfficiency());
			locEventLoop->AddFactory(new DEventWriterROOT_factory_ReactionEfficiency()); 
			return NOERROR;
		}
};

#endif // _DFactoryGenerator_ReactionEfficiency_

