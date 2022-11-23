// $Id$
//
//    File: DFactoryGenerator_cpp_hists.h
// Created: Tue May  3 09:24:43 EDT 2022
// Creator: aaustreg (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DFactoryGenerator_cpp_hists_
#define _DFactoryGenerator_cpp_hists_

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_cpp_hists.h"

class DFactoryGenerator_cpp_hists : public jana::JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_cpp_hists";}
		
		jerror_t GenerateFactories(jana::JEventLoop* locEventLoop)
		{
			locEventLoop->AddFactory(new DReaction_factory_cpp_hists());
			return NOERROR;
		}
};

#endif // _DFactoryGenerator_cpp_hists_

