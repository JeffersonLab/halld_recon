// $Id$
//
//    File: DFactoryGenerator_B3pi_eff_missprot.h
// Created: Fri Jun 30 00:38:18 EDT 2017
// Creator: jmhardin (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DFactoryGenerator_B3pi_eff_missprot_
#define _DFactoryGenerator_B3pi_eff_missprot_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_B3pi_eff_missprot.h"

class DFactoryGenerator_B3pi_eff_missprot : public JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_B3pi_eff_missprot";}

		void GenerateFactories(JFactorySet* fs)
		{
			fs->Add(new DReaction_factory_B3pi_eff_missprot());
		}
};

#endif // _DFactoryGenerator_B3pi_eff_missprot_

