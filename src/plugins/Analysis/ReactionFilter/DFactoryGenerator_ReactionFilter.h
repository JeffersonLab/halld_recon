// $Id$
//
//    File: DFactoryGenerator_ReactionFilter.h
// Created: Mon Nov 21 17:54:40 EST 2016
// Creator: pmatt (on Darwin Pauls-MacBook-Pro-2.local 13.4.0 i386)
//

#ifndef _DFactoryGenerator_ReactionFilter_
#define _DFactoryGenerator_ReactionFilter_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_ReactionFilter.h"

class DFactoryGenerator_ReactionFilter : public JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_ReactionFilter";}
		
		void GenerateFactories(JFactorySet* fs)
		{
			fs->Add(new DReaction_factory_ReactionFilter());
		}
};

#endif // _DFactoryGenerator_ReactionFilter_

