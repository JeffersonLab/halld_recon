#ifndef _DFactoryGenerator_b1pi_hists_
#define _DFactoryGenerator_b1pi_hists_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_b1pi_hists.h"

class DFactoryGenerator_b1pi_hists: public JFactoryGenerator
{
	public:
		DFactoryGenerator_b1pi_hists(){}
		virtual ~DFactoryGenerator_b1pi_hists(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_b1pi_hists";}

		void GenerateFactories(JFactorySet *factory_set) override
		{
			factory_set->Add(new DReaction_factory_b1pi_hists());
		}
};

#endif // _DFactoryGenerator_b1pi_hists_

