// $Id$
//
//    File: DFactoryGenerator_p4pi_hists.h
// Created: Mon Aug 29 16:20:58 EDT 2016
// Creator: aaustreg (on Linux halld01.jlab.org 2.6.32-642.3.1.el6.x86_64 x86_64)
//

#ifndef _DFactoryGenerator_p4pi_hists_
#define _DFactoryGenerator_p4pi_hists_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_p4pi_hists.h"

class DFactoryGenerator_p4pi_hists : public JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_p4pi_hists";}
		
		void GenerateFactories(JFactorySet* fs)
		{
			fs->Add(new DReaction_factory_p4pi_hists());
		}
};

#endif // _DFactoryGenerator_p4pi_hists_

