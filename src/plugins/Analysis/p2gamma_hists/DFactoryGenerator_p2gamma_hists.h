// $Id$
//
//    File: DFactoryGenerator_p2gamma_hists.h
// Created: Tue Apr 28 21:19:41 EDT 2015
// Creator: jrsteven (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _DFactoryGenerator_p2gamma_hists_
#define _DFactoryGenerator_p2gamma_hists_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_p2gamma_hists.h"

class DFactoryGenerator_p2gamma_hists : public JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_p2gamma_hists";}
		
		void GenerateFactories(JFactorySet* factorySet) override
		{
			factorySet->Add(new DReaction_factory_p2gamma_hists());
		}
};

#endif // _DFactoryGenerator_p2gamma_hists_

