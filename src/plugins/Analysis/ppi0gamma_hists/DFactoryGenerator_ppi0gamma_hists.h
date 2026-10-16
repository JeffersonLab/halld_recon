// $Id$
//
//    File: DFactoryGenerator_ppi0gamma_hists.h
//

#ifndef _DFactoryGenerator_ppi0gamma_hists_
#define _DFactoryGenerator_ppi0gamma_hists_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_ppi0gamma_hists.h"

class DFactoryGenerator_ppi0gamma_hists : public JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_ppi0gamma_hists";}
		
		void GenerateFactories(JFactorySet* fs)
		{
			fs->Add(new DReaction_factory_ppi0gamma_hists());
		}
};

#endif // _DFactoryGenerator_ppi0gamma_hists_

