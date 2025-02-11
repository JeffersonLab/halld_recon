// $Id$
// DFactoryGenerator_Z2pi_trees, modeled after DFactoryGenerator_p2pi_trees
//
//    File: DFactoryGenerator_p2pi_trees.h
// Created: Wed Mar 29 16:34:58 EDT 2017
// Creator: elton (on Linux ifarm1401.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DFactoryGenerator_Z2pi_trees_
#define _DFactoryGenerator_Z2pi_trees_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_Z2pi_trees.h"

class DFactoryGenerator_Z2pi_trees : public JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_Z2pi_trees";}
		
		void GenerateFactories(JFactorySet* fs)
		{
			fs->Add(new DReaction_factory_Z2pi_trees());
		}
};

#endif // _DFactoryGenerator_Z2pi_trees_

