// $Id$
//
//    File: DFactoryGenerator_dirc_tree.h
//

#ifndef _DFactoryGenerator_dirc_tree_
#define _DFactoryGenerator_dirc_tree_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_dirc_tree.h"

class DFactoryGenerator_dirc_tree : public JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_dirc_tree";}
		
		void GenerateFactories(JFactorySet* fs) override
		{
			fs->Add(new DReaction_factory_dirc_tree());
		}
};

#endif // _DFactoryGenerator_dirc_tree_

