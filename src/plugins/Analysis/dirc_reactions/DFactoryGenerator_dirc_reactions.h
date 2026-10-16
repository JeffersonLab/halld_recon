// $Id$
//
//    File: DFactoryGenerator_dirc_reactions.h
//

#ifndef _DFactoryGenerator_dirc_reactions_
#define _DFactoryGenerator_dirc_reactions_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_dirc_reactions.h"

class DFactoryGenerator_dirc_reactions : public JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_dirc_reactions";}

		void GenerateFactories(JFactorySet* fs)
		{
			fs->Add(new DReaction_factory_dirc_reactions());
		}
};

#endif // _DFactoryGenerator_dirc_reactions_

