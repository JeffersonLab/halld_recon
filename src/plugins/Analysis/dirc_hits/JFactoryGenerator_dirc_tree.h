// $Id$
//
//    File: JFactoryGenerator_dirc_tree.h
// Created: Fri Apr 12 18:47:09 CEST 2019
// Creator: aali (on Linux lxbk0198 3.16.0-7-amd64 unknown)
//

#ifndef _JFactoryGenerator_dirc_tree_
#define _JFactoryGenerator_dirc_tree_

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

//#include "dirc_tree_factory.h"

#include "DReaction_factory_dirc_tree.h"

class JFactoryGenerator_dirc_tree: public jana::JFactoryGenerator{
	public:
		JFactoryGenerator_dirc_tree(){}
		virtual ~JFactoryGenerator_dirc_tree(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JFactoryGenerator_dirc_tree";}
		
		jerror_t GenerateFactories(jana::JEventLoop *loop){
			//loop->AddFactory(new dirc_tree_factory());
			loop->AddFactory(new DReaction_factory_dirc_tree());
			return NOERROR;
		}

};

#endif // _JFactoryGenerator_dirc_tree_

