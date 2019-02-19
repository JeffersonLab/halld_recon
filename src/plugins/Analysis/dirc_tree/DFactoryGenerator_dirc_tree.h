// $Id$
//
//    File: DFactoryGenerator_dirc_tree.h
//

#ifndef _DFactoryGenerator_dirc_tree_
#define _DFactoryGenerator_dirc_tree_

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_dirc_tree.h"

class DFactoryGenerator_dirc_tree : public jana::JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_dirc_tree";}
		
		jerror_t GenerateFactories(jana::JEventLoop* locEventLoop)
		{
			locEventLoop->AddFactory(new DReaction_factory_dirc_tree());
			return NOERROR;
		}
};

#endif // _DFactoryGenerator_dirc_tree_

